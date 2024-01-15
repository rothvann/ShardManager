#include "psychopomp/placer/ExpressionTree.h"

#include <deque>
#include <iostream>

namespace psychopomp {
ExpressionTree::ExpressionTree(
    std::shared_ptr<SolvingState> state, std::shared_ptr<SparseMappingTree> assignmentTree,
    std::function<int32_t(const SparseMappingTree&,
                          const std::vector<std::shared_ptr<MovementMap>>&,
                          const MetricsMap&, const std::vector<DomainId>&,
                          std::pair<Domain, DomainId>)>
        calcMetricFunc)
    : state_(state),
      assignmentTree_(assignmentTree),
      calcMetricFunc_(calcMetricFunc) {
  initializeMetricState();
}

std::shared_ptr<SparseMappingTree> ExpressionTree::getAssignmentTree() const {
  return assignmentTree_;
}

void ExpressionTree::canaryMoves(std::shared_ptr<MovementMap> committedMoves,
                                 std::shared_ptr<MovementMap> canaryMoves) {
  metricsMap_.clear();
  const auto& allMovements = canaryMoves->getAllMovements();
  // Check if shard / bin exists in assignment tree if exists then add shard to
  // be updated
  std::unordered_map<Domain,
                     std::unordered_map<DomainId, std::vector<DomainId>>>
      toUpdate;
  for (auto [shardId, binId] : allMovements) {
    auto parents = assignmentTree_->getParents(
        state_->getShardDomain(), shardId,
        {state_->getMovementMap(), committedMoves, canaryMoves});
    for (auto [parentDomain, parentDomainId] : parents) {
      toUpdate[parentDomain][parentDomainId].emplace_back(shardId);
    }
  }
  propagateChanges(toUpdate,
                   {state_->getMovementMap(), committedMoves, canaryMoves});
}

void ExpressionTree::commitMoves() { metricsMap_.commit(); }

const MetricsMap& ExpressionTree::getMetricsMap() const { return metricsMap_; }


void ExpressionTree::initializeMetricState() {
  auto shardDomain = state_->getShardDomain();
  auto shardIds = assignmentTree_->getAllDomainIds(shardDomain);
  std::unordered_map<Domain,
                     std::unordered_map<DomainId, std::vector<DomainId>>>
      toUpdate;
  for (auto shardId : shardIds) {
    auto parents = assignmentTree_->getParents(
        state_->getShardDomain(), shardId, {state_->getMovementMap()});
    for (auto [domain, domainId] : parents) {
      toUpdate[domain][domainId].emplace_back(shardId);
    }
  }

  propagateChanges(toUpdate, {state_->getMovementMap()});
}

void ExpressionTree::propagateChanges(
    const std::unordered_map<
        Domain, std::unordered_map<DomainId, std::vector<DomainId>>>& toUpdate,
    const std::vector<std::shared_ptr<MovementMap>>& movementMaps) {
  std::unordered_map<Domain,
                     std::unordered_map<DomainId, std::vector<DomainId>>>
      nextToUpdate;

  for (auto& [domain, domainIdChildrenMap] : toUpdate) {
    for (auto& [domainId, changedChildren] : domainIdChildrenMap) {
      auto prevVal = metricsMap_.get(domain, domainId);
      int32_t curVal =
          calcMetricFunc_(*assignmentTree_, movementMaps, metricsMap_, changedChildren,
                          {domain, domainId});
      if (!prevVal.has_value() || prevVal.value() != curVal) {
        metricsMap_.set(true /* isCanary */, curVal, domain, domainId);

        auto parents =
            assignmentTree_->getParents(domain, domainId, movementMaps);
        for (auto [parentDomain, parentDomainId] : parents) {
          nextToUpdate[parentDomain][parentDomainId].emplace_back(domainId);
        }
      }
    }
  }
  if (!nextToUpdate.empty()) {
    propagateChanges(nextToUpdate, movementMaps);
  }
}

}  // namespace psychopomp
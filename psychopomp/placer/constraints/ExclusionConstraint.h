#pragma once

#include <unordered_map>

#include "psychopomp/placer/constraints/Constraint.h"

// Create count map of each shard
namespace psychopomp {

class ExclusionConstraint : public Constraint {
 public:
  ExclusionConstraint(std::shared_ptr<SolvingState> state,
                      MovementConsistency consistency, Domain exclusionDomain,
                      const std::vector<DomainId>& shardIds,
                      int32_t faultWeight)
      : Constraint(state),
        exclusionDomain_(exclusionDomain),
        faultWeight_(faultWeight) {
    auto func =
        [&](const AssignmentTree& assignmentTree,
            const std::vector<std::shared_ptr<MovementMap>>& movementMaps,
            const MetricsMap& metricMap,
            const std::vector<DomainId>& changedChildren,
            std::pair<Domain, DomainId> node) -> int32_t {
      return addBinCount(assignmentTree, movementMaps, node);
    };
    expressionTree_ = std::make_shared<ExpressionTree>(
        state_, state->getShardDomain(), shardIds, func);
  }

 private:
  int32_t addBinCount(
      const AssignmentTree& assignmentTree,
      const std::vector<std::shared_ptr<MovementMap>>& movementMaps,
      std::pair<Domain, DomainId> node) {
    // Get next parent bin
    auto domain = node.first;
    auto domainId = node.second;

    if (domain == state_->getShardDomain()) {
      auto currentBin = getCurrentBin(assignmentTree, state_->getShardDomain(),
                                      domainId, state_->getBinDomain());
      auto futureBin =
          getFutureBin(assignmentTree, movementMaps, state_->getShardDomain(),
                       domainId, state_->getBinDomain());

      if (currentBin != kDefaultBin) {
        const auto& shardInfo = state_->getShardInfo(domainId);
        auto shardRangeId = shardInfo.shardRangeId;
        changeShardRangeCount(futureBin, shardRangeId, -1);
      }
      if (futureBin != kDefaultBin) {
        const auto& shardInfo = state_->getShardInfo(domainId);
        auto shardRangeId = shardInfo.shardRangeId;
        changeShardRangeCount(futureBin, shardRangeId, 1);
      }
    }
    return 0;
  }

  void changeShardRangeCount(DomainId binId, ShardRangeId shardRangeId,
                             int32_t count) {
    auto binParent = state_->getBinParentInDomain(binId, exclusionDomain_);
    if (!binParent) {
      return;
    }
    auto prevCount = shardRangeIdCount_[*binParent][shardRangeId];
    shardRangeIdCount_[*binParent][shardRangeId] += count;
    auto currentCount = prevCount + count;

    auto prevFaultWeight = faultWeight_ * (prevCount > 1 ? prevCount : 0);
    auto currentFaultWeight =
        faultWeight_ * (currentCount > 1 ? currentCount : 0);

    auto delta = currentFaultWeight - prevFaultWeight;

    // Maybe change to shardRangeIdCount_ to set and add for specific bins
    addTotalWeight(delta);
  }

  std::shared_ptr<ExpressionTree> expressionTree_;

  Domain exclusionDomain_;
  int32_t faultWeight_;

  std::unordered_map<DomainId, std::unordered_map<ShardRangeId, size_t>>
      shardRangeIdCount_;
};
}  // namespace psychopomp

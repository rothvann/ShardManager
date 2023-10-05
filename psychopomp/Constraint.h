#pragma once

#include <iostream>
#include <memory>

#include "psychopomp/ExpressionTree.h"
#include "psychopomp/State.h"
#include "psychopomp/Types.h"

namespace psychopomp {

class Constraint {
 public:
  Constraint(std::shared_ptr<State> state) : state_(state) {}
  virtual void canaryMoves(std::shared_ptr<MovementMap> comittiedMoves,
                           std::shared_ptr<MovementMap> canaryMoves) = 0;

  void commit() {
    commitMoves();

    auto& binWeightInfo = state_->getBinWeightInfo();
    binWeightInfo.binWeightMap.commit();
    binWeightInfo.totalWeight.commit();
  }

 protected:
  virtual void commitMoves() = 0;

  void resetBinWeights() {}

  void addBinWeight(DomainId domainId, int64_t binWeightDelta) {
    auto& binWeightInfo = state_->getBinWeightInfo();
    auto prevWeight =
        binWeightInfo.binWeightMap.getFromCommittedMap(domainId).value_or(0);
    binWeightInfo.binWeightMap.set(true /* isCanary */,
                                   prevWeight + binWeightDelta, domainId);

    auto prevTotalWeight =
        binWeightInfo.totalWeight.getCommittedVal().value_or(0);
    binWeightInfo.totalWeight.set(true /* isCanary */,
                                  prevTotalWeight + binWeightDelta);
  }

  std::shared_ptr<State> state_;
};

class MetricConstraint : public Constraint {
 public:
  MetricConstraint(std::shared_ptr<State> state, Domain domain,
                   const std::vector<DomainId>& domainIds, Metric metric,
                   int32_t capacity, int32_t faultWeight)
      : Constraint(state),
        domain_(domain),
        metric_(metric),
        capacity_(capacity),
        faultWeight_(faultWeight) {
    auto func =
        [&](const AssignmentTree& assignmentTree,
            const std::vector<std::shared_ptr<MovementMap>>& movementMaps,
            const MetricsMap& metricMap, const std::vector<DomainId>& children,
            std::pair<Domain, DomainId> node) -> int32_t {
      auto childDomain = assignmentTree.getChildDomain(node.first);
      int32_t val = metricMap.get(node.first, node.second).value_or(0);

      for (auto child : children) {
        bool isFutureChild = true;
        if (node.first == state_->getBinDomain()) {
          for (auto movementMap : movementMaps) {
            auto nextBin = movementMap->getNextBin(child);
            if (nextBin.has_value() && nextBin.value() != node.second) {
              isFutureChild = false;
              break;
            }
          }
        }

        auto metric = metricMap.get(childDomain, child);
        if (isFutureChild) {
          val += metric.value_or(0);
        } else {
          val -= metric.value_or(0);
        }
      }

      if (node.first == domain_) {
        int64_t prevWeight = 0;
        if (domainFaultMap_[node.second]) {
          prevWeight = domainFaultMap_[node.second] * faultWeight_;
        }

        if (val > capacity_) {
          int64_t excess = val - capacity;
          domainFaultMap_[node.second] = excess;

          auto currentWeight = excess * faultWeight_;
          auto weightDelta = currentWeight - prevWeight;
          addBinWeight(node.second, weightDelta);
        } else {
          domainFaultMap_[node.second] = 0;
        }
      }
      return val;
    };
    expressionTree_ = std::make_shared<ExpressionTree>(state_, metric_, domain_,
                                                       domainIds, func);
    commit();
  }

  virtual void canaryMoves(std::shared_ptr<MovementMap> committedMoves,
                           std::shared_ptr<MovementMap> canaryMoves) {
    /*
    for (auto [treeDomainId, expressionTree] : expressionTreeMap_) {
      expressionTree->canaryMoves(committedMoves, canaryMoves);
    }
    */
    expressionTree_->canaryMoves(committedMoves, canaryMoves);
  }

  virtual void commitMoves() {
    /*
    for (auto [treeDomainId, expressionTree] : expressionTreeMap_) {
      expressionTree->commitMoves();
    }
    */
    expressionTree_->commitMoves();
  }

  virtual int32_t getWeight(DomainId domainId) {
    // Special case for bin domain
    if (domain_ == state_->getBinDomain()) {
      auto it = domainFaultMap_.find(domainId);
      if (it == domainFaultMap_.end()) {
        return 0;
      }
      return it->second * faultWeight_;
    }

    int32_t totalWeight = 0;
    /*
    for (auto [treeDomainId, expressionTree] : expressionTreeMap_) {
      if (expressionTree->getAssignmentTree()->doesNodeExist(
              state_->getBinDomain(), domainId)) {
        totalWeight += domainFaultMap_[treeDomainId] * faultWeight_;
      }
    }
    */
    return totalWeight;
  }

 private:
  Domain domain_;
  Metric metric_;
  int32_t capacity_;
  int32_t faultWeight_;

  std::shared_ptr<ExpressionTree> expressionTree_;
  std::unordered_map<DomainId, std::shared_ptr<ExpressionTree>>
      expressionTreeMap_;
  std::unordered_map<DomainId, int32_t> domainFaultMap_;
};

}  // namespace psychopomp
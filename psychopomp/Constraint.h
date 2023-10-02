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
  virtual void commitMoves() = 0;
  virtual int32_t getWeight(DomainId domainId) = 0;
  virtual int32_t getTotalWeight() = 0;

 protected:
  void addBinWeight(DomainId domainId, int64_t binWeightDelta) {
    auto& binWeightInfo = state_->getBinWeightInfo();
    binWeightInfo.binWeightMap[domainId] += binWeightDelta;
    binWeightInfo.totalWeight += binWeightDelta;
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
    auto func = [&](const AssignmentTree& assignmentTree,
                    const std::vector<MovementMap&>& movementMaps,
                    const MetricsMap& metricMap,
                    const std::vector<DomainId>& children,
                    std::pair<Domain, DomainId> node) -> int32_t {
      auto childDomain = assignmentTree.getChildDomain(node.first);
      int32_t val = metricMap.getMetric(node.first, node.second).value_or(0);

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

        auto metric = metricMap.getMetric(childDomain, child);
        if (isFutureChild) {
          val += metric.value_or(0);
        } else {
          val -= metric.value_or(0);
        }
      }

      if (node.first == domain_) {
        if (domainFaultMap_[node.second]) {
          totalWeight_ -= domainFaultMap_[node.second] * faultWeight_;
          addBinWeight(node.second,
                       -domainFaultMap_[node.second] * faultWeight_);
        }

        if (val > capacity_) {
          auto excess = val - capacity;
          domainFaultMap_[node.second] = excess;
          totalWeight_ += excess * faultWeight_;
          addBinWeight(node.second, excess * faultWeight);
        } else {
          domainFaultMap_[node.second] = 0;
        }
      }
      return val;
    };
    expressionTree_ = std::make_shared<ExpressionTree>(state_, metric_, domain_,
                                                       domainIds, func);
    /*
    for (auto domainId : domainIds) {
      expressionTreeMap_[domainId] = std::make_shared<ExpressionTree>(
          state_, metric_, domain_, std::vector<DomainId>{domainId}, func);
    }
    */
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

  virtual int32_t getTotalWeight() { return totalWeight_; }

 private:
  std::shared_ptr<State> state_;
  Domain domain_;
  Metric metric_;
  int32_t capacity_;
  int32_t faultWeight_;

  std::shared_ptr<ExpressionTree> expressionTree_;
  std::unordered_map<DomainId, std::shared_ptr<ExpressionTree>>
      expressionTreeMap_;
  std::unordered_map<DomainId, int32_t> domainFaultMap_;
  int32_t totalWeight_;
};

}  // namespace psychopomp
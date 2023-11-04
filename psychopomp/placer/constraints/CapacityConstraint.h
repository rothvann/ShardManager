#pragma once

#include "psychopomp/placer/constraints/Constraint.h"

namespace psychopomp {

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
            const MetricsMap& metricMap,
            const std::vector<DomainId>& changedChildren,
            std::pair<Domain, DomainId> node) -> int32_t {
      auto childDomain = assignmentTree.getChildDomain(node.first);
      int64_t val = metricMap.get(node.first, node.second).value_or(0);

      for (auto child : changedChildren) {
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

        int32_t metric = 0;
        if (childDomain == state_->getShardDomain()) {
          metric = state_->getShardMetric(metric_, child);
        } else {
          metric =
              metricMap.get(childDomain, child).value_or(0) -
              metricMap.getFromCommittedMap(childDomain, child).value_or(0);
        }
        if (isFutureChild) {
          val += metric;
        } else {
          val -= metric;
        }
      }

      updateWeightIfPossible(node, val);
      return val;
    };
    expressionTree_ = std::make_shared<ExpressionTree>(state_, metric_, domain_,
                                                       domainIds, func);
    commit();
  }

  void canaryMoves(std::shared_ptr<MovementMap> committedMoves,
                   std::shared_ptr<MovementMap> canaryMoves) override {
    domainFaultMap_.clear();
    expressionTree_->canaryMoves(committedMoves, canaryMoves);
    auto& binWeightInfo = state_->getBinWeightInfo();
  }

  void commitMoves() override {
    expressionTree_->commitMoves();
    domainFaultMap_.commit();
  }

 private:
  void updateWeightIfPossible(std::pair<Domain, DomainId> node, int64_t val) {
    if (node.first == domain_) {
      int64_t prevWeight = domainFaultMap_.get(node.second).value_or(0);

      auto currentWeight = 0;
      if (val > capacity_) {
        currentWeight = (val - capacity_) * faultWeight_;
      }

      domainFaultMap_.set(true /* isCanary */, currentWeight, node.second);

      auto weightDelta = currentWeight - prevWeight;
      if (weightDelta == 0) {
        return;
      }

      if (domain_ == state_->getBinDomain()) {
        addBinWeight(node.second, weightDelta);
      } else {
        addTotalWeight(weightDelta);
      }
    }
  }

  std::string getName() const override { return "Metric Constraint"; }

  Domain domain_;
  Metric metric_;
  int32_t capacity_;
  int32_t faultWeight_;

  std::shared_ptr<ExpressionTree> expressionTree_;
  CommittableMap<std::unordered_map<DomainId, int64_t>, int64_t>
      domainFaultMap_;
};
}  // namespace psyschopomp
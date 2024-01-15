#pragma once

#include "psychopomp/placer/constraints/Constraint.h"

namespace psychopomp {

class DrainConstraint : public Constraint {
 public:
  DrainConstraint(std::shared_ptr<SolvingState> state, MovementConsistency consistency,
                  Domain domain, const std::vector<DomainId>& domainIds,
                  int32_t faultWeight)
      : Constraint(state),
        consistency_(consistency),
        domain_(domain),
        metric_(kShardCountMetric),
        faultWeight_(faultWeight) {
    auto func =
        [&](const SparseMappingTree& assignmentTree,
            const std::vector<std::shared_ptr<MovementMap>>& movementMaps,
            const MetricsMap& metricMap,
            const std::vector<DomainId>& changedChildren,
            std::pair<Domain, DomainId> node) -> int32_t {
      return calculate(assignmentTree, movementMaps, metricMap, changedChildren,
                       node);
    };
    expressionTree_ = std::make_shared<ExpressionTree>(state_, domain_,
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
  int32_t calculate(
      const SparseMappingTree& assignmentTree,
      const std::vector<std::shared_ptr<MovementMap>>& movementMaps,
      const MetricsMap& metricMap, const std::vector<DomainId>& changedChildren,
      std::pair<Domain, DomainId> node) {
    auto val =
        shardCountOperator(state_, assignmentTree, movementMaps, metricMap,
                           changedChildren, node, consistency_);
    updateWeightIfPossible(node, val);
    return val;
  }

  void updateWeightIfPossible(std::pair<Domain, DomainId> node, int64_t val) {
    if (node.first == domain_) {
      int64_t prevWeight = domainFaultMap_.get(node.second).value_or(0);

      auto currentWeight = val * faultWeight_;

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

  MovementConsistency consistency_;
  Domain domain_;
  Metric metric_;
  int32_t faultWeight_;

  std::shared_ptr<ExpressionTree> expressionTree_;
  CommittableMap<std::unordered_map<DomainId, int64_t>, int64_t>
      domainFaultMap_;
};
}  // namespace psychopomp
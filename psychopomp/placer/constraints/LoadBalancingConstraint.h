#pragma once

#include "psychopomp/placer/constraints/Constraint.h"

namespace psychopomp {

class LoadBalancingConstraint : public Constraint {
 public:
  LoadBalancingConstraint(std::shared_ptr<State> state, Domain domain,
                          const std::vector<DomainId>& domainIds, Metric metric,
                          int32_t maxDelta, double faultWeightMultiplier)
      : Constraint(state),
        domain_(domain),
        metric_(metric),
        maxDelta_(maxDelta),
        faultWeightMultiplier_(faultWeightMultiplier) {
    std::vector<std::vector<DomainId>> loadBalancingDomainIdMap = {domainIds};
    loadBalancingDomain_ = state->addDomain(domain_, loadBalancingDomainIdMap);
    auto minFunc =
        [&](const AssignmentTree& assignmentTree,
            const std::vector<std::shared_ptr<MovementMap>>& movementMaps,
            const MetricsMap& metricMap,
            const std::vector<DomainId>& changedChildren,
            std::pair<Domain, DomainId> node) -> int32_t {
      return calculate(assignmentTree, movementMaps, metricMap, changedChildren,
                       node, [](int32_t a, int32_t b) -> bool {
                         return std::less<int32_t>()(a, b);
                       });
    };
    auto maxFunc =
        [&](const AssignmentTree& assignmentTree,
            const std::vector<std::shared_ptr<MovementMap>>& movementMaps,
            const MetricsMap& metricMap,
            const std::vector<DomainId>& changedChildren,
            std::pair<Domain, DomainId> node) -> int32_t {
      return calculate(assignmentTree, movementMaps, metricMap, changedChildren,
                       node, [](int32_t a, int32_t b) -> bool {
                         return std::greater<int32_t>()(a, b);
                       });
    };
    minExpressionTree_ =
        std::make_shared<ExpressionTree>(state_, metric_, loadBalancingDomain_,
                                         std::vector<DomainId>{0}, minFunc);
    maxExpressionTree_ =
        std::make_shared<ExpressionTree>(state_, metric_, loadBalancingDomain_,
                                         std::vector<DomainId>{0}, maxFunc);
    updateWeights();
    commit();
  }

  void canaryMoves(std::shared_ptr<MovementMap> committedMoves,
                   std::shared_ptr<MovementMap> canaryMoves) override {
    domainUpdatedSet_.clear();
    domainFaultMap_.clear();
    minExpressionTree_->canaryMoves(committedMoves, canaryMoves);
    maxExpressionTree_->canaryMoves(committedMoves, canaryMoves);
    updateWeights();
  }

  void commitMoves() override {
    minExpressionTree_->commitMoves();
    maxExpressionTree_->commitMoves();
    domainFaultMap_.commit();
  }

 private:
  int32_t calculate(
      const AssignmentTree& assignmentTree,
      const std::vector<std::shared_ptr<MovementMap>>& movementMaps,
      const MetricsMap& metricMap, const std::vector<DomainId>& changedChildren,
      std::pair<Domain, DomainId> node,
      std::function<bool(int32_t, int32_t)> cmp) {
    auto domain = node.first;
    auto domainId = node.second;

    // Calculate tree node min / max
    auto children =
        assignmentTree.getChildren(node.first, node.second, movementMaps);
    auto childDomain = children.first;
    auto& childVector = children.second;

    if (domain == domain_) {
      domainUpdatedSet_.insert(domainId);
    }

    if (domain == state_->getBinDomain()) {
      int32_t sum = 0;
      for (auto child : childVector) {
        if (shouldConsiderShard(MovementConsistency::AFTER, movementMaps, child,
                                domainId)) {
          auto metric = state_->getShardMetric(metric_, child);
          sum += metric;
        }
      }
      return sum;
    } else {
      std::vector<int32_t> metrics;
      for (auto child : childVector) {
        auto metric = metricMap.get(childDomain, child);
        if (metric) {
          metrics.emplace_back(*metric);
        }
      }
      if (metrics.empty()) {
        return 0;
      }
      // Does not return min but applies our compare function
      return *std::min_element(metrics.begin(), metrics.end(), cmp);
    }
  };

  void updateWeights() {
    auto minMetric = minExpressionTree_->getMetricsMap()
                         .get(loadBalancingDomain_, 0)
                         .value_or(0);
    for (auto domainId : domainUpdatedSet_) {
      int64_t prevWeight = domainFaultMap_.get(domainId).value_or(0);
      auto maxMetric = maxExpressionTree_->getMetricsMap()
                           .get(domain_, domainId)
                           .value_or(0);
      auto delta = std::abs(maxMetric - minMetric);
      int64_t newWeight = 0;
      if (delta >= maxDelta_) {
        newWeight = (delta - maxDelta_) * faultWeightMultiplier_;
      }

      int64_t weightDelta = newWeight - prevWeight;
      addBinWeight(domainId, weightDelta);
    }
  }

  std::string getName() const override { return "Load Balancing Constraint"; }

  Domain domain_;
  Domain loadBalancingDomain_;
  Metric metric_;
  int32_t maxDelta_;
  double faultWeightMultiplier_;

  std::shared_ptr<ExpressionTree> minExpressionTree_;
  std::shared_ptr<ExpressionTree> maxExpressionTree_;

  std::unordered_set<DomainId> domainUpdatedSet_;
  CommittableMap<std::unordered_map<DomainId, int64_t>, int64_t>
      domainFaultMap_;
};
}  // namespace psychopomp
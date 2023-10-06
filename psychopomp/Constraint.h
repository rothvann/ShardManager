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
    addTotalWeight(binWeightDelta);
  }

  void addTotalWeight(int64_t weightDelta) {
    auto& binWeightInfo = state_->getBinWeightInfo();
    auto prevTotalWeight =
        binWeightInfo.totalWeight.getCommittedVal().value_or(0);
    binWeightInfo.totalWeight.set(true /* isCanary */,
                                  prevTotalWeight + weightDelta);
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

      updateWeightIfPossible(node, val);
      return val;
    };
    expressionTree_ = std::make_shared<ExpressionTree>(state_, metric_, domain_,
                                                       domainIds, func);
    commit();
  }

  virtual void canaryMoves(std::shared_ptr<MovementMap> committedMoves,
                           std::shared_ptr<MovementMap> canaryMoves) {
    expressionTree_->canaryMoves(committedMoves, canaryMoves);
  }

  virtual void commitMoves() { expressionTree_->commitMoves(); }

 private:
  void updateWeightIfPossible(std::pair<Domain, DomainId> node, int32_t val) {
    if (node.first == domain_) {
      int64_t prevWeight = domainFaultMap_[node.second] ? faultWeight_ : 0;

      if (val <= capacity_) {
        domainFaultMap_[node.second] = 0;
        return;
      }

      domainFaultMap_[node.second] = true;

      auto currentWeight = faultWeight_;
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

  Domain domain_;
  Metric metric_;
  int32_t capacity_;
  int32_t faultWeight_;

  std::shared_ptr<ExpressionTree> expressionTree_;
  std::unordered_map<DomainId, bool> domainFaultMap_;
};

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

      return val;
    };
  }

 private:
  Domain domain_;
  Metric metric_;
  int32_t maxDelta_;
  double faultWeightMultiplier_;

  std::shared_ptr<ExpressionTree> expressionTree_;

  std::unordered_map<DomainId, int32_t> domainMetricMap_;
  std::unordered_set<DomainId> domainUpdatedSet_;
};

}  // namespace psychopomp
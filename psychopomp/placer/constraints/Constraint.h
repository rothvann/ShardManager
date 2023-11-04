#pragma once

#include <iostream>
#include <memory>

#include "psychopomp/Types.h"
#include "psychopomp/placer/constraints/ConstraintUtils.h"
#include "psychopomp/placer/ExpressionTree.h"
#include "psychopomp/placer/State.h"

namespace psychopomp {

class Constraint {
 public:
  Constraint(std::shared_ptr<State> state)
      : state_(state) {}
  virtual void canaryMoves(std::shared_ptr<MovementMap> comittiedMoves,
                           std::shared_ptr<MovementMap> canaryMoves) = 0;

  void commit() {
    commitMoves();

    auto& binWeightInfo = state_->getBinWeightInfo();
    binWeightInfo.binWeightMap.commit();
    binWeightInfo.totalWeight.commit();
  }

 protected:
  virtual std::string getName() const = 0;
  virtual void commitMoves() = 0;

  void addBinWeight(DomainId domainId, int64_t binWeightDelta) {
    auto& binWeightInfo = state_->getBinWeightInfo();
    auto prevWeight = binWeightInfo.binWeightMap.get(domainId).value_or(0);
    binWeightInfo.binWeightMap.set(true /* isCanary */,
                                   prevWeight + binWeightDelta, domainId);
    addTotalWeight(binWeightDelta);
  }

  void addTotalWeight(int64_t weightDelta) {
    auto& binWeightInfo = state_->getBinWeightInfo();
    auto prevTotalWeight = binWeightInfo.totalWeight.get().value_or(0);
    binWeightInfo.totalWeight.set(true /* isCanary */,
                                  prevTotalWeight + weightDelta);
  }

  std::shared_ptr<State> state_;
};
}  // namespace psychopomp
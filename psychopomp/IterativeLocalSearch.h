#pragma once

#include <iostream>
#include <memory>

#include "folly/MapUtil.h"
#include "psychopomp/Constraint.h"
#include "psychopomp/ExpressionTree.h"
#include "psychopomp/State.h"
#include "psychopomp/utils/Committable.h"
#include "psychopomp/utils/RandomGenerator.h"

// Client
// cpu 50
// cpu capacity 200

// 0 - 1, 2, 3, 4
//
//
//

namespace psychopomp {

class IterativeLocalSearch {
 public:
  explicit IterativeLocalSearch(uint32_t maxIterations = 50000)
      : maxIterations_(maxIterations),
        randomGen_(random::SeededRandomGenerator()) {}

  std::shared_ptr<MovementMap> solve(
      std::shared_ptr<State> state,
      std::vector<std::shared_ptr<Constraint>> constraints) {
    std::shared_ptr<MovementMap> committedMoves =
        std::make_shared<MovementMap>();

    size_t numBins = state->getDomainSize(state->getBinDomain());

    std::vector<std::pair<int32_t, DomainId>> binWeights;
    binWeights.reserve(numBins);

    auto currentWeight =
        state->getBinWeightInfo().totalWeight.get().value_or(0);

    for (size_t iter = 0; iter < maxIterations_; iter++) {
      // Check for unassigned shards

      // Order by hottest shards
      binWeights.clear();
      for (DomainId binId = 0; binId < numBins; binId++) {
        binWeights.emplace_back(
            state->getBinWeightInfo().binWeightMap.get(binId).value_or(0),
            binId);
      }
      std::sort(binWeights.begin(), binWeights.end(),
                std::greater<std::pair<int32_t, DomainId>>());

      // Do move
      bool hasFoundMove = false;
      for (auto [weight, binId] : binWeights) {
        auto movement = getMovementForBin(state, committedMoves, constraints,
                                          binId, currentWeight);
        if (movement.has_value()) {
          committedMoves->addMovements(movement.value());
          for (auto constraint : constraints) {
            constraint->commit();
          }
          hasFoundMove = true;
          break;
        }
      }
      if (!hasFoundMove) {
        break;
      }
    }

    return committedMoves;
  }

  std::optional<std::shared_ptr<MovementMap>> getMovementForBin(
      std::shared_ptr<State> state, std::shared_ptr<MovementMap> committedMoves,
      std::vector<std::shared_ptr<Constraint>> constraints, DomainId binId,
      int64_t& currentWeight) {
    std::shared_ptr<MovementMap> canariedMoves =
        std::make_shared<MovementMap>();
    size_t numBins = state->getDomainSize(state->getBinDomain());
    // go through all shards
    auto& shards =
        state->getAssignmentTree()->getChild(state->getBinDomain(), binId);

    if (shards.size() == 0) {
      return std::nullopt;
    }

    for (size_t iter = 0; iter < 5; iter++) {
      auto shard = shards[randomGen_() % (shards.size())];
      // Check if shard still exists in bin
      auto nextBin = committedMoves->getNextBin(shard);
      if (nextBin.has_value() && nextBin.value() != binId) {
        continue;
      }

      // find next bin
      for (size_t binsChecked = 0; binsChecked < 2; binsChecked++) {
        DomainId nextBinId = randomGen_() % (numBins - 1);
        if (nextBinId == binId) {
          nextBinId++;
        }

        // Check if weight is lower
        canariedMoves->clearMovements();
        canariedMoves->addMovement(shard, nextBinId);
        updateWeights(constraints, committedMoves, canariedMoves);

        auto newWeight =
            state->getBinWeightInfo().totalWeight.get().value_or(0);
        if (newWeight < currentWeight) {
          currentWeight = newWeight;
          return canariedMoves;
        }
      }
    }
    return std::nullopt;
  }

  void updateWeights(std::vector<std::shared_ptr<Constraint>>& constraints,
                     std::shared_ptr<MovementMap> committedMoves,
                     std::shared_ptr<MovementMap> canaryMoves) {
    for (auto constraint : constraints) {
      constraint->canaryMoves(committedMoves, canaryMoves);
    }
  }

 private:
  uint32_t maxIterations_;
  std::mt19937 randomGen_;
};

}  // namespace psychopomp
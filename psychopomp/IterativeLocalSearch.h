#include <memory>

#include "Searcher.h"
#include "States.h"
#include "utils/StateUtils.h"

namespace psychopomp {

template <typename Searcher, typename CostFunc>
class IterativeLocalSearch {
 public:
  explicit IterativeLocalSearch(std::shared_ptr<Searcher> searcher,
                                std::shared_ptr<CostEstimator> CostEstimator,
                                uint32_t maxIterations = 50000)
      : searcher_(searcher),
        costEstimator_(CostEstimator),
        maxIterations_(maxIterations) {}

  AssignmentState solve(const AssignmentState& state) {
    // Create computation state
    auto computationState = utils::getComputationState(state);
    auto initialPlacements = computationState.shardPlacements;

    // Solve
    auto cost =
        costEstimator_->get(computationState.shardPlacements, computationState);
    for (size_t iteration = 0; iteration < maxIterations_; iteration++) {
      std::vector<uint32_t> newShardPlacements =
          searcher_->get(computationState);

      auto newCost = costEstimator_->get(newShardPlacements, computationState);
      if (newCost < cost) {
        cost = newCost;
        computationState.shardPlacements = newShardPlacements;
      }
    }

    // Convert computation state to assigment state
    auto assignmentState = utils::getAssignmentState(computationState);

    // Diff assignment states to get transitioning shards
    auto transitionMap =
        utils::getShardTransitionMap(initialPlacements, computationState);

    // Add unmoveable shards back to assignment state
    utils::addStaticShardsToAssignmentState(assignmentState, computationState);

    return assignmentState;
  }

 private:
  std::shared_ptr<Searcher> searcher_;
  std::shared_ptr<Solver> costEstimator_;
  uint32_t maxIterations_;
};

}  // namespace psychopomp
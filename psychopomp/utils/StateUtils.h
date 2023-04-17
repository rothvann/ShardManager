#include "psychopomp/States.h"

namespace psychopomp {
namespace utils {

ComputationState getComputationState(const AssignmentState& assignmentState) {
  ComputationState compState;

  std::unordered_map<uint64_t, size_t> binToIndex;

  std::vector<uint64_t> binUIDs;
  binUIDs.reserve(assignmentState.binUIDToNameMap.size());

  BinState initBinState;
  initBinState.metrics = std::array<uint32_t, 5>();
  std::vector<BinState> binStates(assignmentState.binUIDToNameMap.size(),
                                  initBinState);

  int index = 0;
  for (const auto& [bin, name] : assignmentState.binUIDToNameMap) {
    binUIDs.push_back(bin);
    binToIndex[bin] = index++;
  }

  std::vector<uint64_t> shardUIDs;
  std::vector<ShardState> shardStates;
  std::vector<uint32_t> shardPlacements;
  shardUIDs.reserve(assignmentState.shardStateMap.size());
  shardPlacements.reserve(assignmentState.shardStateMap.size());

  std::vector<uint64_t> staticShardUIDs;
  std::vector<uint32_t> staticShardPlacements;

  for (const auto& [bin, shards] : assignmentState.shardPlacementMap) {
    auto binIndex = binToIndex[bin];
    for (const auto shard : shards) {
      const auto& shardState = assignmentState.shardStateMap.at(shard);
      const auto& shardTransitionState =
          assignmentState.shardTransitionMap.at(shard);
      // Don't include transitioning shards in computation state
      if (shardTransitionState.status != TransitionStatus::NONE) {
        shardUIDs.push_back(shard);
        shardStates.push_back(shardState);
        shardPlacements.push_back(binIndex);
      } else {
        staticShardUIDs.push_back(shard);
        staticShardPlacements.push_back(binIndex);

        // Precompute bin metrics for transtioning shards
        for (size_t i = 0; i < shardState.metrics.size(); i++) {
          binStates[binIndex].metrics[i] += shardState.metrics[i];
        }
      }
    }
  }

  compState.binLimits = assignmentState.binLimits;

  compState.binUIDs = std::move(binUIDs);
  compState.binStates = std::move(binStates);
  compState.shardUIDs = std::move(shardUIDs);
  compState.shardStates = std::move(shardStates);
  compState.shardPlacements = std::move(shardPlacements);
  compState.staticShardUIDs = std::move(staticShardUIDs);
  compState.staticShardPlacements = std::move(staticShardPlacements);

  return compState;
}

ShardPlacementMap getShardPlacementMap(const ComputationState& compState) {
  ShardPlacementMap shardPlacements;
  for (size_t i = 0; i < compState.shardUIDs.size(); i++) {
    auto bin = compState.binUIDs[compState.shardPlacements[i]];
    shardPlacements[bin].push_back(compState.shardUIDs[i]);
  }

  return shardPlacements;
}

ShardTransitionMap getShardTransitionMap(
    const std::vector<uint32_t>& initialPlacements,
    const ComputationState& compState) {
  ShardTransitionMap shardTransitionMap;

  for (size_t i = 0; i < compState.shardUIDs.size(); i++) {
    auto shardUID = compState.shardUIDs[i];
    TransitionState transitionState;
    if (initialPlacements[i] != compState.shardPlacements[i]) {
      auto prevBin = compState.binUIDs[initialPlacements[i]];
      auto nextBin = compState.binUIDs[compState.shardPlacements[i]];

      transitionState.status = TransitionStatus::ADDING;
      transitionState.prevShardBin = prevBin;
    } else {
      transitionState.status = TransitionStatus::NONE;
    }

    shardTransitionMap[shardUID] = std::move(transitionState);
  }
  return shardTransitionMap;
}

void addStaticShardsToAssignmentState(AssignmentState& assignmentState,
                                      const ComputationState& compState) {
  for (size_t i = 0; i < compState.staticShardUIDs.size(); i++) {
    auto shardUID = compState.staticShardUIDs[i];
    auto bin = compState.binUIDs[compState.staticShardPlacements[i]];

    assignmentState.shardPlacementMap[bin].push_back(shardUID);
  }
}

}  // namespace utils
}  // namespace psychopomp
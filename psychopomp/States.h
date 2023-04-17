#include <array>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace psychopomp {

struct ShardId {
  uint32_t primaryId;
  uint32_t replicaId;
};

struct ShardState {
  std::array<uint32_t, 5> metrics;
};

struct BinState {
  std::array<uint32_t, 5> metrics;
};

struct BinLimits {
  std::array<uint32_t, 5> limits;
};

enum TransitionStatus {
  NONE = 0,    // Not transitioning
  ADDING = 1,  // Waiting for add shard to finish before swapping
  READY = 2,   // Ready to swap
};

struct TransitionState {
  TransitionStatus status;
  uint64_t prevShardBin;
};

typedef std::unordered_map<uint64_t, std::vector<uint64_t>> ShardPlacementMap;
typedef std::unordered_map<uint64_t, ShardState> ShardStateMap;
typedef std::unordered_map<uint64_t, TransitionState> ShardTransitionMap;

struct AssignmentState {
  std::unordered_map<uint64_t, std::string> binUIDToNameMap;
  std::unordered_map<uint64_t, ShardId> shardUIDToIdMap;

  ShardPlacementMap shardPlacementMap;
  ShardStateMap shardStateMap;
  ShardTransitionMap shardTransitionMap;

  BinLimits binLimits;
};

struct ComputationState {
  // 1 : 1 relationship for all bin vectors
  std::vector<uint64_t> binUIDs;
  std::vector<BinState> binStates;

  // Contains only moveable shards
  // 1 : 1 relationship for all shard vectors
  std::vector<uint64_t> shardUIDs;
  // Contains index of binUIDs
  std::vector<uint32_t> shardPlacements;
  std::vector<ShardState> shardStates;

  BinLimits binLimits;

  // For static shards which are transitioning or should not be moved
  // 1 : 1 relationship
  std::vector<uint64_t> staticShardUIDs;
  std::vector<uint32_t> staticShardPlacements;
};

}  // namespace psychopomp

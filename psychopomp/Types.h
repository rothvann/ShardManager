#pragma once

#include <cstdlib>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace psychopomp {

typedef size_t Domain;
typedef size_t DomainId;
typedef size_t Metric;

struct RangeJoinInfo {};

struct RangeSplitInfo {};

enum class MutationType {
  JOIN,
  SPLIT,
};

enum class RangeState {
  NORMAL,
  MUTATING,
  UNKNOWN,
};

struct RangeMutation {
  std::vector<std::pair<int64_t, int64_t>> currentRanges;
  std::vector<std::pair<int64_t, int64_t>> futureRanges;
};

struct AllocatorOutput {
  std::vector<std::pair<int64_t, int64_t>> shardRanges;
  std::vector<RangeMutation> shardMutations;
};

struct AllocatorInput {
  std::vector<std::pair<int64_t, int64_t>> shardRanges;
  std::vector<RangeState> shardRangeStates;

  std::vector<size_t> shardToRangeIndex;

  std::vector<RangeMutation> prevShardMutations;
  std::unordered_map<Metric, std::vector<int32_t>> metricVectorMap_;
};

struct PlacerOutput {
  std::vector<std::pair<DomainId, std::pair<DomainId, DomainId>>>
      shardMovements;
};

struct PlacerInput {
  std::vector<int64_t> shardRanges;
  std::vector<size_t> shardToRangeIndex;
  std::vector<bool> isPrimaryShard;
  std::unordered_map<Metric, std::vector<int32_t>> metricVectorMap_;

  std::vector<std::vector<DomainId>> binToShardIds;
  std::vector<RangeMutation> shardMutations;

  std::vector<std::pair<DomainId, std::pair<DomainId, DomainId>>>
      shardMovements;
};

}  // namespace psychopomp
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

union RangeMutation {
  RangeJoinInfo joinInfo;
  RangeSplitInfo splitInfo;
};

struct RangeMutationInfo {
  enum MutationType {
    JOIN,
    SPLIT,
  };
  RangeMutation mutation;
};

struct AllocatorOutput {
  std::vector<RangeMutationInfo> shardMutations;
};

struct AllocatorInput {
  /*
  Each adjacent pair of elements is a range. Consider [0, 5, 15, 35], the pairs
  are: [0, 4] [5, 14] [15, 34] and the whole range is [0, 34]
  */
  std::vector<int64_t> shardRanges;

  // Vector contains metrics for each shard in shardToRangeIndex. Sizes should
  // match.
  std::vector<size_t> shardToRangeIndex;
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
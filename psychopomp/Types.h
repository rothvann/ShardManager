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

struct AllocatorInput {
  Output previousOutput;

  // Vector contains metrics for each shard in the previous range. Sizes should
  // match.
  std::unordered_map<Metric, std::vector<int32_t>> metricVectorMap_;
};

struct AllocatorOutput {
  /*
  Each adjacent pair of elements is a range. Consider [0, 5, 15, 35], the pairs
  are: [0, 4] [5, 14] [15, 34] and the whole range is [0, 34]
  */
  std::vector<int64_t> shardRanges;
};

}  // namespace psychopomp
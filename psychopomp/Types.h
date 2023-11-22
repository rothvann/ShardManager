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
typedef size_t ShardRangeId;
typedef size_t ReplicaId; 

const DomainId kDefaultBin = 0;
const DomainId kDefaultDomain = 0;
const Metric kShardCountMetric = 0;

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

struct ShardInfo {
  ShardRangeId shardRangeId; 
  ReplicaId replicaId;
};


}  // namespace psychopomp
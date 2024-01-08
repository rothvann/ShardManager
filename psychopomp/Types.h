#pragma once

#include <cstdlib>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ServiceDiscovery.pb.h"

namespace psychopomp {

typedef size_t Domain;
typedef size_t DomainId;
typedef size_t Metric;

typedef std::string BinName;
typedef std::string ServiceName;

struct ShardInfo {
  std::pair<int64_t, int64_t> shardRange;
  ShardRangeStatus shardStatus;
};

typedef size_t ShardRangeId;
typedef size_t ReplicaId;
struct MappedShardInfo {
  ShardRangeId shardRangeId;
  ReplicaId replicaId;
};

const DomainId kDefaultBin = 0;
const DomainId kDefaultDomain = 0;
const Metric kShardCountMetric = 0;

enum class MutationType {
  JOIN,
  SPLIT,
};

enum class RangeState {
  NORMAL,
  MUTATING,
  UNKNOWN,
};

struct RangeJoinInfo {};

struct RangeSplitInfo {};
}  // namespace psychopomp
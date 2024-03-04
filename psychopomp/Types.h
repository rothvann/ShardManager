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
typedef int32_t MetricValue;

typedef uint64_t BinId;
typedef uint64_t ServiceId;
typedef uint64_t ShardKey;
typedef uint64_t ShardId;

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

struct SolvingConfig {
  size_t replicationFactor;
};
}  // namespace psychopomp
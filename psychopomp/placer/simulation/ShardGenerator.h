#pragma once

#include "psychopomp/Types.h"
#include "psychopomp/placer/utils/RandomGenerator.h"

namespace psychopomp {

std::vector<ShardInfo> generateShardInfos(size_t numShardRanges,
                                          size_t numReplicas) {
  std::vector<ShardInfo> shardInfos;
  shardInfos.reserve(numShardRanges * numReplicas);
  for (ShardRangeId rangeId = 0; rangeId < numShardRanges; rangeId++) {
    for (ReplicaId replicaId = 0; replicaId < numReplicas; replicaId++) {
      shardInfos.emplace_back(ShardInfo{rangeId, replicaId});
    }
  }
  return shardInfos;
}
}  // namespace psychopomp
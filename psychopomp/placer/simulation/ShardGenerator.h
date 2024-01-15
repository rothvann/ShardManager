#pragma once

#include "psychopomp/Types.h"
#include "psychopomp/placer/utils/RandomGenerator.h"

namespace psychopomp {

std::vector<MappedShardInfo> generateMappedShards(size_t numShardRanges,
                                          size_t numReplicas) {
  std::vector<MappedShardInfo> shardInfos;
  shardInfos.reserve(numShardRanges * numReplicas);
  for (ShardRangeId rangeId = 0; rangeId < numShardRanges; rangeId++) {
    for (ReplicaId replicaId = 0; replicaId < numReplicas; replicaId++) {
      shardInfos.emplace_back(MappedShardInfo{rangeId});
    }
  }
  return shardInfos;
}
}  // namespace psychopomp
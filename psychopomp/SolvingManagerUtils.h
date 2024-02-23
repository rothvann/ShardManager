#pragma once

#include "psychopomp/Types.h"

namespace psychopomp {

std::vector<std::pair<ShardKey, ShardKey>> generateShardKeyRangeMap(
    size_t start, size_t end, size_t numShards) {
  ShardKey range = (end - start) / numShards;
  if (range <= 0) {
    // log error
    return {};
  }
  std::vector<std::pair<ShardKey, ShardKey>> shardKeyRangeMap;
  shardKeyRangeMap.reserve(numShards);
  shardKeyRangeMap.emplace_back(start, start + range + 1);
  for (size_t i = 0; i < numShards % range; i++) {
    shardKeyRangeMap.emplace_back(start, start + range + 1);
    start += range + 2;
  }
  while (shardKeyRangeMap.back().second < end) {
    shardKeyRangeMap.emplace_back(shardKeyRangeMap.back().second + 1,
                                  shardKeyRangeMap.back().second + 1 + range);
  }
  return shardKeyRangeMap;
}

void mapBinAndShards(
    std::unordered_map<BinId, std::vector<ShardInfo>>& binMapping) {
    
    }

}  // namespace psychopomp
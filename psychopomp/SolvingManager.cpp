#include "psychopomp/SolvingManager.h"

#include "folly/MapUtil.h"

namespace psychopomp {
void SolvingManager::updateAll() {
  auto services = mappingProvider_->getServiceMappings();
  for (auto& [service, bins] : services) {
    for (auto& [binName, shardInfos] : bins) {
    }
  }
}

void SolvingManager::update(
    ServiceName& svcName,
    std::unordered_map<BinName, std::vector<std::pair<ShardKey, ShardKey>>>&
        shardMappings) {
  // Get or create default shard mapping
  auto shardKeyRangeMapping =
      folly::get_default(shardKeyRangeMappings_, svcName, nullptr);
  if (!shardKeyRangeMapping) {
    std::vector<std::pair<ShardKey, ShardKey>> shardKeyRangeMap;
    ShardKey start = 0;
    ShardKey end = 10000;
    size_t numShards = 100;
    ShardKey range = ((end - start) + 1) / numShards;
    if (range <= 0) {
      // log error
      return;
    }
    shardKeyRangeMap.reserve(numShards);
    shardKeyRangeMap.emplace_back(start,
                                  start + range + ((end - start) % numShards));
    while (shardKeyRangeMap.back().second < end) {
      shardKeyRangeMap.emplace_back(shardKeyRangeMap.back().second + 1,
                                    shardKeyRangeMap.back().second + 1 + range);
    }

    shardKeyRangeMappings_.emplace(
        svcName, std::make_shared<std::vector<std::pair<ShardKey, ShardKey>>>(
                     shardKeyRangeMap));
    shardKeyRangeMapping =
        folly::get_default(shardKeyRangeMappings_, svcName, nullptr);
  }

  // Map all shards
  std::vector<std::pair<ShardKey, ShardKey>> unknownShardRanges;
  
  std::vector<MappedShardInfo> shardInfoVector;
  std::vector<std::vector<DomainId>> binShardMapping(shardMappings.size());
  size_t binCount = 0;
  for (auto& [binName, shardInfos] : shardMappings) {
    binShardMapping[binCount].reserve(shardInfos.size());
    for (auto& shardRange : shardInfos) {
      auto closestShard =
          std::lower_bound(shardKeyRangeMapping->begin(),
                           shardKeyRangeMapping->end(), shardRange);
      if (closestShard == shardKeyRangeMapping->end() ||
          *closestShard != shardRange) {
        // Log smth mb
        unknownShardRanges.push_back(shardRange);
        continue;
      }
      MappedShardInfo mappedShardInfo;
      mappedShardInfo.shardRangeId =
          (closestShard - shardKeyRangeMapping->begin());
      shardInfoVector.push_back(mappedShardInfo);
      
      binShardMapping[binCount].push_back(shardInfoVector.size() - 1);
    }
    binCount++;
  }
  
  // Create SolvingState
  

}
}  // namespace psychopomp
#include "psychopomp/SolvingManager.h"

#include <limits>

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
  populateShardKeyRangeMap(svcName);
  auto shardKeyRangeMapping =
      folly::get_default(shardKeyRangeMappings_, svcName, nullptr);
  if (!shardKeyRangeMapping) {
    // log failed to populate

    return;
  }

  // Map all shards
  std::vector<std::pair<ShardKey, ShardKey>> unknownShardRanges;

  std::vector<MappedShardInfo> shardInfoVector;
  std::vector<std::vector<DomainId>> binShardMapping(shardMappings.size() + 1);
  size_t binCount = 0;
  for (auto& [binName, shardInfos] : shardMappings) {
    binCount++;
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
  }

  // Count shards
  // Remove / add

  // Create SolvingState
  std::make_shared<SolvingState> solvingState(std::move(shardInfoVector))
}

void SolvingManager::populateSolvingConfig(ServiceName& svcName) {
  // Dummy code
  // Need to retrieve from s3
}

void SolvingManager::populateShardKeyRangeMap(ServiceName& svcName) {
  auto shardKeyRangeMapping =
      folly::get_default(shardKeyRangeMappings_, svcName, nullptr);

  // Dummy code
  // Need to retrieve previous map from s3
  if (!shardKeyRangeMapping) {
    std::vector<std::pair<ShardKey, ShardKey>> shardKeyRangeMap;
    ShardKey start = 0;
    ShardKey end = std::numeric_limits<int64_t>::max();
    size_t numShards = 100;
    ShardKey range = (end - start) / numShards;
    if (range <= 0) {
      // log error
      return;
    }
    shardKeyRangeMap.reserve(numShards);
    shardKeyRangeMap.emplace_back(start,
                                  start + range + ((numShards % range));
    while (shardKeyRangeMap.back().second < end) {
      shardKeyRangeMap.emplace_back(shardKeyRangeMap.back().second + 1,
                                    shardKeyRangeMap.back().second + 1 + range);
    }

    shardKeyRangeMappings_.emplace(
        svcName, std::make_shared<std::vector<std::pair<ShardKey, ShardKey>>>(
                     shardKeyRangeMap));
  }
}

void mapShardsToKeyRanges(
    ServiceName& svcName,
    std::unordered_map<BinName, std::vector<std::pair<ShardKey, ShardKey>>>&
        shardMappings);
}  // namespace psychopomp
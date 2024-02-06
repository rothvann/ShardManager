#include "psychopomp/SolvingManager.h"

#include <limits>

#include "folly/MapUtil.h"

namespace {
std::vector<std::pair<psychopomp::ShardKey, psychopomp::ShardKey>>
generateShardKeyRangeMap(size_t start, size_t end, size_t numShards) {
  ShardKey range = (end - start) / numShards;
  if (range <= 0) {
    // log error
    return {};
  }
  shardKeyRangeMap.reserve(numShards);
      shardKeyRangeMap.emplace_back(start,
                                    start + range + ((numShards % range));
  while (shardKeyRangeMap.back().second < end) {
    shardKeyRangeMap.emplace_back(shardKeyRangeMap.back().second + 1,
                                  shardKeyRangeMap.back().second + 1 + range);
  }
  return shardKeyRangeMap;
}

void mapBinAndShards(std::unordered_map<BinName, std::vector<ShardInfo>& binMapping, )
}  // namespace

namespace psychopomp {
void SolvingManager::updateAll() {
  binMappings_ = mappingProvider_->getServiceMappings();
  for (auto& [service, bins] : binMappings_) {
    for (auto& [binName, shardInfos] : bins) {
    }
  }
}

void SolvingManager::populateServiceConfig(
    std::unordered_set<ServiceName>& serviceNames) {
  // Dummy code
  // Need to retrieve from s3
}

void SolvingManager::populateShardKeyRangeMap(
    std::unordered_set<ServiceName>& serviceNames) {
  for (auto& svcName : serviceNames) {
    auto shardKeyRangeMapping =
        folly::get_default(shardKeyRangeMappings_, svcName, nullptr);

    // Dummy code
    // Need to retrieve previous map from s3
    if (!shardKeyRangeMapping) {
      std::vector<std::pair<ShardKey, ShardKey>> shardKeyRangeMap;

      shardKeyRangeMappings_.emplace(
          svcName, std::make_shared<std::vector<std::pair<ShardKey, ShardKey>>>(
                       generateShardKeyRangeMap(
                           0, std::numeric_limits<int64_t>::max(), 100)));
    }
  }
}

void SolvingManager::createSolvingState(
    std::unordered_set<ServiceName>& serviceNames) {
  solvingStates_.clear();
  solvingStates_.reserve(serviceNames.size());

  for (auto& svcName : serviceNames) {
    auto shardKeyRangeMapping =
        folly::get_default(shardKeyRangeMappings_, svcName, nullptr);
    auto binMapping = folly::get_ptr(binMappings_, svcName);
    if (!shardKeyRangeMapping || !binMapping) {
      // log failed
      continue;
    }

    // Map all shards
    auto shardInfoVector = std::make_shared<std::vector<MappedShardInfo>>();
    auto binMappedShards = std::make_shared<std::vector<std::vector<DomainId>>>(
        binMapping->size() + 1);
    
    size_t binCount = 0;
    for (auto& [binName, shardInfos] : *binMapping) {
      binCount++;
      (*binMappedShards)[binCount].reserve(shardInfos.size());
      for (auto& shardInfo : shardInfos) {
        auto closestShard =
            std::lower_bound(shardKeyRangeMapping->begin(),
                             shardKeyRangeMapping->end(), shardInfo.shardRange);
        if (closestShard == shardKeyRangeMapping->end() ||
            *closestShard != shardInfo.shardRange) {
          // Log smth mb
          // We ignore
          continue;
        }
        MappedShardInfo mappedShardInfo;
        mappedShardInfo.shardRangeId =
            (closestShard - shardKeyRangeMapping->begin());
        shardInfoVector->push_back(mappedShardInfo);

        (*binMappedShards)[binCount].push_back(shardInfoVector->size());
      }
    }

    solvingStates_[svcName] = std::make_shared<SolvingState>(
        shardInfoVector, std::vector<<std::vector<std::vector<DomainId>>>(), std::vector<std::vector<DomainId>>(), binMappedShards,
        std::vector<std::vector<Metric>>());
  }
}
}  // namespace psychopomp
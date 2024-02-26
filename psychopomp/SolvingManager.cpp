#include "psychopomp/SolvingManager.h"

#include <limits>

#include "folly/MapUtil.h"
#include "psychopomp/NodeMapper.h"
#include "psychopomp/SolvingManagerUtils.h"

namespace psychopomp {

void SolvingManager::updateAll() {
  binMappings_ = mappingProvider_->getServiceMappings();
  for (auto& [service, bins] : binMappings_) {
    for (auto& [binName, shardInfos] : bins) {
    }
  }
}

void SolvingManager::populateServiceConfig(
    std::unordered_set<ServiceId>& serviceIds) {
  // Dummy code
  // Need to retrieve from s3
}

void SolvingManager::populateShardKeyRangeMap(
    std::unordered_set<ServiceId>& serviceIds) {
  for (auto& svcId : serviceIds) {
    auto shardKeyRangeMapping =
        folly::get_default(shardKeyRangeMappings_, svcId, nullptr);

    // Dummy code
    // Need to retrieve previous map from s3
    if (!shardKeyRangeMapping) {
      std::vector<std::pair<ShardKey, ShardKey>> shardKeyRangeMap;

      shardKeyRangeMappings_.emplace(
          svcId, std::make_shared<std::vector<std::pair<ShardKey, ShardKey>>>(
                     generateShardKeyRangeMap(
                         0, std::numeric_limits<int64_t>::max(), 100)));
    }
  }
}

void SolvingManager::createSolvingState(
    std::unordered_set<ServiceId>& serviceIds) {
  solvingStates_.clear();
  solvingStates_.reserve(serviceIds.size());

  for (auto& svcId : serviceIds) {
    auto shardKeyRangeMapping =
        folly::get_default(shardKeyRangeMappings_, svcId, nullptr);
    auto binMapping = folly::get_ptr(binMappings_, svcId);
    if (!shardKeyRangeMapping || !binMapping) {
      // log failed
      continue;
    }

    // Map all shards
    auto& binDomainIdMapping = binDomainIdMappings_[svcId];
    binDomainIdMapping.clear();
    std::vector<MappedShardInfo> shardInfoVector;
    std::vector<std::vector<DomainId>> binMappedShards(binMapping->size() + 1);

    size_t binCount = 0;
    for (auto& [binName, shardInfos] : *binMapping) {
      binCount++;
      binMappedShards[binCount].reserve(shardInfos.size());
      binDomainIdMapping[binCount] = binName;
      for (auto& shardInfo : shardInfos) {
        auto rangePair = std::make_pair<ShardKey, ShardKey>(
            shardInfo.range().start(), shardInfo.range().end());
        auto closestShard =
            std::lower_bound(shardKeyRangeMapping->begin(),
                             shardKeyRangeMapping->end(), rangePair);
        if (closestShard == shardKeyRangeMapping->end() ||
            *closestShard != rangePair) {
          // Log smth mb
          // We ignore
          continue;
        }
        MappedShardInfo mappedShardInfo;
        mappedShardInfo.shardRangeId =
            (closestShard - shardKeyRangeMapping->begin());
        shardInfoVector.push_back(mappedShardInfo);

        binMappedShards[binCount].push_back(shardInfoVector.size());
      }
    }

    solvingStates_[svcId] = std::make_shared<SolvingState>(
        std::make_unique<std::vector<MappedShardInfo>>(
            std::move(shardInfoVector)),
        std::make_unique<std::vector<std::vector<DomainId>>>(),
        std::make_unique<std::vector<std::vector<MetricValue>>>(),
        std::vector<std::vector<std::vector<DomainId>>>(), binMappedShards);
  }
}
}  // namespace psychopomp
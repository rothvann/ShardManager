#include "psychopomp/SolvingManager.h"

#include <limits>

#include "folly/MapUtil.h"
#include "psychopomp/SolvingManagerUtils.h"
#include "psychopomp/placer/utils/RandomGenerator.h"

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

void SolvingManager::populateShardMap(
    std::unordered_set<ServiceId>& serviceIds) {
  for (auto& svcId : serviceIds) {
    auto shardMapping = folly::get_optional(shardMappings_, svcId);

    // Dummy code
    // Need to retrieve previous map from s3
    if (!shardMapping) {
      std::vector<std::pair<ShardKey, ShardKey>> shardKeyRanges =
          generateShardKeyRangeMap(0, std::numeric_limits<int64_t>::max(), 100);
      std::unordered_map<ShardId, Shard> tempShardMapping;
      auto randomGen = random::SeededRandomGenerator<std::mt19937_64>();
      for (auto& range : shardKeyRanges) {
        // Generate random ids
        auto id = randomGen();

        Shard shard;
        shard.set_id(id);
        shard.set_type(ShardType::SHARD_TYPE_SECONDARY);
        shard.mutable_range()->set_start(range.first);
        shard.mutable_range()->set_end(range.second);

        tempShardMapping.emplace(id, std::move(shard));
      }

      shardMappings_.emplace(svcId, std::move(tempShardMapping));
    }
  }
}

void SolvingManager::populateShardMetricsMap(std::unordered_set<ServiceId>& serviceIds) {
  // Do nothing for now
}

void SolvingManager::createSolvingState(
    std::unordered_set<ServiceId>& serviceIds) {
  solvingStates_.clear();
  solvingStates_.reserve(serviceIds.size());

  for (auto& svcId : serviceIds) {
    auto* shardMapping = folly::get_ptr(shardMappings_, svcId);
    auto binMapping = folly::get_ptr(binMappings_, svcId);
    auto& shardMetricsMapping = shardMetricsMappings_[svcId];
    if (!shardMapping || !binMapping) {
      // log failed
      continue;
    }

    auto& svcConfig = serviceConfig_[svcId];
    // Map all shards
    auto nodeMapper = std::make_shared<NodeMapper>(
        svcConfig.node_config(), svcConfig.metric_config(), *binMapping,
        *shardMapping, shardMetricsMapping);
    nodeMappers_[svcId] = nodeMapper;

    solvingStates_[svcId] = std::make_shared<SolvingState>(
        nodeMapper->getShardInfoVector(), nodeMapper->getMetricVectors(),
        nodeMapper->getNodeMapping());
  }
}
}  // namespace psychopomp
#include "psychopomp/SolvingManager.h"

#include <limits>

#include "folly/MapUtil.h"
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

    auto& svcConfig = serviceConfig_[svcId];
    // Map all shards
    auto nodeMapper = std::make_shared<NodeMapper>(
        svcConfig.node_config(), svcConfig.metric_config(), *binMapping,
        *shardKeyRangeMapping);
    nodeMappers_[svcId] = nodeMapper;

    solvingStates_[svcId] = std::make_shared<SolvingState>(
        nodeMapper->getShardInfoVector(), nodeMapper->getMetricVectors(),
        nodeMapper->getNodeMapping());
  }
}
}  // namespace psychopomp
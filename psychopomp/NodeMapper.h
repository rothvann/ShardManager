#pragma once

#include "CamferConfig.pb.h"
#include "ServiceDiscovery.pb.h"
#include "folly/MapUtil.h"
#include "folly/Optional.h"
#include "psychopomp/ServiceMappingProvider.h"
#include "psychopomp/Types.h"
#include "psychopomp/placer/SolvingState.h"

namespace psychopomp {

class NodeMapper {
 public:
  NodeMapper(
      const camfer::NodeConfig& nodeConfig,
      const camfer::MetricConfig& metricConfig,
      const std::unordered_map<BinId, BinInfo>& binMapping,
      const std::vector<std::pair<ShardKey, ShardKey>>& shardKeyRangeMapping)
      : shardInfos_(std::make_shared<std::vector<ShardInfo>>()),
        metricVectors_(
            std::make_shared<std::vector<std::vector<MetricValue>>>()),
        nodeMapping_(std::make_shared<
                     std::vector<std::vector<std::vector<DomainId>>>>()) {
    addLevelMapping(nodeConfig);
    createMetricMapping(metricConfig);
    createNodeMapping(binMapping, shardKeyRangeMapping);
  }

  folly::Optional<DomainId> getDomainId(BinId binName) {
    return folly::get_optional(domainIdBinMapping_, binName);
  }

  folly::Optional<BinId> getBinId(DomainId domainId) {
    return folly::get_optional(binDomainIdMapping_, domainId);
  }

  std::shared_ptr<std::vector<ShardInfo>> getShardInfoVector() {
    return shardInfos_;
  }

  std::shared_ptr<std::vector<std::vector<MetricValue>>> getMetricVectors() {
    return metricVectors_;
  }

  std::shared_ptr<std::vector<std::vector<std::vector<DomainId>>>>
  getNodeMapping() {
    return nodeMapping_;
  }

 private:
  DomainId getDomainId(const size_t domainIndex, const std::string& node) {
    auto& domainId = domainToNodeDomainIdMapping_[domainIndex][node];
    if (domainId == 0) {
      domainId = domainToNodeDomainIdMapping_.size();
    }
    return domainId;
  }

  void addLevelMapping(const camfer::NodeConfig& nodeConfig) {
    auto& domains = nodeConfig.domains();
    nodeMapping_->resize(domains.size() + 2);
    for (size_t domainIndex = 0; domainIndex < domains.size(); domainIndex++) {
      auto& nodes = nodeConfig.domain_nodes().at(domains[domainIndex]).nodes();
      (*nodeMapping_)[domainIndex].resize(nodes.size());
      for (auto& node : nodes) {
        auto& name = node.name();
        auto& children = node.children();
        auto parentId = getDomainId(domainIndex, name);
        for (auto& child : children) {
          auto childId = getDomainId(domainIndex + 1, child);
          (*nodeMapping_)[domainIndex][parentId].push_back(childId);
        }
      }
    }
  }

  void createMetricMapping(const camfer::MetricConfig& metricConfig) {
    auto& metrics = metricConfig.metrics();
    for (size_t metric = 0; metric < metrics.size(); metric++) {
      metricMapping_.push_back(metrics.at(metric));
    }
  }

  void createNodeMapping(
      const std::unordered_map<BinId, BinInfo>& binMapping,
      const std::vector<std::pair<ShardKey, ShardKey>>& shardKeyRangeMapping) {
    // Create bin mapping
    size_t binCount = 0;
    for (auto& [binId, binInfo] : binMapping) {
      binCount++;
      binDomainIdMapping_[binCount] = binId;
      domainIdBinMapping_[binId] = binCount;

      std::vector<DomainId> shardsInBin;

      // Create shard info
      for (auto& shardInfo : binInfo.shardInfos) {
        auto rangePair = std::make_pair<ShardKey, ShardKey>(
            shardInfo.shard().range().start(), shardInfo.shard().range().end());
        auto closestShard =
            std::lower_bound(shardKeyRangeMapping.begin(),
                             shardKeyRangeMapping.end(), rangePair);
        if (closestShard == shardKeyRangeMapping.end() ||
            *closestShard != rangePair) {
          // Log smth mb
          // We ignore
          continue;
        }
        shardInfos_->emplace_back(shardInfo);
        shardsInBin.push_back(shardInfos_->size() - 1);

        // Create shard metric info
        std::vector<folly::Optional<MetricValue>> metrics(
            metricMapping_.size());
        for (auto& metricKey : metricMapping_) {
          auto metricVal =
              folly::get_optional(shardInfo.metrics().metrics(), metricKey);
          // metrics.push_back(metricVal);
        }
        // metricVectors_->push_back(std::move(metrics));
      }
      // Map bin to nodes in map
    }
  }
  std::shared_ptr<std::vector<ShardInfo>> shardInfos_;
  std::shared_ptr<std::vector<std::vector<MetricValue>>> metricVectors_;
  std::shared_ptr<std::vector<std::vector<std::vector<DomainId>>>> nodeMapping_;

  std::vector<std::string> metricMapping_;
  std::unordered_map<DomainId, BinId> binDomainIdMapping_;
  std::unordered_map<BinId, DomainId> domainIdBinMapping_;

  std::unordered_map<size_t, std::unordered_map<std::string, DomainId>>
      domainToNodeDomainIdMapping_;
};
}  // namespace psychopomp
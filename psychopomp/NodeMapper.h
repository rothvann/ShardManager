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
      const std::unordered_map<ShardId, Shard>& shardMapping,
      const std::unordered_map<ShardId, ShardMetrics>& shardMetricsMapping)
      : shardInfos_(std::make_shared<std::vector<ShardInfo>>()),
        metricVectors_(
            std::make_shared<std::vector<std::vector<MetricValue>>>()),
        nodeMapping_(std::make_shared<
                     std::vector<std::vector<std::vector<DomainId>>>>()) {
    addDomainMapping(nodeConfig);
    createMetricMapping(metricConfig);
    createNodeMapping(binMapping, shardMapping, shardMetricsMapping);
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

  size_t getDomainIndex(const std::string& domain) {
    return folly::get_default(domainIndexMapping_, domain, -1);
  }

  const std::string& getDomain(const size_t index) {
    return indexDomainMapping_[index];
  }

  void addDomainMapping(const camfer::NodeConfig& nodeConfig) {
    // Map indices backwards so that 0, 1 are shard / bin domains
    auto& domains = nodeConfig.domains();
    size_t startingIndex = domains.size() + 1;
    for (auto& domain : domains) {
      indexDomainMapping_[startingIndex] = domain;
      domainIndexMapping_[domain] = startingIndex;
      startingIndex--;
    }

    nodeMapping_->resize(domains.size() + 2);
    for (auto& domain : domains) {
      auto domainIndex = getDomainIndex(domain);
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
      const std::unordered_map<ShardId, Shard>& shardMapping,
      const std::unordered_map<ShardId, ShardMetrics>& shardMetricsMapping) {
    // Create bin mapping
    size_t domainId = 0;
    for (auto& [binId, binInfo] : binMapping) {
      domainId++;
      binDomainIdMapping_[domainId] = binId;
      domainIdBinMapping_[binId] = domainId;

      std::vector<DomainId> shardsInBin;

      // Create shard info
      for (auto& shardInfo : binInfo.shardInfos) {
        // Check shard id matches: key pair, primary / secondary
        auto* shard = folly::get_ptr(shardMapping, shardInfo.shard().id());
        auto& clientShard = shardInfo.shard();
        if (!shard || shard->type() != clientShard.type() ||
            shard->range().start() != clientShard.range().start() ||
            shard->range().end() != clientShard.range().end()) {
          // Log shard mismatch
          continue;
        }

        shardInfos_->emplace_back(shardInfo);
        shardsInBin.push_back(shardInfos_->size() - 1);

        // Create shard metric info
        std::vector<MetricValue> metrics(metricMapping_.size());

        auto prevMetrics =
            folly::get_optional(shardMetricsMapping, shard->id());
        for (auto& metricKey : metricMapping_) {
          auto metricVal =
              folly::get_optional(shardInfo.metrics().metrics(), metricKey);
          if (!metricVal && prevMetrics) {
            auto it = prevMetrics->metrics().find(metricKey);
            if (it != prevMetrics->metrics().end()) {
              metricVal = it->second;
            }
          }
          metrics.emplace_back(metricVal.value_or(0));
        }
        metricVectors_->emplace_back(std::move(metrics));
      }

      // Map bin to nodes in map
      const auto& binParentDomain = getDomain(2);
      auto* binNode = folly::get_ptr(binInfo.domainToNodesMap, binParentDomain);
      if (!binNode || binNode->empty()) {
        continue;
      }
      auto parentId = getDomainId(2, *binNode);
      (*nodeMapping_)[2][parentId].push_back(domainId);
    }
  }

  std::shared_ptr<std::vector<ShardInfo>> shardInfos_;
  std::shared_ptr<std::vector<std::vector<MetricValue>>> metricVectors_;
  std::shared_ptr<std::vector<std::vector<std::vector<DomainId>>>> nodeMapping_;

  std::vector<std::string> metricMapping_;
  std::unordered_map<DomainId, BinId> binDomainIdMapping_;
  std::unordered_map<BinId, DomainId> domainIdBinMapping_;

  std::unordered_map<size_t, std::string> indexDomainMapping_;
  std::unordered_map<std::string, size_t> domainIndexMapping_;
  std::unordered_map<size_t, std::unordered_map<std::string, DomainId>>
      domainToNodeDomainIdMapping_;
};
}  // namespace psychopomp
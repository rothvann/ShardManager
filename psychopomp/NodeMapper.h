#pragma once

#include "CamferConfig.pb.h"
#include "ServiceDiscovery.pb.h"
#include "folly/MapUtil.h"
#include "folly/Optional.h"
#include "psychopomp/Types.h"

namespace psychopomp {
namespace {
class NodeTrie {
 public:
  NodeTrie& addNode(const std::string& node, DomainId domainId, Domain domain) {
    auto& child = children_[node];
    child.domain_ = domain;
    child.domainId_ = domainId;

    return child;
  }

  folly::Optional<std::vector<std::pair<Domain, DomainId>>> getDomainPairs(
      std::vector<std::string>& nodes) {
    std::vector<std::pair<Domain, DomainId>> pairVec;
    pairVec.reserve(nodes.size());

    auto* currentNode = this;
    for (auto& node : nodes) {
      currentNode = folly::get_ptr(currentNode->children_, node);
      if (!currentNode) {
        return folly::none;
      }
      pairVec.emplace_back(currentNode->domain_, currentNode->domainId_);
    }
    return pairVec;
  }

 private:
  std::unordered_map<std::string, NodeTrie> children_;
  Domain domain_;
  DomainId domainId_;
};
}  // namespace

class NodeMapper {
 public:
  NodeMapper(
      const camfer::NodeConfig& nodeConfig,
      const camfer::MetricConfig& metricConfig,
      const std::unordered_map<BinId, BinInfo>& binMapping,
      const std::vector<std::pair<ShardKey, ShardKey>>& shardKeyRangeMapping)
      : mappedShardInfos_(std::make_shared<std::vector<MappedShardInfo>>()),
        metricVectors_(
            std::make_shared<std::vector<std::vector<MetricValue>>>()),
        nodeMapping_(std::make_shared<
                     std::vector<std::vector<std::vector<DomainId>>>>()) {
    auto& levels = nodeConfig.node_levels();
    auto& nodeChildrenVector = nodeConfig.node_children_vector();
    nodeMapping_->resize(levels.size() + 2);
    for (size_t levelIndex = 0; levelIndex < levels.size(); levelIndex++) {
      (*nodeMapping_)[levelIndex].resize(
          nodeChildrenVector.at(levels.at(levelIndex)).children().size());
    }

    std::unordered_map<size_t, size_t> levelToIndexMap;
    addLevelMapping(nodeConfig, levelToIndexMap, nodeTrie_, 0);
    createMetricMapping(metricConfig);
    createNodeMapping(binMapping, shardKeyRangeMapping);
  }

  folly::Optional<DomainId> getDomainId(BinId binName) {
    return folly::get_optional(domainIdBinMapping_, binName);
  }

  folly::Optional<BinId> getBinId(DomainId domainId) {
    return folly::get_optional(binDomainIdMapping_, domainId);
  }

  std::shared_ptr<std::vector<MappedShardInfo>> getMappedShardInfoVector() {
    return mappedShardInfos_;
  }

  std::shared_ptr<std::vector<std::vector<MetricValue>>> getMetricVectors() {
    return metricVectors_;
  }

  std::shared_ptr<std::vector<std::vector<std::vector<DomainId>>>>
  getNodeMapping() {
    return nodeMapping_;
  }

 private:
  void addLevelMapping(const camfer::NodeConfig& nodeConfig,
                       std::unordered_map<size_t, size_t>& levelToIndexMap,
                       NodeTrie& nodeTrie, size_t levelIndex) {
    auto& levels = nodeConfig.node_levels();
    if (levelIndex >= levels.size()) {
      return;
    }
    auto& level = levels[levelIndex];
    auto childrenIndex = levelIndex <= 0 ? 0 : levelToIndexMap[levelIndex - 1];
    auto& nodeChildrenVector = nodeConfig.node_children_vector();
    auto& nodes =
        nodeChildrenVector.at(level).children().at(childrenIndex).nodes();

    for (auto& node : nodes) {
      auto& nextNode = nodeTrie.addNode(node, levelIndex + 1,
                                        levelToIndexMap[levelIndex] + 1);
      (*nodeMapping_)[levelIndex][childrenIndex].push_back(
          levelToIndexMap[levelIndex] + 1);
      addLevelMapping(nodeConfig, levelToIndexMap, nextNode, levelIndex + 1);
      levelToIndexMap[levelIndex]++;
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
            shardInfo.range().start(), shardInfo.range().end());
        auto closestShard =
            std::lower_bound(shardKeyRangeMapping.begin(),
                             shardKeyRangeMapping.end(), rangePair);
        if (closestShard == shardKeyRangeMapping.end() ||
            *closestShard != rangePair) {
          // Log smth mb
          // We ignore
          continue;
        }
        MappedShardInfo mappedShardInfo;
        mappedShardInfo.shardRangeId =
            (closestShard - shardKeyRangeMapping.begin());
        mappedShardInfos_->push_back(mappedShardInfo);

        shardsInBin.push_back(mappedShardInfos_->size() - 1);

        // Create shard metric info
        std::vector<MetricValue> metrics(metricMapping_.size());
        for (auto& metricKey : metricMapping_) {
          auto metricVal = static_cast<MetricValue>(
              folly::get_default(shardInfo.metrics().metrics(), metricKey, 0));
          metrics.push_back(metricVal);
        }
        metricVectors_->push_back(std::move(metrics));
      }
      // Map bin to nodes in map
    }
  }
  std::shared_ptr<std::vector<MappedShardInfo>> mappedShardInfos_;
  std::shared_ptr<std::vector<std::vector<MetricValue>>> metricVectors_;
  std::shared_ptr<std::vector<std::vector<std::vector<DomainId>>>> nodeMapping_;

  std::vector<std::string> metricMapping_;
  std::unordered_map<DomainId, BinId> binDomainIdMapping_;
  std::unordered_map<BinId, DomainId> domainIdBinMapping_;
  NodeTrie nodeTrie_;
};
}  // namespace psychopomp
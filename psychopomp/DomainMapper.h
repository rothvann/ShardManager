#pragma once

#include "CamferConfig.pb.h"
#include "ServiceDiscovery.pb.h"
#include "folly/MapUtil.h"
#include "folly/Optional.h"
#include "psychopomp/Types.h"

namespace psychopomp {
namespace {
class NodesTrie {
 public:
  NodesTrie& addNode(const std::string& node, DomainId domainId,
                     Domain domain) {
    auto& child = children_[node];
    child.domain_ = domain;
    child.domainId_ = domainId;
  }

  folly::Optional<std::pair<Domain, DomainId>> getDomainPair(
      std::vector<std::string>& nodes) {
    auto* currentNode = this;
    for (auto& node : nodes) {
      currentNode = folly::get_ptr(currentNode->children_, node);
      if (!currentNode) {
        return folly::none;
      }
    }
    return std::make_pair(currentNode->domain_, currentNode->domainId_);
  }

 private:
  std::unordered_map<std::string, NodesTrie> children_;
  Domain domain_;
  DomainId domainId_;
};
}  // namespace

class DomainMapper {
 public:
  DomainMapper(
      const camfer::NodesConfig& nodesConfig,
      const std::unordered_map<BinId, std::vector<ShardInfo>>& binMapping,
      std::shared_ptr<std::vector<std::pair<ShardKey, ShardKey>>>
          shardKeyRangeMapping) {
    // Create bin mapping
    size_t binCount = 0;
    for (auto& [binId, shardInfos] : binMapping) {
      binCount++;
      binDomainIdMapping_[binCount] = binId;
      domainIdBinMapping_[binId] = binCount;
    }

    // Create nodes mapping
    std::unordered_map<size_t, size_t> levelToIndexMap;
    addLevelsMapping(nodesConfig, levelToIndexMap, 0, nodesTrie_);
  }

  folly::Optional<DomainId> getDomainId(BinId binName) {
    return folly::get_optional(domainIdBinMapping_, binName);
  }

  folly::Optional<BinId> getBinId(DomainId domainId) {
    return folly::get_optional(binDomainIdMapping_, domainId);
  }

  folly::Optional<std::pair<Domain, DomainId>> getDomainPair(
      std::vector<std::string> groups) {
    return nodesTrie_.getDomainPair(groups);
  }

 private:
  void addLevelsMapping(const camfer::NodesConfig& groupsConfig,
                        std::unordered_map<size_t, size_t>& levelToIndexMap,
                        size_t levelIndex, NodesTrie& nodesTrie) {
    auto& nodeChildren = groupsConfig.node_children();
    auto& levels = groupsConfig.node_levels();
    auto addLevels = [&](size_t levelIndex, NodesTrie& nodesTrie) {
      auto& level = levels[levelIndex];
      for (auto& node : nodeChildren.at(level).nodes()) {
        auto& nextNode = nodesTrie.addNode(node, levelIndex + 1,
                                           ++levelToIndexMap[levelIndex]);
        addLevelsMapping(groupsConfig, levelToIndexMap, levelIndex + 1,
                         nextNode);
      }
    };
  }

  std::unordered_map<DomainId, BinId> binDomainIdMapping_;
  std::unordered_map<BinId, DomainId> domainIdBinMapping_;
  NodesTrie nodesTrie_;
};
}  // namespace psychopomp
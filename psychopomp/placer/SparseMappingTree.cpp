#include "psychopomp/placer/SparseMappingTree.h"

#include "folly/MapUtil.h"

namespace psychopomp {

SparseMappingTree::SparseMappingTree(Domain shardDomain, Domain binDomain)
    : shardDomain_(shardDomain), binDomain_(binDomain) {}

void SparseMappingTree::addMapping(std::pair<Domain, DomainId> parent,
                                   Domain childDomain,
                                   std::vector<DomainId> childrenDomainIds) {
  const auto parentDomain = parent.first;
  const auto parentDomainId = parent.second;

  auto currentChildIt = parentToChildDomainMap_.find(parentDomain);
  if (currentChildIt != parentToChildDomainMap_.end() &&
      currentChildIt->second != childDomain) {
    // log error
    return;
  }

  parentToChildDomainMap_[parentDomain] = childDomain;

  for (auto child : childrenDomainIds) {
    childToParentMap_[childDomain][child].emplace(parentDomain, parentDomainId);
  }

  parentToChildMap_[parentDomain][parentDomainId] =
      std::move(childrenDomainIds);
}

std::vector<std::pair<Domain, DomainId>> SparseMappingTree::getParents(
    Domain domain, DomainId domainId,
    std::vector<std::shared_ptr<MovementMap>> movementMaps) const {
  std::vector<std::pair<Domain, DomainId>> domainParents;
  if (childToParentMap_.count(domain) != 0 &&
      childToParentMap_.at(domain).count(domainId) != 0) {
    const auto& domainParentsMap = childToParentMap_.at(domain).at(domainId);

    domainParents.reserve(domainParentsMap.size());

    for (auto [domain, domainId] : domainParentsMap) {
      domainParents.emplace_back(domain, domainId);
    }
  }

  if (domain == shardDomain_) {
    for (auto movementMap : movementMaps) {
      const auto parentBin = movementMap->getNextBin(domainId);

      if (parentBin) {
        // Check if new parent exists in tree
        const auto& binDomainMap = parentToChildMap_.at(binDomain_);
        auto parentIt = binDomainMap.find(parentBin.value());
        if (parentIt != binDomainMap.end()) {
          domainParents.emplace_back(binDomain_, parentBin.value());
        }
      }
    }
  }

  return domainParents;
}

std::pair<Domain, std::vector<DomainId>> SparseMappingTree::getChildren(
    Domain domain, DomainId domainId,
    std::vector<std::shared_ptr<MovementMap>> movementMaps) const {
  std::vector<DomainId> children;
  if (parentToChildDomainMap_.find(domain) == parentToChildDomainMap_.end()) {
    return {};
  }
  if (domain == binDomain_) {
    for (auto movementMap : movementMaps) {
      const auto& incomingShards = movementMap->getIncomingShards(domainId);
      children.insert(children.end(), incomingShards.begin(),
                      incomingShards.end());
    }
  }
  auto childDomain = getChildDomain(domain);
  if (!childDomain) {
    return {};
  }
  auto& mappedChildren = parentToChildMap_.at(domain).at(domainId);
  children.insert(children.end(), mappedChildren.begin(), mappedChildren.end());
  return {*childDomain, children};
}

const std::vector<DomainId>& SparseMappingTree::getChildren(
    Domain domain, DomainId domainId) const {
  return folly::get_ref_default(parentToChildMap_, domain, domainId,
                                noChildrenVector_);
}

folly::Optional<Domain> SparseMappingTree::getChildDomain(Domain domain) const {
  return folly::get_optional(parentToChildDomainMap_, domain);
}

std::vector<DomainId> SparseMappingTree::getAllDomainIds(Domain domain) {
  std::vector<DomainId> domainIds;
  if (parentToChildDomainMap_.count(domain) != 0) {
    domainIds.reserve(parentToChildMap_[domain].size());
    for (const auto& [domainId, child] : parentToChildMap_[domain]) {
      domainIds.emplace_back(domainId);
    }

    return domainIds;
  }

  // Check childToParentMapchildToParentMap_
  domainIds.reserve(childToParentMap_[domain].size());
  for (const auto& [domainId, parents] : childToParentMap_[domain]) {
    domainIds.emplace_back(domainId);
  }

  return domainIds;
}

bool SparseMappingTree::doesNodeExist(Domain domain, DomainId domainId) {
  const auto& childMap = childToParentMap_[domain];
  const auto& parentMap = parentToChildMap_[domain];
  auto childIt = childMap.find(domainId);
  auto parentIt = parentMap.find(domainId);
  return childIt != childMap.end() || parentIt != parentMap.end();
}

std::shared_ptr<SparseMappingTree> SparseMappingTree::createPartialTree(
    Domain domain, const std::vector<DomainId>& treeParents) const {
  auto partialTree =
      std::make_shared<SparseMappingTree>(shardDomain_, binDomain_);
  std::deque<std::pair<Domain, DomainId>> nodesToAdd;
  for (auto parentId : treeParents) {
    nodesToAdd.emplace_back(domain, parentId);
  }

  auto addChild = [&](Domain childDomain,
                      const std::vector<DomainId>& childDomainIds) {
    for (auto domainId : childDomainIds) {
      nodesToAdd.emplace_back(childDomain, domainId);
    }
  };

  while (!nodesToAdd.empty()) {
    auto& parent = nodesToAdd.front();
    const auto& children = getChildren(parent.first, parent.second);
    auto childDomain = getChildDomain(parent.first);
    if (!childDomain) {
      nodesToAdd.pop_front();
      continue;
    }
    partialTree->addMapping(parent, *childDomain, children);
    addChild(*childDomain, children);
    nodesToAdd.pop_front();
  }

  return partialTree;
}
}  // namespace psychopomp
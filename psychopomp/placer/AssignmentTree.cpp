#include "psychopomp/placer/AssignmentTree.h"

namespace psychopomp {

AssignmentTree::AssignmentTree(Domain shardDomain, Domain binDomain)
    : shardDomain_(shardDomain), binDomain_(binDomain) {}

void AssignmentTree::addMapping(
    std::pair<Domain, DomainId> parent,
    std::pair<Domain, std::vector<DomainId>> children) {
  const auto parentDomain = parent.first;
  const auto parentDomainId = parent.second;
  const auto childDomain = children.first;
  const auto& childrenDomainIds = children.second;

  auto currentChildIt = parentToChildDomainMap_.find(parentDomain);
  if (currentChildIt != parentToChildDomainMap_.end() &&
      currentChildIt->second != childDomain) {
    // log error
    return;
  }

  parentToChildDomainMap_[parentDomain] = childDomain;

  for (auto child : childrenDomainIds) {
    childToParentMap_[childDomain][child][parentDomain].insert(parentDomainId);
  }

  parentToChildMap_[parentDomain][parentDomainId] =
      std::move(childrenDomainIds);
}

std::vector<std::pair<Domain, DomainId>> AssignmentTree::getParents(
    Domain domain, DomainId domainId,
    std::vector<std::shared_ptr<MovementMap>> movementMaps) const {
  std::vector<std::pair<Domain, DomainId>> domainParents;
  if (childToParentMap_.count(domain) != 0 &&
      childToParentMap_.at(domain).count(domainId) != 0) {
    const auto& domainParentsMap = childToParentMap_.at(domain).at(domainId);

    domainParents.reserve(domainParentsMap.size());

    for (auto [domain, parents] : domainParentsMap) {
      for (auto domainId : parents) {
        domainParents.emplace_back(domain, domainId);
      }
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

std::pair<Domain, std::vector<DomainId>> AssignmentTree::getChildren(
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
  const auto& childDomain = parentToChildDomainMap_.at(domain);
  auto& mappedChildren = parentToChildMap_.at(domain).at(domainId);
  children.insert(children.end(), mappedChildren.begin(), mappedChildren.end());
  return {childDomain, children};
}

std::vector<DomainId>& AssignmentTree::getChildren(Domain domain,
                                                   DomainId domainId) {
  return parentToChildMap_[domain][domainId];
}

Domain AssignmentTree::getChildDomain(Domain domain) const {
  return parentToChildDomainMap_.at(domain);
}

std::vector<DomainId> AssignmentTree::getAllDomainIds(Domain domain) {
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

bool AssignmentTree::doesNodeExist(Domain domain, DomainId domainId) {
  const auto& childMap = childToParentMap_[domain];
  const auto& parentMap = parentToChildMap_[domain];
  auto childIt = childMap.find(domainId);
  auto parentIt = parentMap.find(domainId);
  return childIt != childMap.end() || parentIt != parentMap.end();
}

}  // namespace psychopomp
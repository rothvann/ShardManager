#pragma once

#include <iostream>
#include <unordered_set>

#include "psychopomp/Types.h"
#include "psychopomp/placer/MovementMap.h"

namespace psychopomp {
class SparseMappingTree {
 public:
  SparseMappingTree(Domain shardDomain, Domain binDomain);

  void addMapping(std::pair<Domain, DomainId> parent, Domain childDomain,
                   std::vector<DomainId> childrenDomainIds);

  std::vector<std::pair<Domain, DomainId>> getParents(
      Domain domain, DomainId domainId,
      std::vector<std::shared_ptr<MovementMap>> movementMaps) const;

  std::pair<Domain, std::vector<DomainId>> getChildren(
      Domain domain, DomainId domainId,
      std::vector<std::shared_ptr<MovementMap>> movementMaps) const;

  const std::vector<DomainId>& getChildren(Domain domain,
                                           DomainId domainId) const;

  folly::Optional<Domain> getChildDomain(Domain domain) const;
  std::vector<DomainId> getAllDomainIds(Domain domain);

  bool doesNodeExist(Domain domain, DomainId domainId);

  std::shared_ptr<SparseMappingTree> createPartialTree(
      Domain domain, const std::vector<DomainId>& treeParents) const;

 private:
  Domain shardDomain_;
  Domain binDomain_;

  const std::vector<DomainId> noChildrenVector_;

  // Shard Assignment
  std::unordered_map<
      Domain,
      std::unordered_map<DomainId, std::unordered_map<Domain, DomainId>>>
      childToParentMap_;

  std::unordered_map<Domain, Domain> parentToChildDomainMap_;
  std::unordered_map<Domain,
                     std::unordered_map<DomainId, std::vector<DomainId>>>
      parentToChildMap_;
};
}  // namespace psychopomp
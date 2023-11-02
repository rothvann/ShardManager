#pragma once

#include <iostream>
#include <unordered_set>

#include "psychopomp/Types.h"
#include "psychopomp/placer/MovementMap.h"

namespace psychopomp {
class AssignmentTree {
 public:
  AssignmentTree(Domain shardDomain, Domain binDomain);

  void addMapping(std::pair<Domain, DomainId> parent,
                  std::pair<Domain, std::vector<DomainId>> children);

  std::vector<std::pair<Domain, DomainId>> getParents(
      Domain domain, DomainId domainId,
      std::vector<std::shared_ptr<MovementMap>> movementMaps) const;

  std::pair<Domain, std::vector<DomainId>> getChildren(
      Domain domain, DomainId domainId,
      std::vector<std::shared_ptr<MovementMap>> movementMaps) const;

  std::vector<DomainId>& getChildren(Domain domain, DomainId domainId);

  Domain getChildDomain(Domain domain) const;
  std::vector<DomainId> getAllDomainIds(Domain domain);

  bool doesNodeExist(Domain domain, DomainId domainId);

 private:
  Domain shardDomain_;
  Domain binDomain_;

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
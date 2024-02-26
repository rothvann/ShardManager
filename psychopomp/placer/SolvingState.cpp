#include "psychopomp/placer/SolvingState.h"

#include <iostream>

#include "folly/MapUtil.h"

namespace psychopomp {

SolvingState::SolvingState(
    std::shared_ptr<std::vector<MappedShardInfo>> shardInfoVector,
    std::shared_ptr<std::vector<std::vector<MetricValue>>> metricVectors,
    std::shared_ptr<std::vector<std::vector<std::vector<DomainId>>>>
        domainMapping)
    : shardDomain_(0),
      binDomain_(1),
      domainOffset_(2),
      shardInfoVector_(shardInfoVector),
      metricVectors_(metricVectors) {
  domainElements_.emplace(shardDomain_, shardInfoVector_->size());

  assignmentTree_ =
      std::make_shared<SparseMappingTree>(shardDomain_, binDomain_);

  auto childDomain = shardDomain_;
  for (size_t domain = 0; domain < domainMapping->size(); domain++) {
    auto parentDomain = domain + 1;
    addDomainInternal(parentDomain, domain, (*domainMapping)[domain]);
  }
}

void SolvingState::addDomainInternal(
    const Domain& parentDomain, const Domain& childDomain,
    const std::vector<std::vector<DomainId>>& parentChildMap) {
  domainElements_.emplace(parentDomain, parentChildMap.size());

  for (size_t parentId = 0; parentId < parentChildMap.size(); parentId++) {
    const auto& childIdVector = parentChildMap[parentId];
    assignmentTree_->addMapping({parentDomain, parentId}, childDomain,
                                childIdVector);
  }
}

std::shared_ptr<SparseMappingTree> SolvingState::getAssignmentTree() const {
  return assignmentTree_;
}

std::shared_ptr<MovementMap> SolvingState::getMovementMap() const {
  return movementMap_;
}

Domain SolvingState::getShardDomain() const { return shardDomain_; }

Domain SolvingState::getBinDomain() const { return binDomain_; }

const MappedShardInfo& SolvingState::getShardInfo(DomainId shardId) const {
  return (*shardInfoVector_)[shardId];
}

size_t SolvingState::getDomainSize(Domain domain) const {
  return folly::get_default(domainElements_, domain, 0);
}

std::vector<MetricValue> SolvingState::getMetric(Metric metric) const {
  return (*metricVectors_)[metric];
}

int32_t SolvingState::getShardMetric(Metric metric, DomainId domainId) const {
  return (*metricVectors_)[metric][domainId];
}

BinWeightInfo& SolvingState::getBinWeightInfo() { return binWeightInfo_; }
}  // namespace psychopomp
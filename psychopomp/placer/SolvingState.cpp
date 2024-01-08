#include "psychopomp/placer/SolvingState.h"

#include <iostream>

#include "folly/MapUtil.h"

namespace psychopomp {

SolvingState::SolvingState(const std::vector<ShardInfo>& shardInfoVector,
             const std::vector<size_t> domainSizes,
             const std::vector<std::vector<DomainId>>& binDomainsMapping,
             const std::vector<std::vector<DomainId>>& binShardMapping)
    : domainCounter_(0),
      movementMap_(std::make_shared<MovementMap>()),
      shardInfoVector_(shardInfoVector) {
  setShards(shardInfoVector_.size());
  binDomain_ = getNewDomain();
  assignmentTree_ = std::make_shared<AssignmentTree>(shardDomain_, binDomain_);
  addDomainInternal(binDomain_, shardDomain_, binShardMapping);

  auto childDomain = binDomain_;
  auto numBins = binShardMapping.size();
  for (size_t domain = 0; domain < domainSizes.size(); domain++) {
    auto parentDomain = getNewDomain();
    std::vector<std::vector<DomainId>> parentChildMapping(domainSizes[domain]);
    for (DomainId binId = 0; binId < numBins; binId++) {
      auto parentId = binDomainsMapping[binId][domain];
      parentChildMapping[parentId].emplace_back(binId);
      binDomainsMapping_[binId][parentDomain] = parentId;
    }

    addDomainInternal(parentDomain, childDomain, parentChildMapping);
    childDomain = parentDomain;
  }
}

void SolvingState::setShards(const size_t numShards) {
  shardDomain_ = getNewDomain();
  domainElements_.emplace(shardDomain_, numShards);
}

Domain SolvingState::addDomain(
    const Domain& childDomain,
    const std::vector<std::vector<DomainId>>& parentChildMap) {
  Domain parentDomain = getNewDomain();
  addDomainInternal(parentDomain, childDomain, parentChildMap);
  return parentDomain;
}

void SolvingState::addDomainInternal(
    const Domain& parentDomain, const Domain& childDomain,
    const std::vector<std::vector<DomainId>>& parentChildMap) {
  domainElements_.emplace(parentDomain, parentChildMap.size());

  for (size_t parentId = 0; parentId < parentChildMap.size(); parentId++) {
    const auto& childIdVector = parentChildMap[parentId];
    assignmentTree_->addMapping({parentDomain, parentId},
                                {childDomain, childIdVector});
  }
}

std::shared_ptr<AssignmentTree> SolvingState::getAssignmentTree() const {
  return assignmentTree_;
}

std::shared_ptr<MovementMap> SolvingState::getMovementMap() const {
  return movementMap_;
}

void SolvingState::addMetric(Metric metric, const std::vector<int32_t>& metricVector) {
  metricVectorMap_.emplace(metric, metricVector);
}

Domain SolvingState::getShardDomain() const { return shardDomain_; }

Domain SolvingState::getBinDomain() const { return binDomain_; }

const ShardInfo& SolvingState::getShardInfo(DomainId shardId) const {
  return shardInfoVector_[shardId];
}

std::optional<DomainId> SolvingState::getBinParentInDomain(
    DomainId binId, Domain parentDomain) const {
  auto* ptr = folly::get_ptr(binDomainsMapping_, binId, parentDomain);
  if (ptr) {
    return *ptr;
  }
  return std::nullopt;
}

size_t SolvingState::getDomainSize(Domain domain) const {
  return folly::get_default(domainElements_, domain, 0);
}

std::vector<Metric> SolvingState::getMetrics() const {
  std::vector<Metric> metrics;
  metrics.reserve(metricVectorMap_.size());
  for (const auto& [metric, values] : metricVectorMap_) {
    metrics.emplace_back(metric);
  }
  return metrics;
}

std::vector<int32_t> SolvingState::getMetric(Metric metric) const {
  return metricVectorMap_.at(metric);
}

int32_t SolvingState::getShardMetric(Metric metric, DomainId domainId) const {
  return metricVectorMap_.at(metric)[domainId];
}

BinWeightInfo& SolvingState::getBinWeightInfo() { return binWeightInfo_; }

Domain SolvingState::getNewDomain() { return domainCounter_++; }
}  // namespace psychopomp
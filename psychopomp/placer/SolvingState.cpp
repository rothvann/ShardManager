#include "psychopomp/placer/SolvingState.h"

#include <iostream>

#include "folly/MapUtil.h"

namespace psychopomp {

SolvingState::SolvingState(std::shared_ptr<std::vector<MappedShardInfo>> shardInfoVector,
             const std::vector<size_t> domainSizes,
             std::shared_ptr<std::vector<std::vector<DomainId>>> binDomainsMapping,
             std::shared_ptr<std::vector<std::vector<DomainId>>> binShardMapping)
    : shardDomain_(0),
      binDomain_(1), 
      domainOffset_(2),
      movementMap_(std::make_shared<MovementMap>()),
      shardInfoVector_(shardInfoVector) {
  domainElements_.emplace(shardDomain_, shardInfoVector_->size());

  assignmentTree_ = std::make_shared<SparseMappingTree>(shardDomain_, binDomain_);
  addDomainInternal(binDomain_, shardDomain_, *binShardMapping);

  auto childDomain = binDomain_;
  auto numBins = binShardMapping->size();
  for (size_t domain = 0; domain < domainSizes.size(); domain++) {
    std::vector<std::vector<DomainId>> parentChildMapping(domainSizes[domain]);
    for (DomainId binId = 0; binId < numBins; binId++) {
      auto parentId = (*binDomainsMapping)[binId][domain];
      parentChildMapping[parentId].emplace_back(binId);
      binDomainsMapping_[binId][domain + domainOffset_] = parentId;
    }

    addDomainInternal(domain + domainOffset_, childDomain, parentChildMapping);
    childDomain = domain + domainOffset_;
  }
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

std::shared_ptr<SparseMappingTree> SolvingState::getAssignmentTree() const {
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

const MappedShardInfo& SolvingState::getShardInfo(DomainId shardId) const {
  return (*shardInfoVector_)[shardId];
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
}  // namespace psychopomp
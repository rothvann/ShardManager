#include "psychopomp/State.h"

#include <iostream>

namespace psychopomp {

State::State(const Domain& shardDomain, const Domain& binDomain)
    : shardDomain_(shardDomain),
      binDomain_(binDomain),
      assignmentTree_(std::make_shared<AssignmentTree>(shardDomain, binDomain)),
      movementMap_(std::make_shared<MovementMap>()) {}

void State::setShards(const std::vector<DomainId>& shards) {
  domainElements_.emplace(shardDomain_, shards);
}

void State::addDomain(
    const Domain& parentDomain, const Domain& childDomain,
    const std::unordered_map<DomainId, std::vector<DomainId>>& parentChildMap) {
  auto& parentDomainElements = domainElements_[parentDomain];
  parentDomainElements.reserve(parentChildMap.size());

  auto& parentDomainIds = domainIdMap_[parentDomain];

  for (const auto& [parentId, childIdVector] : parentChildMap) {
    parentDomainElements.emplace_back(parentId);
    parentDomainIds[parentId] = parentId;

    assignmentTree_->addMapping({parentDomain, parentId},
                                {childDomain, childIdVector});
  }
}

std::shared_ptr<AssignmentTree> State::getAssignmentTree() const {
  return assignmentTree_;
}

std::shared_ptr<MovementMap> State::getMovementMap() const {
  return movementMap_;
}

void State::addMetric(Metric metric, const std::vector<int32_t>& metricVector) {
  metricsMap_.emplace(metric, metricVector);
}

Domain State::getShardDomain() const { return shardDomain_; }

Domain State::getBinDomain() const { return binDomain_; }

size_t State::getDomainSize(Domain domain) const {
  auto it = domainElements_.find(domain);
  if (it != domainElements_.end()) {
    return it->second.size();
  }
  return 0;
}

std::vector<Metric> State::getMetrics() const {
  std::vector<Metric> metrics;
  metrics.reserve(metricsMap_.size());
  for (const auto& [metric, values] : metricsMap_) {
    metrics.emplace_back(metric);
  }
  return metrics;
}

std::vector<int32_t> State::getMetric(Metric metric) const {
  return metricsMap_.at(metric);
}

int32_t State::getShardMetric(Metric metric, DomainId domainId) const {
  return metricsMap_.at(metric)[domainId];
}

}  // namespace psychopomp
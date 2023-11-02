#include "psychopomp/placer/State.h"

#include <iostream>

#include "folly/MapUtil.h"

namespace psychopomp {

State::State(const size_t numShards,
             const std::vector<std::vector<DomainId>>& parentChildMap)
    : domainCounter_(0), movementMap_(std::make_shared<MovementMap>()) {
  setShards(numShards);
  binDomain_ = getNewDomain();
  assignmentTree_ = std::make_shared<AssignmentTree>(shardDomain_, binDomain_);
  addDomainInternal(binDomain_, shardDomain_, parentChildMap);
}

void State::setShards(const size_t numShards) {
  shardDomain_ = getNewDomain();
  domainElements_.emplace(shardDomain_, numShards);
}

Domain State::addDomain(
    const Domain& childDomain,
    const std::vector<std::vector<DomainId>>& parentChildMap) {
  Domain parentDomain = getNewDomain();
  addDomainInternal(parentDomain, childDomain, parentChildMap);
  return parentDomain;
}

void State::addDomainInternal(
    const Domain& parentDomain, const Domain& childDomain,
    const std::vector<std::vector<DomainId>>& parentChildMap) {
  domainElements_.emplace(parentDomain, parentChildMap.size());

  for (size_t parentId = 0; parentId < parentChildMap.size(); parentId++) {
    const auto& childIdVector = parentChildMap[parentId];
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
  metricVectorMap_.emplace(metric, metricVector);
}

Domain State::getShardDomain() const { return shardDomain_; }

Domain State::getBinDomain() const { return binDomain_; }

size_t State::getDomainSize(Domain domain) const {
  return folly::get_default(domainElements_, domain, 0);
}

std::vector<Metric> State::getMetrics() const {
  std::vector<Metric> metrics;
  metrics.reserve(metricVectorMap_.size());
  for (const auto& [metric, values] : metricVectorMap_) {
    metrics.emplace_back(metric);
  }
  return metrics;
}

std::vector<int32_t> State::getMetric(Metric metric) const {
  return metricVectorMap_.at(metric);
}

int32_t State::getShardMetric(Metric metric, DomainId domainId) const {
  return metricVectorMap_.at(metric)[domainId];
}

BinWeightInfo& State::getBinWeightInfo() { return binWeightInfo_; }

Domain State::getNewDomain() { return domainCounter_++; }
}  // namespace psychopomp
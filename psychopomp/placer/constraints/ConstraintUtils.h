#pragma once

#include "psychopomp/placer/ExpressionTree.h"
#include "psychopomp/placer/State.h"

namespace psychopomp {

enum class MovementConsistency { BEFORE = 1, AFTER = 2, BOTH = 3, DURING = 4 };

bool shouldConsiderShard(const MovementConsistency& consistency,
                         bool isFutureChild, bool isMoving) {
  switch (consistency) {
    case MovementConsistency::BEFORE:
      return !isFutureChild;
    case MovementConsistency::AFTER:
      return isFutureChild;
    case MovementConsistency::BOTH:
      return true;
    case MovementConsistency::DURING:
      return isMoving;
  }
  return false;
}

bool shouldConsiderShard(
    const MovementConsistency& consistency,
    const std::vector<std::shared_ptr<MovementMap>>& movementMaps,
    DomainId childId, DomainId binId) {
  bool isFutureChild = true;
  bool isMoving = false;
  for (auto movementMap : movementMaps) {
    auto nextBin = movementMap->getNextBin(childId);
    if (nextBin.has_value()) {
      isMoving = true;
      if (nextBin.value() != binId) {
        isFutureChild = false;
        break;
      }
    }
  }
  return shouldConsiderShard(consistency, isFutureChild, isMoving);
}

DomainId getCurrentBin(const AssignmentTree& assignmentTree, Domain shardDomain,
                       DomainId shardId, Domain binDomain) {
  auto parents = assignmentTree.getParents(shardDomain, shardId, {});
  for (auto& parent : parents) {
    if (parent.first == binDomain) {
      return parent.second;
    }
  }
  return kDefaultBin;
}

DomainId getFutureBin(
    const AssignmentTree& assignmentTree,
    const std::vector<std::shared_ptr<psychopomp::MovementMap>>& movementMaps,
    Domain shardDomain, DomainId shardId, Domain binDomain) {
  for (auto movementMap : movementMaps) {
    auto nextBin = movementMap->getNextBin(shardId);
    if (nextBin.has_value()) {
      return nextBin.value();
    }
  }
  auto parents = assignmentTree.getParents(shardDomain, shardId, {});
  for (auto& parent : parents) {
    if (parent.first == binDomain) {
      return parent.second;
    }
  }
  return kDefaultBin;
}

int32_t sumOperator(
    std::shared_ptr<State> state, Metric metric,
    const AssignmentTree& assignmentTree,
    const std::vector<std::shared_ptr<MovementMap>>& movementMaps,
    const MetricsMap& metricMap, const std::vector<DomainId>& changedChildren,
    std::pair<Domain, DomainId> node, MovementConsistency consistency) {
  auto& domain = node.first;
  auto& domainId = node.second;
  auto childDomain = assignmentTree.getChildDomain(domain);
  int64_t val = metricMap.get(domain, domainId).value_or(0);

  for (auto child : changedChildren) {
    bool isFutureChild = true;
    if (domain == state->getBinDomain()) {
      if (!shouldConsiderShard(consistency, movementMaps, child, domainId)) {
        isFutureChild = false;
      }
    }

    int32_t metric = 0;
    if (childDomain == state->getShardDomain()) {
      metric = state->getShardMetric(metric, child);
    } else {
      metric = metricMap.get(childDomain, child).value_or(0) -
               metricMap.getFromCommittedMap(childDomain, child).value_or(0);
    }
    if (isFutureChild) {
      val += metric;
    } else {
      val -= metric;
    }
  }
  return val;
};


int32_t shardCountOperator(
    std::shared_ptr<State> state,
    const AssignmentTree& assignmentTree,
    const std::vector<std::shared_ptr<MovementMap>>& movementMaps,
    const MetricsMap& metricMap, const std::vector<DomainId>& changedChildren,
    std::pair<Domain, DomainId> node, MovementConsistency consistency) {
  auto& domain = node.first;
  auto& domainId = node.second;
  auto childDomain = assignmentTree.getChildDomain(domain);
  int64_t val = metricMap.get(domain, domainId).value_or(0);

  for (auto child : changedChildren) {
    bool isFutureChild = true;
    if (domain == state->getBinDomain()) {
      if (!shouldConsiderShard(consistency, movementMaps, child, domainId)) {
        isFutureChild = false;
      }
    }

    if (isFutureChild) {
      val += 1;
    } else {
      val -= 1;
    }
  }
  return val;
};

}  // namespace psychopomp
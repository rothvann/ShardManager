#pragma once

#include <experimental/optional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "psychopomp/AssignmentTree.h"
#include "psychopomp/MovementMap.h"
#include "psychopomp/Types.h"
#include "psychopomp/utils/Committable.h"

namespace psychopomp {

struct BinWeightInfo {
  CommittableMap<std::unordered_map<DomainId, int64_t>, int64_t> binWeightMap;
  CommittableKey<int64_t> totalWeight;
};

class State {
 public:
  State(const Domain& shardDomain, const Domain& binDomain);

  void setShards(const std::vector<DomainId>& shards);

  void addDomain(const Domain& parentDomain, const Domain& childDomain,
                 const std::unordered_map<DomainId, std::vector<DomainId>>&
                     parentChildMap);

  std::shared_ptr<AssignmentTree> getAssignmentTree() const;

  std::shared_ptr<MovementMap> getMovementMap() const;

  void addMetric(Metric metric, const std::vector<int32_t>& metricVector);

  Domain getShardDomain() const;
  Domain getBinDomain() const;

  size_t getDomainSize(Domain domain) const;

  std::vector<Metric> getMetrics() const;
  std::vector<int32_t> getMetric(Metric metric) const;
  int32_t getShardMetric(Metric metric, DomainId domainId) const;

  BinWeightInfo& getBinWeightInfo();

 private:
  Domain shardDomain_;
  Domain binDomain_;

  std::unordered_map<Metric, std::vector<int32_t>> metricVectorMap_;
  BinWeightInfo binWeightInfo_;

  // Shard Assignment
  std::unordered_map<Domain, std::vector<DomainId>> domainElements_;
  std::unordered_map<Domain, std::unordered_map<DomainId, DomainId>>
      domainIdMap_;

  std::shared_ptr<AssignmentTree> assignmentTree_;

  // Movements
  std::shared_ptr<MovementMap> movementMap_;
};
}  // namespace psychopomp
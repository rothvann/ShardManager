#pragma once

#include <experimental/optional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "ankerl/unordered_dense.h"
#include "psychopomp/Types.h"
#include "psychopomp/placer/MovementMap.h"
#include "psychopomp/placer/SparseMappingTree.h"
#include "psychopomp/placer/utils/Committable.h"

namespace psychopomp {

struct BinWeightInfo {
  CommittableMap<std::unordered_map<DomainId, int64_t>, int64_t> binWeightMap;
  CommittableKey<int64_t> totalWeight;
};

class SolvingState {
 public:
  SolvingState(
      std::shared_ptr<std::vector<MappedShardInfo>> shardInfoVector,
      std::shared_ptr<std::vector<std::vector<MetricValue>>> metricVectors,
      std::shared_ptr<std::vector<std::vector<std::vector<DomainId>>>>
          domainMapping);

  std::shared_ptr<SparseMappingTree> getAssignmentTree() const;

  std::shared_ptr<MovementMap> getMovementMap() const;

  Domain getShardDomain() const;
  Domain getBinDomain() const;

  const MappedShardInfo& getShardInfo(DomainId shardId) const;

  size_t getDomainSize(Domain domain) const;

  std::vector<MetricValue> getMetric(Metric metric) const;
  int32_t getShardMetric(Metric metric, DomainId domainId) const;

  BinWeightInfo& getBinWeightInfo();

  static const Domain kDomainLimit = 10;

 private:
  void addDomainInternal(
      const Domain& parentDomain, const Domain& childDomain,
      const std::vector<std::vector<DomainId>>& parentChildMap);

  const Domain shardDomain_;
  const Domain binDomain_;
  const size_t domainOffset_;

  BinWeightInfo binWeightInfo_;

  // Shard Assignment
  std::unordered_map<Domain, size_t> domainElements_;
  std::unordered_map<Domain, std::unordered_map<DomainId, DomainId>>
      domainIdMap_;

  std::shared_ptr<std::vector<MappedShardInfo>> shardInfoVector_;
  std::shared_ptr<std::vector<std::vector<MetricValue>>> metricVectors_;

  std::shared_ptr<SparseMappingTree> assignmentTree_;

  // Movements
  std::shared_ptr<MovementMap> movementMap_;
};
}  // namespace psychopomp
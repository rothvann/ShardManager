#pragma once

#include <experimental/optional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "ankerl/unordered_dense.h"
#include "psychopomp/Types.h"
#include "psychopomp/placer/AssignmentTree.h"
#include "psychopomp/placer/MovementMap.h"
#include "psychopomp/placer/utils/Committable.h"

namespace psychopomp {

struct BinWeightInfo {
  CommittableMap<std::unordered_map<DomainId, int64_t>, int64_t> binWeightMap;
  CommittableKey<int64_t> totalWeight;
};

class SolvingState {
 public:
  SolvingState(const std::vector<ShardInfo>& shardInfoVector,
        const std::vector<size_t> domainSizes_,
        const std::vector<std::vector<DomainId>>& binDomainsMapping,
        const std::vector<std::vector<DomainId>>& binShardMapping);

  Domain addDomain(const Domain& childDomain,
                   const std::vector<std::vector<DomainId>>& parentChildMap);

  std::shared_ptr<AssignmentTree> getAssignmentTree() const;

  std::shared_ptr<MovementMap> getMovementMap() const;

  void addMetric(Metric metric, const std::vector<int32_t>& metricVector);

  Domain getShardDomain() const;
  Domain getBinDomain() const;

  const ShardInfo& getShardInfo(DomainId shardId) const;

  std::optional<DomainId> getBinParentInDomain(DomainId binId,
                                                 Domain parentDomain) const;

  size_t getDomainSize(Domain domain) const;

  std::vector<Metric> getMetrics() const;
  std::vector<int32_t> getMetric(Metric metric) const;
  int32_t getShardMetric(Metric metric, DomainId domainId) const;

  BinWeightInfo& getBinWeightInfo();

 private:
  void addDomainInternal(
      const Domain& parentDomain, const Domain& childDomain,
      const std::vector<std::vector<DomainId>>& parentChildMap);
  Domain getNewDomain();

  void setShards(const size_t numShards);
  Domain shardDomain_;
  Domain binDomain_;
  size_t domainCounter_;

  std::unordered_map<Metric, std::vector<int32_t>> metricVectorMap_;
  BinWeightInfo binWeightInfo_;

  // Shard Assignment
  std::unordered_map<Domain, size_t> domainElements_;
  std::unordered_map<Domain, std::unordered_map<DomainId, DomainId>>
      domainIdMap_;

  std::vector<ShardInfo> shardInfoVector_;
  std::unordered_map<DomainId, std::unordered_map<Domain, DomainId>>
      binDomainsMapping_;

  std::shared_ptr<AssignmentTree> assignmentTree_;

  // Movements
  std::shared_ptr<MovementMap> movementMap_;
};
}  // namespace psychopomp
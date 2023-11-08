#pragma once

#include "psychopomp/placer/constraints/Constraint.h"

// Create count map of each shard
namespace psychopomp {

class ExclusionConstraint : public Constraint {
 public:
  ExclusionConstraint(std::shared_ptr<State> state,
                      MovementConsistency consistency, Domain exclusionDomain,
                      const std::vector<DomainId>& shardIds,
                      int32_t faultWeight)
      : exclusionDomain_(exclusionDomain), faultWeight_(faultWeight) {}

 private:
  Domain exclusionDomain_;
  int32_t faultWeight_;
};
}  // namespace psychopomp

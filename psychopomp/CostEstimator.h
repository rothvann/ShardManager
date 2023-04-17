#include "States.h"

namespace psychopomp {

class CostEstimator {
 public:
  virtual uint64_t get(const std::vector<uint32_t>& shardPlacements,
                       const ComputationState& compState) = 0;
};

}  // namespace psychopomp
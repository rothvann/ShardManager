#include "States.h"

namespace psychopomp {

class Searcher {
 public:
  virtual std::vector<uint32_t> get(const ComputationState& compState) = 0;
};

}  // namespace psychopomp
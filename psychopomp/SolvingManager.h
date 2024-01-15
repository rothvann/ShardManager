#include <folly/executors/CPUThreadPoolExecutor.h>

#include <chrono>

#include "psychopomp/ServiceMappingProvider.h"
#include "psychopomp/placer/IterativeLocalSearch.h"
#include "psychopomp/placer/SolvingState.h"

// Create state
// Remove finished mutations
// Solve
// Add mutations

namespace psychopomp {

class SolvingManager {
 public:
  SolvingManager(std::shared_ptr<ServiceMappingProvider> mappingProvider,
                 size_t periodMs, size_t numThreads);

 private:
  void updateAll();

  void update(
      ServiceName& svcName,
      std::unordered_map<BinName, std::vector<std::pair<ShardKey, ShardKey>>>& shardMappings);

  std::shared_ptr<ServiceMappingProvider> mappingProvider_;
  size_t periodMs_;
  size_t numThreads_;

  folly::CPUThreadPoolExecutor threadPool_;

  std::unordered_map<ServiceName, std::shared_ptr<MovementMap>> movementMaps_;
  std::unordered_map<ServiceName, std::shared_ptr<SolvingState>> solvingStates_;
  std::unordered_map<
      ServiceName, std::shared_ptr<std::vector<std::pair<ShardKey, ShardKey>>>>
      shardKeyRangeMappings_;
};
}  // namespace psychopomp
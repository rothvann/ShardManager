#include <folly/executors/CPUThreadPoolExecutor.h>

#include <chrono>

#include "psychopomp/ServiceMappingProvider.h"
#include "psychopomp/placer/IterativeLocalSearch.h"
#include "psychopomp/placer/SolvingState.h"
#include "CamferConfig.pb.h"

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

  // Stages
  void populateServiceConfig(std::unordered_set<ServiceName>& serviceNames);
  void populateShardKeyRangeMap(std::unordered_set<ServiceName>& serviceNames);
  void createSolvingState(std::unordered_set<ServiceName>& serviceNames);
  void solve(std::unordered_set<ServiceName>& serviceNames);
  void outputSolution(std::unordered_set<ServiceName>& serviceNames);

  void mapShardsToKeyRanges(
      ServiceName& svcName,
      std::unordered_map<BinName, std::vector<std::pair<ShardKey, ShardKey>>>&
          shardMappings);

  std::shared_ptr<ServiceMappingProvider> mappingProvider_;
  size_t periodMs_;
  size_t numThreads_;

  folly::CPUThreadPoolExecutor threadPool_;

  std::unordered_map<ServiceName, camfer::ServiceConfig> serviceConfig_;
  std::unordered_map<
      ServiceName, std::shared_ptr<std::vector<std::pair<ShardKey, ShardKey>>>>
      shardKeyRangeMappings_;
      
  std::unordered_map<
      ServiceName, std::unordered_map<BinName, std::vector<ShardInfo>>> binMappings_;
  std::unordered_map<ServiceName, std::shared_ptr<MovementMap>> movementMaps_;
  std::unordered_map<ServiceName, std::shared_ptr<SolvingState>> solvingStates_;

};
}  // namespace psychopomp
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
  void populateServiceConfig(std::unordered_set<ServiceId>& serviceIds);
  void populateShardKeyRangeMap(std::unordered_set<ServiceId>& serviceIds);
  void createSolvingState(std::unordered_set<ServiceId>& serviceIds);
  void solve(std::unordered_set<ServiceId>& serviceIds);
  void outputSolution(std::unordered_set<ServiceId>& serviceIds);

  void mapShardsToKeyRanges(
      ServiceId& svcId,
      std::unordered_map<BinId, std::vector<std::pair<ShardKey, ShardKey>>>&
          shardMappings);

  std::shared_ptr<ServiceMappingProvider> mappingProvider_;
  size_t periodMs_;
  size_t numThreads_;

  folly::CPUThreadPoolExecutor threadPool_;

  std::unordered_map<ServiceId, camfer::ServiceConfig> serviceConfig_;
  std::unordered_map<
      ServiceId, std::shared_ptr<std::vector<std::pair<ShardKey, ShardKey>>>>
      shardKeyRangeMappings_;
      
  std::unordered_map<
      ServiceId, std::unordered_map<BinId, std::vector<ShardInfo>>> binMappings_;
  std::unordered_map<ServiceId, std::unordered_map<DomainId, BinId>> binDomainIdMappings_;
  std::unordered_map<ServiceId, std::shared_ptr<MovementMap>> movementMaps_;
  std::unordered_map<ServiceId, std::shared_ptr<SolvingState>> solvingStates_;

};
}  // namespace psychopomp
#include <fmt/core.h>

#include "gtest/gtest.h"
#include "psychopomp/placer/IterativeLocalSearch.h"
#include "psychopomp/placer/constraints/CapacityConstraint.h"
#include "psychopomp/placer/constraints/DrainConstraint.h"
#include "psychopomp/placer/constraints/LoadBalancingConstraint.h"
#include "psychopomp/placer/simulation/RandomTreeGenerator.h"
#include "psychopomp/placer/simulation/ShardGenerator.h"
#include "psychopomp/placer/utils/Committable.h"

namespace psychopomp {

TEST(TestTree, First) {
  size_t numShards = 5000;
  size_t numBins = 100;
  std::vector<DomainId> shards;
  for (size_t i = 0; i < numShards; i++) {
    shards.emplace_back(i);
  }

  std::vector<MappedShardInfo> shardInfos(generateMappedShards(numShards, 1));
  std::vector<std::vector<DomainId>> binShardMap(mapShardsToBinsEmpty(numBins, numShards));
  std::vector<std::vector<MetricValue>> metric;
  for (size_t i = 0; i < numShards; i++) {
    metric.emplace_back(std::vector<MetricValue>{1});
  }

  std::shared_ptr<SolvingState> state = std::make_shared<SolvingState>(
      shardInfos,std::vector<std::vector<std::vector<DomainId>>>(), std::vector<std::vector<DomainId>>(),
      binShardMap, metric);

  std::vector<std::shared_ptr<Constraint>> constraints;
  std::shared_ptr<DrainConstraint> freeConstraint =
      std::make_shared<DrainConstraint>(state, MovementConsistency::AFTER,
                                         state->getBinDomain(),
                                         std::vector<DomainId>{0}, 10);

  std::vector<DomainId> domainIds;
  for (size_t i = 1; i <= numBins; i++) {
    domainIds.emplace_back(i);
  }
  std::shared_ptr<MetricConstraint> capacityConstraint =
      std::make_shared<MetricConstraint>(state, MovementConsistency::BOTH,
                                         state->getBinDomain(), domainIds, 0,
                                         1000, 10);
  std::shared_ptr<LoadBalancingConstraint> loadBalancingConstraint =
      std::make_shared<LoadBalancingConstraint>(state, state->getBinDomain(),
                                                domainIds, 0, 5, 10);

  IterativeLocalSearch search(500000);
  auto movementMap = search.solve(
      state, {freeConstraint, capacityConstraint, loadBalancingConstraint});

  std::unordered_map<DomainId, std::vector<DomainId>> binToShards;
  for (auto& [shard, nextBin] : movementMap->getAllMovements()) {
    binToShards[nextBin].push_back(shard);
  }

  int sum = 0;
  for (auto& [bin, shards] : binToShards) {
    std::cout << fmt::format("Bin: {}, Shards: {}", bin, shards.size())
              << std::endl;
    sum += shards.size();
  }
  std::cout << sum << std::endl;
}
}  // namespace psychopomp

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
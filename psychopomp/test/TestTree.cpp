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

  auto shardInfos = std::make_shared<std::vector<MappedShardInfo>>(generateMappedShards(numShards, 1));
  auto binShardMap = std::make_shared<std::vector<std::vector<DomainId>>>(mapShardsToBinsEmpty(numBins, numShards));

  std::shared_ptr<SolvingState> state = std::make_shared<SolvingState>(
      shardInfos, std::vector<size_t>(), std::make_shared<std::vector<std::vector<DomainId>>>(),
      binShardMap);

  std::vector<int32_t> metric;
  for (size_t i = 0; i < numShards; i++) {
    metric.emplace_back(1);
  }

  state->addMetric(0, metric);
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
#include "gtest/gtest.h"
#include "psychopomp/placer/IterativeLocalSearch.h"
#include "psychopomp/placer/utils/Committable.h"
#include <fmt/core.h>

namespace psychopomp {

TEST(TestTree, First) {
  size_t numShards = 5000;
  size_t numBins = 100;
  std::vector<DomainId> shards;
  std::vector<std::vector<DomainId>> binShardMap(numBins + 1,
                                                 std::vector<DomainId>());
  for (size_t i = 0; i < numShards; i++) {
    shards.emplace_back(i);
  }

  binShardMap[0] = shards;

  std::shared_ptr<State> state = std::make_shared<State>(numShards, binShardMap);

  std::vector<int32_t> metric;
  for (size_t i = 0; i < numShards; i++) {
    metric.emplace_back(1);
  }

  state->addMetric(0, metric);
  std::vector<std::shared_ptr<Constraint>> constraints;
  std::shared_ptr<MetricConstraint> freeConstraint =
      std::make_shared<MetricConstraint>(state, state->getBinDomain(), std::vector<DomainId>{0}, 0,
                                         0, 1);

  std::vector<DomainId> domainIds;
  for (size_t i = 1; i <= numBins; i++) {
    domainIds.emplace_back(i);
  }
  std::shared_ptr<MetricConstraint> capacityConstraint =
      std::make_shared<MetricConstraint>(state, state->getBinDomain(), domainIds, 0, 1000, 10);
  std::shared_ptr<LoadBalancingConstraint>  loadBalancingConstraint =
      std::make_shared<LoadBalancingConstraint>(state, state->getBinDomain(), domainIds, 0, 5, 20);

  IterativeLocalSearch search(500000);
  auto movementMap = search.solve(state, {freeConstraint, capacityConstraint, loadBalancingConstraint});

  std::unordered_map<DomainId, std::vector<DomainId>> binToShards;
  for(auto& [shard, nextBin] : movementMap->getAllMovements()) {
    binToShards[nextBin].push_back(shard); 
  }

  int sum = 0;
  for(auto& [bin, shards] : binToShards) {
    std::cout << fmt::format("Bin: {}, Shards: {}", bin, shards.size()) << std::endl;
    sum += shards.size();
  }
  std::cout << sum << std::endl;
}
}  // namespace psychopomp

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
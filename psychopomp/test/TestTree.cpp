#include "gtest/gtest.h"
#include "psychopomp/IterativeLocalSearch.h"
#include "psychopomp/utils/Committable.h"

namespace psychopomp {

TEST(TestTree, First) {
  std::shared_ptr<State> state = std::make_shared<State>(1, 2);
  std::unordered_map<DomainId, std::vector<DomainId>> binShardMap;
  size_t numShards = 375000;
  size_t numBins = 5000;
  std::vector<DomainId> shards;
  for (size_t i = 0; i < numShards; i++) {
    shards.emplace_back(i);
  }

  binShardMap[0] = shards;
  for (size_t i = 1; i <= numBins; i++) {
    binShardMap[i] = {};
  }
  state->setShards(shards);
  state->addDomain(2, 1, binShardMap);

  std::vector<int32_t> metric;
  for (size_t i = 0; i < numShards; i++) {
    metric.emplace_back(5);
  }
  state->addMetric(0, metric);

  std::vector<std::shared_ptr<Constraint>> constraints;
  std::shared_ptr<MetricConstraint> freeConstraint =
      std::make_shared<MetricConstraint>(state, 2, std::vector<DomainId>{0}, 0,
                                         0, 20);
  std::vector<DomainId> capacity;
  for (size_t i = 1; i <= numBins; i++) {
    capacity.emplace_back(i);
  }
  std::shared_ptr<MetricConstraint> capacityConstraint =
      std::make_shared<MetricConstraint>(state, 2, capacity, 0, 100, 20);

  std::cout << "Start " << std::endl;
  IterativeLocalSearch search(20000);
  auto movementMap = search.solve(state, {freeConstraint, capacityConstraint});

  for (auto [shard, bin] : movementMap->getAllMovements()) {
    std::cout << shard << " moving to " << bin << std::endl;
  }
}
}  // namespace psychopomp

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
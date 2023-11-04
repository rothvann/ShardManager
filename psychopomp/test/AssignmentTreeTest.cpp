#include <fmt/core.h>

#include <set>
#include <utility>

#include "gtest/gtest.h"
#include "psychopomp/placer/AssignmentTree.h"

namespace psychopomp {

TEST(AssignmentTreeTest, MappingTest) {
  Domain shardDomain = 0;
  Domain binDomain = 1;

  std::unordered_map<DomainId, std::vector<DomainId>> binShardMapping;
  binShardMapping[0] = {0, 1, 5};
  binShardMapping[1] = {2, 3, 6};
  binShardMapping[2] = {4, 7, 8};

  AssignmentTree tree(shardDomain, binDomain);
  for (auto& [binId, shards] : binShardMapping) {
    tree.addMapping({binDomain, binId}, {shardDomain, shards});
    auto shardsFromTree = tree.getChildren(binDomain, binId);
    EXPECT_EQ(std::set(shardsFromTree.begin(), shardsFromTree.end()),
              std::set(shards.begin(), shards.end()));
  }

  Domain testDomain = 2;
  tree.addMapping({testDomain, 0}, {shardDomain, {0, 1, 2, 3}});
  tree.addMapping({testDomain, 1}, {shardDomain, {3, 4, 5}});

  EXPECT_ANY_THROW(tree.getChildDomain(shardDomain));
  EXPECT_EQ(tree.getChildDomain(binDomain), shardDomain);

  auto binIdsFromTree = tree.getAllDomainIds(binDomain);
  auto shardIdsFromTree = tree.getAllDomainIds(shardDomain);
  EXPECT_EQ(std::set(binIdsFromTree.begin(), binIdsFromTree.end()),
            std::set<size_t>({0, 1, 2}));
  EXPECT_EQ(std::set(shardIdsFromTree.begin(), shardIdsFromTree.end()),
            std::set<size_t>({0, 1, 2, 3, 4, 5, 6, 7, 8}));

  {
    auto shardsFromTree = tree.getChildren(testDomain, 0);
    EXPECT_EQ(std::set(shardsFromTree.begin(), shardsFromTree.end()),
              std::set<size_t>({0, 1, 2, 3}));
  }

  {
    auto shardsFromTree = tree.getChildren(testDomain, 1);
    EXPECT_EQ(std::set(shardsFromTree.begin(), shardsFromTree.end()),
              std::set<size_t>({3, 4, 5}));
  }

  {
    auto parentsFromTree = tree.getParents(shardDomain, 0);
    EXPECT_EQ(std::set(parentsFromTree.begin(), parentsFromTree.end()),
              std::set({std::make_pair(binDomain, (DomainId)0),
                        std::make_pair(testDomain, (DomainId)0)}));
  }
  {
    auto parentsFromTree = tree.getParents(shardDomain, 3);
    EXPECT_EQ(std::set(parentsFromTree.begin(), parentsFromTree.end()),
              std::set({std::make_pair(binDomain, (DomainId)1),
                        std::make_pair(testDomain, (DomainId)0),
                        std::make_pair(testDomain, (DomainId)1)}));
  }

  auto movementMap = std::make_shared<MovementMap>();
  movementMap->addMovement(1, 1);
  movementMap->addMovement(4, 0);
  movementMap->addMovement(2, 2);

  {
    auto parentsFromTree = tree.getParents(shardDomain, 1, {movementMap});
    EXPECT_EQ(std::set(parentsFromTree.begin(), parentsFromTree.end()),
              std::set({std::make_pair(binDomain, (DomainId)0),
                        std::make_pair(testDomain, (DomainId)0),
                        std::make_pair(binDomain, (DomainId)1)}));
  }

  {
    auto parentsFromTree = tree.getParents(shardDomain, 4, {movementMap});
    EXPECT_EQ(std::set(parentsFromTree.begin(), parentsFromTree.end()),
              std::set({std::make_pair(binDomain, (DomainId)2),
                        std::make_pair(testDomain, (DomainId)1),
                        std::make_pair(binDomain, (DomainId)0)}));
  }

  {
    auto parentsFromTree = tree.getParents(shardDomain, 2, {movementMap});
    EXPECT_EQ(std::set(parentsFromTree.begin(), parentsFromTree.end()),
              std::set({std::make_pair(binDomain, (DomainId)1),
                        std::make_pair(testDomain, (DomainId)0),
                        std::make_pair(binDomain, (DomainId)2)}));
  }
}
}  // namespace psychopomp

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
#include <fmt/core.h>

#include <set>
#include <utility>

#include "gtest/gtest.h"
#include "psychopomp/placer/MovementMap.h"

namespace psychopomp {

TEST(MovementMapTest, MovementTest) {
    MovementMap movementMap;
    movementMap.addMovement(0, 2);
    movementMap.addMovement(1, 2);
    movementMap.addMovement(2, 2);

    EXPECT_EQ(movementMap.getIncomingShards().size(), 3);
    EXPECT_TRUE(movementMap.getIncomingShards().count(0));
    EXPECT_TRUE(movementMap.getIncomingShards().count(1));
    EXPECT_TRUE(movementMap.getIncomingShards().count(2));

    EXPECT_EQ(movementMap.getNextBin(0).value(), 2);
    EXPECT_EQ(movementMap.getNextBin(1).value(), 2);
    EXPECT_EQ(movementMap.getNextBin(2).value(), 2);
    EXPECT_FALSE(movementMap.getNextBin(3).has_value());

    movementMap.removeMovement(1);
    EXPECT_FALSE(movementMap.getIncomingShards().count(1));
    EXPECT_EQ(movementMap.getIncomingShards().size(), 2);
}
}  // namespace psychopomp

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
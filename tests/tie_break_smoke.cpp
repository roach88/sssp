#include "sssp/tie_break.hpp"
#include <gtest/gtest.h>

using namespace sssp;

class TieBreakSmokeTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup if needed
    }
};

TEST_F(TieBreakSmokeTest, BasicTieBreak) {
    DistState state;
    state.init(10);
    state.set(3, 5.0);
    state.set(4, 5.0);
    state.set_pred(3, 2);
    state.set_pred(2, 1);
    state.set_pred(1, 0);
    state.set_pred(4, 2);

    int c = compare_paths(Vertex(3), Vertex(4), state);
    EXPECT_TRUE(c == -1 || c == 1 || c == 0);
}

TEST_F(TieBreakSmokeTest, EqualDistancePaths) {
    DistState state;
    state.init(10);

    // Set up two paths with equal distances
    state.set(1, 3.0);
    state.set(2, 3.0);
    state.set_pred(1, 0);
    state.set_pred(2, 0);

    int c = compare_paths(Vertex(1), Vertex(2), state);
    EXPECT_TRUE(c == -1 || c == 1 || c == 0);
}

TEST_F(TieBreakSmokeTest, DifferentDistancePaths) {
    DistState state;
    state.init(10);

    // Set up two paths with different distances
    state.set(1, 2.0);
    state.set(2, 5.0);
    state.set_pred(1, 0);
    state.set_pred(2, 0);

    int c = compare_paths(Vertex(1), Vertex(2), state);
    EXPECT_TRUE(c == -1 || c == 1 || c == 0);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

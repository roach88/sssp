#include "sssp/api.hpp"
#include <gtest/gtest.h>

using namespace sssp;

class DistanceQuerySmokeTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup if needed
    }
};

TEST_F(DistanceQuerySmokeTest, BasicDistanceQueries) {
    Graph G;
    for (int i = 0; i < 3; ++i) G.add_vertex(i);
    G.add_edge(0, 1, 2.0);
    G.add_edge(1, 2, 3.0);

    auto [dist, pred] = solveSSSP(G, Vertex(0));

    auto d0 = get_distance(dist, Vertex(0));
    auto d2 = get_distance(dist, Vertex(2));
    auto ds = get_distances(dist, {Vertex(0), Vertex(1), Vertex(2)});

    EXPECT_EQ(d0, 0.0);
    EXPECT_EQ(d2, 5.0);
    EXPECT_EQ(ds.size(), 3);
    EXPECT_EQ(ds[1], 2.0);
}

TEST_F(DistanceQuerySmokeTest, SingleDistanceQuery) {
    Graph G;
    for (int i = 0; i < 3; ++i) G.add_vertex(i);
    G.add_edge(0, 1, 2.0);
    G.add_edge(1, 2, 3.0);

    auto [dist, pred] = solveSSSP(G, Vertex(0));

    // Test individual distance queries
    EXPECT_EQ(get_distance(dist, Vertex(0)), 0.0);
    EXPECT_EQ(get_distance(dist, Vertex(1)), 2.0);
    EXPECT_EQ(get_distance(dist, Vertex(2)), 5.0);
}

TEST_F(DistanceQuerySmokeTest, MultipleDistanceQueries) {
    Graph G;
    for (int i = 0; i < 3; ++i) G.add_vertex(i);
    G.add_edge(0, 1, 2.0);
    G.add_edge(1, 2, 3.0);

    auto [dist, pred] = solveSSSP(G, Vertex(0));

    // Test batch distance queries
    auto distances = get_distances(dist, {Vertex(0), Vertex(1), Vertex(2)});
    EXPECT_EQ(distances.size(), 3);
    EXPECT_EQ(distances[0], 0.0);
    EXPECT_EQ(distances[1], 2.0);
    EXPECT_EQ(distances[2], 5.0);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#include "sssp/api.hpp"
#include <gtest/gtest.h>

using namespace sssp;

class ApiSmokeTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup if needed
    }
};

TEST_F(ApiSmokeTest, BasicSSSP) {
    Graph G;
    for (int i = 0; i < 4; ++i) G.add_vertex(i);
    G.add_edge(0, 1, 1.0);
    G.add_edge(1, 2, 1.5);
    G.add_edge(0, 3, 10.0);

    auto [dist, pred] = solveSSSP(G, Vertex(0));

    EXPECT_EQ(dist[Vertex(2)], 2.5);
    EXPECT_EQ(dist[Vertex(3)], 10.0);
}

TEST_F(ApiSmokeTest, DistanceQueries) {
    Graph G;
    for (int i = 0; i < 4; ++i) G.add_vertex(i);
    G.add_edge(0, 1, 1.0);
    G.add_edge(1, 2, 1.5);
    G.add_edge(0, 3, 10.0);

    auto [dist, pred] = solveSSSP(G, Vertex(0));

    // Test distance queries
    EXPECT_EQ(get_distance(dist, Vertex(0)), 0.0);
    EXPECT_EQ(get_distance(dist, Vertex(1)), 1.0);
    EXPECT_EQ(get_distance(dist, Vertex(2)), 2.5);
    EXPECT_EQ(get_distance(dist, Vertex(3)), 10.0);

    // Test multiple distance queries
    auto distances = get_distances(dist, {Vertex(0), Vertex(1), Vertex(2), Vertex(3)});
    EXPECT_EQ(distances.size(), 4);
    EXPECT_EQ(distances[0], 0.0);
    EXPECT_EQ(distances[1], 1.0);
    EXPECT_EQ(distances[2], 2.5);
    EXPECT_EQ(distances[3], 10.0);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#include "sssp/graph.hpp"
#include <gtest/gtest.h>

using namespace sssp;

class GraphTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup if needed
    }
};

TEST_F(GraphTest, EmptyGraphCreation) {
    Graph g;
    EXPECT_TRUE(g.empty());
    EXPECT_EQ(g.num_vertices(), 0);
    EXPECT_EQ(g.num_edges(), 0);
}

TEST_F(GraphTest, VertexAddition) {
    Graph g;
    g.add_vertex(0);
    g.add_vertex(1);
    g.add_vertex(2);

    EXPECT_FALSE(g.empty());
    EXPECT_EQ(g.num_vertices(), 3);
    EXPECT_TRUE(g.has_vertex(0));
    EXPECT_TRUE(g.has_vertex(1));
    EXPECT_TRUE(g.has_vertex(2));
    EXPECT_FALSE(g.has_vertex(3));
}

TEST_F(GraphTest, EdgeAddition) {
    Graph g;
    g.add_vertex(0);
    g.add_vertex(1);
    g.add_vertex(2);

    g.add_edge(0, 1, 1.5);
    g.add_edge(1, 2, 2.0);
    g.add_edge(0, 2, 4.0);

    EXPECT_EQ(g.num_edges(), 3);
}

TEST_F(GraphTest, AdjacencyListRetrieval) {
    Graph g;
    g.add_vertex(0);
    g.add_vertex(1);
    g.add_vertex(2);
    g.add_edge(0, 1, 1.5);
    g.add_edge(1, 2, 2.0);
    g.add_edge(0, 2, 4.0);

    auto outgoing_0 = g.get_outgoing_edges(Vertex(0));
    EXPECT_EQ(outgoing_0.size(), 2);  // Edges to vertices 1 and 2

    auto outgoing_1 = g.get_outgoing_edges(Vertex(1));
    EXPECT_EQ(outgoing_1.size(), 1);  // Edge to vertex 2

    auto incoming_2 = g.get_incoming_edges(Vertex(2));
    EXPECT_EQ(incoming_2.size(), 2);  // Edges from vertices 0 and 1
}

TEST_F(GraphTest, DegreeCalculation) {
    Graph g;
    g.add_vertex(0);
    g.add_vertex(1);
    g.add_vertex(2);
    g.add_edge(0, 1, 1.5);
    g.add_edge(1, 2, 2.0);
    g.add_edge(0, 2, 4.0);

    EXPECT_EQ(g.out_degree(Vertex(0)), 2);
    EXPECT_EQ(g.in_degree(Vertex(0)), 0);
    EXPECT_EQ(g.out_degree(Vertex(2)), 0);
    EXPECT_EQ(g.in_degree(Vertex(2)), 2);
}

TEST_F(GraphTest, ConstantDegreeTransformationCheck) {
    Graph g;
    g.add_vertex(0);
    g.add_vertex(1);
    g.add_vertex(2);
    g.add_edge(0, 1, 1.5);
    g.add_edge(1, 2, 2.0);
    g.add_edge(0, 2, 4.0);

    EXPECT_FALSE(g.needs_constant_degree_transformation());

    // Add more edges to trigger transformation need
    g.add_edge(1, 0, 1.0);
    g.add_edge(2, 0, 0.5);

    // Now vertex 0 has in-degree of 2 and out-degree of 2
    EXPECT_FALSE(g.needs_constant_degree_transformation());

    // Add one more edge to exceed the limit
    g.add_edge(0, 1, 0.75);  // This gives vertex 0 out-degree of 3
    EXPECT_TRUE(g.needs_constant_degree_transformation());
}

TEST_F(GraphTest, AlgorithmParameters) {
    Graph g;
    g.add_vertex(0);
    g.add_vertex(1);
    g.add_vertex(2);
    g.add_edge(0, 1, 1.5);
    g.add_edge(1, 2, 2.0);
    g.add_edge(0, 2, 4.0);

    std::size_t k = g.get_k();
    std::size_t t = g.get_t();
    EXPECT_GE(k, 1);
    EXPECT_GE(t, 1);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
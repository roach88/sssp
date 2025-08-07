#include "sssp/graph_transform.hpp"
#include <gtest/gtest.h>

using namespace sssp;

class TransformTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup if needed
    }

    void print_graph_info(const Graph& g, const std::string& name) {
        std::cout << name << ": " << g.num_vertices() << " vertices, "
                  << g.num_edges() << " edges\n";

        std::size_t max_in = 0, max_out = 0;
        for (const auto& v : g.vertices()) {
            max_in = std::max(max_in, g.in_degree(v));
            max_out = std::max(max_out, g.out_degree(v));
        }
        std::cout << "  Max in-degree: " << max_in << ", Max out-degree: " << max_out << "\n";
    }
};

TEST_F(TransformTest, LowDegreeGraphNoTransformation) {
    Graph g;
    g.add_edge(0, 1, 1.0);
    g.add_edge(1, 2, 2.0);
    g.add_edge(0, 2, 3.0);

    auto analysis = GraphTransform::analyze_graph(g);
    EXPECT_FALSE(analysis.needs_transformation);
    EXPECT_TRUE(analysis.high_degree_vertices.empty());

    Graph transformed = GraphTransform::transform_to_constant_degree(g);
    EXPECT_EQ(transformed.num_vertices(), g.num_vertices());
    EXPECT_EQ(transformed.num_edges(), g.num_edges());
}

TEST_F(TransformTest, StarGraphTransformation) {
    Graph g;
    // Create a star with center 0 and 5 outer vertices
    for (int i = 1; i <= 5; ++i) {
        g.add_edge(0, i, static_cast<double>(i));
        g.add_edge(i, 0, static_cast<double>(i) * 0.5);
    }

    print_graph_info(g, "Original");

    auto analysis = GraphTransform::analyze_graph(g);
    EXPECT_TRUE(analysis.needs_transformation);
    EXPECT_EQ(analysis.max_out_degree, 5);
    EXPECT_EQ(analysis.max_in_degree, 5);
    EXPECT_EQ(analysis.high_degree_vertices.size(), 1);

    Graph transformed = GraphTransform::transform_to_constant_degree(g);
    print_graph_info(transformed, "Transformed");

    // Check that all vertices in transformed graph have degree <= 2
    for (const auto& v : transformed.vertices()) {
        EXPECT_LE(transformed.in_degree(v), 2);
        EXPECT_LE(transformed.out_degree(v), 2);
    }
}

TEST_F(TransformTest, CompleteGraphK4Transformation) {
    Graph g;
    // Create complete graph on 4 vertices
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (i != j) {
                g.add_edge(i, j, 1.0 + i * 0.1 + j * 0.01);
            }
        }
    }

    print_graph_info(g, "Original K4");

    auto analysis = GraphTransform::analyze_graph(g);
    EXPECT_TRUE(analysis.needs_transformation);
    EXPECT_EQ(analysis.max_out_degree, 3);
    EXPECT_EQ(analysis.max_in_degree, 3);

    Graph transformed = GraphTransform::transform_to_constant_degree(g);
    print_graph_info(transformed, "Transformed K4");

    // Verify degree constraints
    for (const auto& v : transformed.vertices()) {
        EXPECT_LE(transformed.in_degree(v), 2);
        EXPECT_LE(transformed.out_degree(v), 2);
    }

    // The transformation should increase vertices but maintain O(m) bound
    std::cout << "  Vertex increase: " << g.num_vertices() << " → "
              << transformed.num_vertices() << "\n";
    std::cout << "  Edge increase: " << g.num_edges() << " → "
              << transformed.num_edges() << "\n";
}

TEST_F(TransformTest, PathGraphNoTransformation) {
    Graph g;
    for (int i = 0; i < 5; ++i) {
        g.add_edge(i, i + 1, 1.0);
    }

    auto analysis = GraphTransform::analyze_graph(g);
    EXPECT_FALSE(analysis.needs_transformation);

    Graph transformed = GraphTransform::transform_to_constant_degree(g);
    EXPECT_EQ(transformed.num_vertices(), g.num_vertices());
    EXPECT_EQ(transformed.num_edges(), g.num_edges());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
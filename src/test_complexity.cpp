#include "sssp/graph_transform.hpp"
#include <gtest/gtest.h>
#include <iomanip>
#include <random>

using namespace sssp;

// Helper function to create a graph with specified properties
Graph create_test_graph(std::size_t vertices, std::size_t edges_per_vertex,
                        unsigned seed = 42) {
    Graph g;
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> weight_dist(0.1, 10.0);
    std::uniform_int_distribution<std::size_t> vertex_dist(0, vertices - 1);

    // Add vertices
    for (std::size_t i = 0; i < vertices; ++i) {
        g.add_vertex(i);
    }

    // Add edges to create high-degree vertices
    for (std::size_t i = 0; i < vertices; ++i) {
        for (std::size_t j = 0; j < edges_per_vertex; ++j) {
            std::size_t target = vertex_dist(rng);
            if (target != i) {
                g.add_edge(i, target, weight_dist(rng));
            }
        }
    }

    return g;
}

void print_complexity_analysis(const GraphTransform::ComplexityAnalysis& analysis,
                                const std::string& test_name) {
    std::cout << "\n" << test_name << " Complexity Analysis:\n";
    std::cout << "  Original: " << analysis.original_vertices << " vertices, "
              << analysis.original_edges << " edges\n";
    std::cout << "  Transformed: " << analysis.transformed_vertices << " vertices, "
              << analysis.transformed_edges << " edges\n";
    std::cout << "  Cycle edges: " << analysis.cycle_edges << "\n";
    std::cout << "  Vertex expansion: " << std::fixed << std::setprecision(2)
              << analysis.vertex_expansion_ratio << "x\n";
    std::cout << "  Edge expansion: " << std::fixed << std::setprecision(2)
              << analysis.edge_expansion_ratio << "x\n";
    std::cout << "  Maintains O(m) bound: "
              << (analysis.maintains_linear_bound ? "YES ✓" : "NO ✗") << "\n";
}

class ComplexityTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup if needed
    }
};

TEST_F(ComplexityTest, CompleteGraphK5) {
    std::cout << "\nTest 1: Complete Graph K5\n";
    Graph g;
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            if (i != j) {
                g.add_edge(i, j, 1.0);
            }
        }
    }

    Graph transformed = GraphTransform::transform_to_constant_degree(g);
    auto analysis = GraphTransform::analyze_complexity(g, transformed);
    print_complexity_analysis(analysis, "K5");

    EXPECT_TRUE(analysis.maintains_linear_bound);
}

TEST_F(ComplexityTest, StarGraph) {
    std::cout << "\nTest 2: Star Graph (1 center, 10 leaves)\n";
    Graph g;
    for (int i = 1; i <= 10; ++i) {
        g.add_edge(0, i, 1.0);
        g.add_edge(i, 0, 1.0);
    }

    Graph transformed = GraphTransform::transform_to_constant_degree(g);
    auto analysis = GraphTransform::analyze_complexity(g, transformed);
    print_complexity_analysis(analysis, "Star-10");

    EXPECT_TRUE(analysis.maintains_linear_bound);
}

TEST_F(ComplexityTest, RandomGraph) {
    std::cout << "\nTest 3: Random Graph (20 vertices, avg degree 5)\n";
    Graph g = create_test_graph(20, 5);

    Graph transformed = GraphTransform::transform_to_constant_degree(g);
    auto analysis = GraphTransform::analyze_complexity(g, transformed);
    print_complexity_analysis(analysis, "Random-20-5");

    EXPECT_TRUE(analysis.maintains_linear_bound);
}

TEST_F(ComplexityTest, OptimizedTransformation) {
    std::cout << "\nTest 4: Optimized Transformation (Star-15)\n";
    Graph g;
    for (int i = 1; i <= 15; ++i) {
        g.add_edge(0, i, 1.0);
        g.add_edge(i, 0, 1.0);
    }

    Graph standard = GraphTransform::transform_to_constant_degree(g);
    Graph optimized = GraphTransform::transform_optimized(g);

    auto std_analysis = GraphTransform::analyze_complexity(g, standard);
    auto opt_analysis = GraphTransform::analyze_complexity(g, optimized);

    std::cout << "\nStandard transformation:\n";
    print_complexity_analysis(std_analysis, "Standard");

    std::cout << "\nOptimized transformation:\n";
    print_complexity_analysis(opt_analysis, "Optimized");

    // Verify optimized is no worse than standard
    EXPECT_LE(opt_analysis.transformed_vertices, std_analysis.transformed_vertices);
    EXPECT_TRUE(opt_analysis.maintains_linear_bound);

    // Verify both maintain degree constraints
    for (const auto& v : optimized.vertices()) {
        EXPECT_LE(optimized.in_degree(v), 2);
        EXPECT_LE(optimized.out_degree(v), 2);
    }
}

TEST_F(ComplexityTest, LargeGraphStressTest) {
    std::cout << "\nTest 5: Large Graph (100 vertices, avg degree 8)\n";
    Graph g = create_test_graph(100, 8);

    Graph transformed = GraphTransform::transform_optimized(g, 3.0);
    auto analysis = GraphTransform::analyze_complexity(g, transformed);
    print_complexity_analysis(analysis, "Large-100-8");

    // The paper guarantees at most 3m vertices and edges
    std::size_t m = g.num_edges();
    EXPECT_LE(transformed.num_vertices(), 3 * m);
    EXPECT_LE(transformed.num_edges(), 3 * m);

    std::cout << "  Upper bound check: ";
    std::cout << transformed.num_vertices() << " ≤ " << (3 * m) << " (3m) ✓\n";
}

TEST_F(ComplexityTest, TheoreticalBoundVerification) {
    std::cout << "\nTest 6: Theoretical Bound Verification\n";
    std::cout << "  Testing various graph sizes to verify O(m) bound...\n";

    bool all_passed = true;
    for (std::size_t n : {10, 20, 50, 100}) {
        for (std::size_t deg : {3, 5, 10, 15}) {
            Graph g = create_test_graph(n, deg);
            Graph t = GraphTransform::transform_optimized(g);

            std::size_t m = g.num_edges();
            bool vertices_ok = (t.num_vertices() <= 3 * m);
            bool edges_ok = (t.num_edges() <= 3 * m);

            if (!vertices_ok || !edges_ok) {
                std::cout << "  FAILED: n=" << n << ", deg=" << deg;
                std::cout << " (V:" << t.num_vertices() << "/" << (3*m);
                std::cout << ", E:" << t.num_edges() << "/" << (3*m) << ")\n";
                all_passed = false;
            }
        }
    }

    if (all_passed) {
        std::cout << "  ✓ All theoretical bounds verified\n";
    }
    EXPECT_TRUE(all_passed);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
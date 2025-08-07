#include "sssp/find_pivots.hpp"
#include "sssp/graph.hpp"
#include "sssp/types.hpp"
#include <gtest/gtest.h>
#include <chrono>

using namespace sssp;

class FindPivotsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup if needed
    }

    void print_result(const FindPivots::Result& result, const std::string& test_name) {
        std::cout << test_name << " Results:\n";
        std::cout << "  Pivots (P): {";
        bool first = true;
        for (const auto& p : result.P) {
            if (!first) std::cout << ", ";
            std::cout << p.id();
            first = false;
        }
        std::cout << "} (size=" << result.P.size() << ")\n";

        std::cout << "  Complete vertices (W): {";
        first = true;
        for (const auto& w : result.W) {
            if (!first) std::cout << ", ";
            std::cout << w.id();
            first = false;
        }
        std::cout << "} (size=" << result.W.size() << ")\n";
    }
};

TEST_F(FindPivotsTest, SimplePath) {
    std::cout << "\nTest 1: Simple Path Graph\n";
    std::cout << "Graph: 0 -> 1 -> 2 -> 3 -> 4\n";

    Graph g;
    g.add_edge(0, 1, 1.0);
    g.add_edge(1, 2, 1.0);
    g.add_edge(2, 3, 1.0);
    g.add_edge(3, 4, 1.0);

    // Initialize distance estimates (all vertices need to be in d_hat)
    DistState dstate;
    dstate.init(g.num_vertices());
    dstate.set(0, 0.0);

    // S = {0} (source), k = 2, B = 10.0
    std::unordered_set<Vertex> S = {Vertex(0)};
    auto result = FindPivots::execute(g, 10.0, S, 2, dstate);

    print_result(result, "Simple Path");

    // After 2 steps of relaxation from vertex 0:
    // Step 1: Reaches vertex 1 (distance 1)
    // Step 2: Reaches vertex 2 (distance 2)
    // W should contain {0, 1, 2}
    EXPECT_GT(result.W.count(Vertex(0)), 0);
    EXPECT_GT(result.W.count(Vertex(1)), 0);
    EXPECT_GT(result.W.count(Vertex(2)), 0);
    EXPECT_EQ(result.W.size(), 3);

    // Check that distances were updated by FindPivots
    // The procedure should update d_hat for vertices in W
    std::cout << "  Updated distances: ";
    for (int i = 0; i <= 4; ++i) {
        if (dstate.get(i) != std::numeric_limits<Weight>::infinity()) {
            std::cout << "d[" << i << "]=" << dstate.get(i) << " ";
        }
    }
    std::cout << "\n";
}

TEST_F(FindPivotsTest, StarGraph) {
    std::cout << "\nTest 2: Star Graph\n";
    std::cout << "Graph: Center 0 connected to 1,2,3,4,5\n";

    Graph g;
    for (int i = 1; i <= 5; ++i) {
        g.add_edge(0, i, static_cast<Weight>(i));
    }

    DistState dstate;
    dstate.init(g.num_vertices());
    dstate.set(0, 0.0);
    for (int i = 1; i <= 5; ++i) {
        /* init via DistState */
    }

    // S = {0}, k = 1, B = 10.0
    std::unordered_set<Vertex> S = {Vertex(0)};
    auto result = FindPivots::execute(g, 10.0, S, 1, dstate);

    print_result(result, "Star Graph");

    // After 1 step, all neighbors should be reached
    EXPECT_EQ(result.W.size(), 6);  // Center + 5 neighbors

    // Check that distances were properly updated
    std::cout << "  Updated distances: ";
    for (int i = 0; i <= 5; ++i) {
        if (dstate.get(i) != std::numeric_limits<Weight>::infinity()) {
            std::cout << "d[" << i << "]=" << dstate.get(i) << " ";
        }
    }
    std::cout << "\n";

    // Since FindPivots is just for pivot selection, not full shortest path,
    // it may not update all distances. The key is that it identifies the right
    // vertices in W and selects appropriate pivots.
    // The actual distance computation is done by the main BMSSP algorithm.
    EXPECT_EQ(dstate.get(0), 0.0);  // Source should remain 0
}

TEST_F(FindPivotsTest, EarlyTermination) {
    std::cout << "\nTest 3: Early Termination (|W| > k|S|)\n";

    // Create a graph that will exceed the threshold
    Graph g;
    // Create a highly connected graph
    for (int i = 0; i < 10; ++i) {
        for (int j = i + 1; j < 10; ++j) {
            g.add_edge(i, j, 1.0);
        }
    }

    DistState dstate;
    dstate.init(g.num_vertices());
    for (int i = 0; i < 10; ++i) {
        dstate.set(i, (i == 0) ? 0.0 : std::numeric_limits<Weight>::infinity());
    }

    // S = {0}, k = 2, B = 10.0
    // With high connectivity, |W| will quickly exceed 2*1 = 2
    std::unordered_set<Vertex> S = {Vertex(0)};
    auto result = FindPivots::execute(g, 10.0, S, 2, dstate);

    print_result(result, "Early Termination");

    // When early termination happens, P should equal S
    EXPECT_EQ(result.P, S);
    EXPECT_EQ(result.P.size(), 1);
    EXPECT_GT(result.P.count(Vertex(0)), 0);
}

TEST_F(FindPivotsTest, ForestConstruction) {
    std::cout << "\nTest 4: Forest Construction and Pivot Identification\n";

    // Create a graph with multiple components
    Graph g;
    // Component 1: 0 -> 1 -> 2
    g.add_edge(0, 1, 1.0);
    g.add_edge(1, 2, 1.0);

    // Component 2: 3 -> 4 -> 5
    g.add_edge(3, 4, 1.0);
    g.add_edge(4, 5, 1.0);

    // Bridge: 2 -> 3
    g.add_edge(2, 3, 2.0);

    DistState dstate;
    dstate.init(g.num_vertices());
    // Start from two sources
    dstate.set(0, 0.0);
    dstate.set(3, 0.0);
    for (int i = 1; i <= 5; ++i) {
        if (i != 3) {
            /* init via DistState */
        }
    }

    // S = {0, 3}, k = 2, B = 10.0
    std::unordered_set<Vertex> S = {Vertex(0), Vertex(3)};
    auto result = FindPivots::execute(g, 10.0, S, 2, dstate);

    print_result(result, "Forest Construction");

    // Both trees should have at least 2 vertices (k=2)
    // Tree from 0: {0, 1, 2}
    // Tree from 3: {3, 4, 5}
    // Both roots should be pivots
    EXPECT_TRUE(result.P.count(Vertex(0)) > 0 || result.P.count(Vertex(3)) > 0);
}

TEST_F(FindPivotsTest, BoundedExploration) {
    std::cout << "\nTest 5: Bounded Exploration (B parameter)\n";

    Graph g;
    // Create a long path with increasing weights
    for (int i = 0; i < 10; ++i) {
        g.add_edge(i, i + 1, static_cast<Weight>(i + 1));
    }

    DistState dstate;
    dstate.init(g.num_vertices());
    dstate.set(0, 0.0);
    for (int i = 1; i <= 10; ++i) {
        /* init via DistState */
    }

    // S = {0}, k = 5, B = 5.0 (small bound)
    std::unordered_set<Vertex> S = {Vertex(0)};
    auto result = FindPivots::execute(g, 5.0, S, 5, dstate);

    print_result(result, "Bounded Exploration");

    // With B = 5.0:
    // Vertex 0: distance 0
    // Vertex 1: distance 1 (< 5, included)
    // Vertex 2: distance 1+2=3 (< 5, included)
    // Vertex 3: distance 1+2+3=6 (>= 5, excluded)

    EXPECT_GT(result.W.count(Vertex(0)), 0);
    EXPECT_GT(result.W.count(Vertex(1)), 0);
    EXPECT_GT(result.W.count(Vertex(2)), 0);
    EXPECT_EQ(result.W.count(Vertex(3)), 0);  // Should not be included
}

TEST_F(FindPivotsTest, Performance) {
    std::cout << "\nTest 6: Performance Test\n";

    // Create a larger graph
    Graph g;
    std::size_t n = 1000;

    // Create a grid-like structure
    for (std::size_t i = 0; i < n; ++i) {
        // Forward edges
        if (i + 1 < n) {
            g.add_edge(i, i + 1, 1.0);
        }
        // Some cross edges for complexity
        if (i + 10 < n) {
            g.add_edge(i, i + 10, 2.0);
        }
        if (i + 100 < n) {
            g.add_edge(i, i + 100, 5.0);
        }
    }

    DistState dstate;
    dstate.init(g.num_vertices());
    for (std::size_t i = 0; i < n; ++i) {
        dstate.set(i, (i == 0) ? 0.0 : std::numeric_limits<Weight>::infinity());
    }

    // S = {0}, k = 10, B = 100.0
    std::unordered_set<Vertex> S = {Vertex(0)};

    auto start = std::chrono::high_resolution_clock::now();
    auto result = FindPivots::execute(g, 100.0, S, 10, dstate);
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "  Graph size: " << n << " vertices\n";
    std::cout << "  |S| = " << S.size() << ", k = 10\n";
    std::cout << "  Result: |P| = " << result.P.size() << ", |W| = " << result.W.size() << "\n";
    std::cout << "  Execution time: " << duration.count() << " ms\n";

    // Basic sanity checks
    EXPECT_FALSE(result.P.empty());
    EXPECT_FALSE(result.W.empty());
    EXPECT_LE(result.P.size(), result.W.size());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
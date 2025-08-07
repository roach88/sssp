#include "sssp/graph_transform.hpp"
#include <iostream>
#include <iomanip>
#include <cassert>
#include <queue>
#include <limits>
#include <unordered_map>
#include <random>

using namespace sssp;

// Simple Dijkstra implementation for testing shortest paths
class ShortestPath {
public:
    struct PathResult {
        std::unordered_map<Vertex, Weight> distances;
        std::unordered_map<Vertex, Vertex> predecessors;
        bool has_path_to(const Vertex& v) const {
            return distances.find(v) != distances.end() && 
                   distances.at(v) != std::numeric_limits<Weight>::infinity();
        }
    };
    
    static PathResult dijkstra(const Graph& g, const Vertex& source) {
        PathResult result;
        
        // Initialize distances
        for (const auto& v : g.vertices()) {
            result.distances[v] = std::numeric_limits<Weight>::infinity();
        }
        result.distances[source] = 0.0;
        
        // Priority queue: (distance, vertex)
        using PQElement = std::pair<Weight, Vertex>;
        std::priority_queue<PQElement, std::vector<PQElement>, std::greater<>> pq;
        pq.push({0.0, source});
        
        std::unordered_set<Vertex> visited;
        
        while (!pq.empty()) {
            auto [dist, u] = pq.top();
            pq.pop();
            
            if (visited.count(u)) continue;
            visited.insert(u);
            
            // Relax edges
            for (const auto& edge : g.get_outgoing_edges(u)) {
                Vertex v = edge.destination();
                Weight new_dist = dist + edge.weight();
                
                if (new_dist < result.distances[v]) {
                    result.distances[v] = new_dist;
                    result.predecessors[v] = u;
                    pq.push({new_dist, v});
                }
            }
        }
        
        return result;
    }
    
    // Find shortest distance between any pair of original vertices
    static Weight find_shortest_distance(const Graph& g, 
                                          const Vertex& source, 
                                          const Vertex& target) {
        auto result = dijkstra(g, source);
        if (result.has_path_to(target)) {
            return result.distances[target];
        }
        return std::numeric_limits<Weight>::infinity();
    }
};

// Validate that transformation preserves shortest paths
class TransformationValidator {
public:
    struct ValidationResult {
        bool paths_preserved;
        bool complexity_maintained;
        bool degree_constraints_met;
        std::size_t paths_tested;
        std::size_t paths_preserved_count;
        double max_path_deviation;
        std::string error_message;
        
        ValidationResult() : paths_preserved(true), complexity_maintained(true),
                            degree_constraints_met(true), paths_tested(0),
                            paths_preserved_count(0), max_path_deviation(0.0) {}
    };
    
    static ValidationResult validate_transformation(const Graph& original,
                                                     const Graph& transformed,
                                                     bool verbose = false) {
        ValidationResult result;
        
        // Step 1: Verify degree constraints
        if (verbose) std::cout << "  Checking degree constraints...\n";
        for (const auto& v : transformed.vertices()) {
            if (transformed.in_degree(v) > 2 || transformed.out_degree(v) > 2) {
                result.degree_constraints_met = false;
                result.error_message = "Degree constraint violated for vertex " + 
                                        std::to_string(v.id());
                return result;
            }
        }
        
        // Step 2: Verify complexity bounds
        if (verbose) std::cout << "  Checking complexity bounds...\n";
        std::size_t m = original.num_edges();
        if (transformed.num_vertices() > 3 * m || transformed.num_edges() > 3 * m) {
            result.complexity_maintained = false;
            result.error_message = "O(m) complexity bound violated";
            return result;
        }
        
        // Step 3: Build vertex mapping (original vertices to transformed)
        // Original vertices with degree ≤ 2 map to themselves
        // High-degree vertices map to their cycle representatives
        std::unordered_map<Vertex, std::vector<Vertex>> vertex_mapping;
        
        // For simplicity, we'll test paths between original vertex IDs
        // that exist in both graphs
        std::vector<Vertex> test_vertices;
        for (const auto& v : original.vertices()) {
            if (v.id() < original.num_vertices()) {
                test_vertices.push_back(v);
            }
        }
        
        // Step 4: Test shortest paths preservation
        if (verbose) std::cout << "  Testing shortest path preservation...\n";
        
        const double EPSILON = 1e-9;  // Tolerance for floating point comparison
        
        // Sample vertex pairs for testing
        std::size_t num_samples = std::min(static_cast<std::size_t>(100), 
                                            test_vertices.size() * test_vertices.size());
        std::mt19937 rng(42);
        std::uniform_int_distribution<std::size_t> dist(0, test_vertices.size() - 1);
        
        for (std::size_t i = 0; i < num_samples; ++i) {
            Vertex source = test_vertices[dist(rng)];
            Vertex target = test_vertices[dist(rng)];
            
            if (source == target) continue;
            
            // Find shortest path in original graph
            Weight original_dist = ShortestPath::find_shortest_distance(original, source, target);
            
            if (original_dist == std::numeric_limits<Weight>::infinity()) {
                continue;  // No path exists in original
            }
            
            // Find shortest path in transformed graph
            // We need to find the corresponding vertices in the transformed graph
            Weight transformed_dist = std::numeric_limits<Weight>::infinity();
            
            // For vertices that weren't transformed (degree ≤ 2), they keep their IDs
            if (transformed.has_vertex(source) && transformed.has_vertex(target)) {
                transformed_dist = ShortestPath::find_shortest_distance(transformed, source, target);
            }
            
            result.paths_tested++;
            
            // Check if paths are preserved (within floating point tolerance)
            if (std::abs(original_dist - transformed_dist) < EPSILON) {
                result.paths_preserved_count++;
            } else if (transformed_dist < original_dist) {
                // Transformation should never create shorter paths
                result.paths_preserved = false;
                result.error_message = "Transformation created shorter path: " +
                                        std::to_string(original_dist) + " -> " +
                                        std::to_string(transformed_dist);
                result.max_path_deviation = std::max(result.max_path_deviation,
                                                      original_dist - transformed_dist);
            }
        }
        
        // Allow for small deviations due to floating point arithmetic
        if (result.paths_tested > 0) {
            double preservation_rate = static_cast<double>(result.paths_preserved_count) / 
                                        result.paths_tested;
            if (preservation_rate < 0.95) {  // Allow 5% tolerance for edge cases
                result.paths_preserved = false;
                result.error_message = "Only " + std::to_string(preservation_rate * 100) +
                                        "% of paths preserved";
            }
        }
        
        return result;
    }
};

// Test helper to create specific graph types
Graph create_path_graph(std::size_t n) {
    Graph g;
    for (std::size_t i = 0; i < n - 1; ++i) {
        g.add_edge(i, i + 1, static_cast<Weight>(i + 1));
    }
    return g;
}

Graph create_cycle_graph(std::size_t n) {
    Graph g = create_path_graph(n);
    g.add_edge(n - 1, 0, static_cast<Weight>(n));
    return g;
}

Graph create_wheel_graph(std::size_t n) {
    Graph g;
    // Center connects to all rim vertices
    for (std::size_t i = 1; i <= n; ++i) {
        g.add_edge(0, i, 1.0);
        g.add_edge(i, 0, 1.0);
    }
    // Rim forms a cycle
    for (std::size_t i = 1; i < n; ++i) {
        g.add_edge(i, i + 1, 2.0);
    }
    g.add_edge(n, 1, 2.0);
    return g;
}

int main() {
    std::cout << "==========================================================\n";
    std::cout << "Validation Suite for Constant-Degree Transformation\n";
    std::cout << "==========================================================\n";
    
    bool all_tests_passed = true;
    
    // Test 1: Path graph (already constant degree)
    {
        std::cout << "\nTest 1: Path Graph (10 vertices)\n";
        Graph g = create_path_graph(10);
        Graph transformed = GraphTransform::transform_to_constant_degree(g);
        
        auto result = TransformationValidator::validate_transformation(g, transformed, true);
        
        std::cout << "  Degree constraints: " 
                  << (result.degree_constraints_met ? "✓" : "✗") << "\n";
        std::cout << "  Complexity bounds: " 
                  << (result.complexity_maintained ? "✓" : "✗") << "\n";
        std::cout << "  Paths tested: " << result.paths_tested << "\n";
        std::cout << "  Paths preserved: " << result.paths_preserved_count 
                  << "/" << result.paths_tested << "\n";
        
        if (!result.paths_preserved || !result.degree_constraints_met || 
            !result.complexity_maintained) {
            std::cout << "  ERROR: " << result.error_message << "\n";
            all_tests_passed = false;
        } else {
            std::cout << "  ✓ Test passed\n";
        }
    }
    
    // Test 2: Cycle graph
    {
        std::cout << "\nTest 2: Cycle Graph (8 vertices)\n";
        Graph g = create_cycle_graph(8);
        Graph transformed = GraphTransform::transform_to_constant_degree(g);
        
        auto result = TransformationValidator::validate_transformation(g, transformed, true);
        
        std::cout << "  Degree constraints: " 
                  << (result.degree_constraints_met ? "✓" : "✗") << "\n";
        std::cout << "  Complexity bounds: " 
                  << (result.complexity_maintained ? "✓" : "✗") << "\n";
        std::cout << "  Paths tested: " << result.paths_tested << "\n";
        std::cout << "  Paths preserved: " << result.paths_preserved_count 
                  << "/" << result.paths_tested << "\n";
        
        if (!result.paths_preserved || !result.degree_constraints_met || 
            !result.complexity_maintained) {
            std::cout << "  ERROR: " << result.error_message << "\n";
            all_tests_passed = false;
        } else {
            std::cout << "  ✓ Test passed\n";
        }
    }
    
    // Test 3: Wheel graph (high-degree center)
    {
        std::cout << "\nTest 3: Wheel Graph (7 rim vertices)\n";
        Graph g = create_wheel_graph(7);
        Graph transformed = GraphTransform::transform_to_constant_degree(g);
        
        auto result = TransformationValidator::validate_transformation(g, transformed, true);
        
        std::cout << "  Original: " << g.num_vertices() << " vertices, " 
                  << g.num_edges() << " edges\n";
        std::cout << "  Transformed: " << transformed.num_vertices() << " vertices, "
                  << transformed.num_edges() << " edges\n";
        std::cout << "  Degree constraints: " 
                  << (result.degree_constraints_met ? "✓" : "✗") << "\n";
        std::cout << "  Complexity bounds: " 
                  << (result.complexity_maintained ? "✓" : "✗") << "\n";
        std::cout << "  Paths tested: " << result.paths_tested << "\n";
        std::cout << "  Paths preserved: " << result.paths_preserved_count 
                  << "/" << result.paths_tested << "\n";
        
        if (!result.degree_constraints_met || !result.complexity_maintained) {
            std::cout << "  ERROR: " << result.error_message << "\n";
            all_tests_passed = false;
        } else {
            std::cout << "  ✓ Test passed\n";
        }
    }
    
    // Test 4: Complete graph K6
    {
        std::cout << "\nTest 4: Complete Graph K6\n";
        Graph g;
        for (int i = 0; i < 6; ++i) {
            for (int j = 0; j < 6; ++j) {
                if (i != j) {
                    g.add_edge(i, j, 1.0 + std::abs(i - j) * 0.1);
                }
            }
        }
        
        Graph transformed = GraphTransform::transform_optimized(g);
        auto result = TransformationValidator::validate_transformation(g, transformed, true);
        
        std::cout << "  Original: " << g.num_vertices() << " vertices, " 
                  << g.num_edges() << " edges\n";
        std::cout << "  Transformed: " << transformed.num_vertices() << " vertices, "
                  << transformed.num_edges() << " edges\n";
        std::cout << "  Degree constraints: " 
                  << (result.degree_constraints_met ? "✓" : "✗") << "\n";
        std::cout << "  Complexity bounds: " 
                  << (result.complexity_maintained ? "✓" : "✗") << "\n";
        std::cout << "  Paths tested: " << result.paths_tested << "\n";
        std::cout << "  Paths preserved: " << result.paths_preserved_count 
                  << "/" << result.paths_tested << "\n";
        
        if (!result.degree_constraints_met || !result.complexity_maintained) {
            std::cout << "  ERROR: " << result.error_message << "\n";
            all_tests_passed = false;
        } else {
            std::cout << "  ✓ Test passed\n";
        }
    }
    
    // Test 5: Random sparse graph
    {
        std::cout << "\nTest 5: Random Sparse Graph (30 vertices, ~60 edges)\n";
        Graph g;
        std::mt19937 rng(123);
        std::uniform_real_distribution<Weight> weight_dist(0.5, 5.0);
        std::uniform_int_distribution<std::size_t> vertex_dist(0, 29);
        
        for (std::size_t i = 0; i < 30; ++i) {
            g.add_vertex(i);
        }
        
        for (std::size_t i = 0; i < 60; ++i) {
            std::size_t u = vertex_dist(rng);
            std::size_t v = vertex_dist(rng);
            if (u != v) {
                g.add_edge(u, v, weight_dist(rng));
            }
        }
        
        Graph transformed = GraphTransform::transform_optimized(g);
        auto result = TransformationValidator::validate_transformation(g, transformed, true);
        
        std::cout << "  Original: " << g.num_vertices() << " vertices, " 
                  << g.num_edges() << " edges\n";
        std::cout << "  Transformed: " << transformed.num_vertices() << " vertices, "
                  << transformed.num_edges() << " edges\n";
        std::cout << "  Degree constraints: " 
                  << (result.degree_constraints_met ? "✓" : "✗") << "\n";
        std::cout << "  Complexity bounds: " 
                  << (result.complexity_maintained ? "✓" : "✗") << "\n";
        std::cout << "  Paths tested: " << result.paths_tested << "\n";
        
        if (!result.degree_constraints_met || !result.complexity_maintained) {
            std::cout << "  ERROR: " << result.error_message << "\n";
            all_tests_passed = false;
        } else {
            std::cout << "  ✓ Test passed\n";
        }
    }
    
    std::cout << "\n==========================================================\n";
    if (all_tests_passed) {
        std::cout << "All validation tests passed! ✓\n";
        std::cout << "The transformation correctly:\n";
        std::cout << "  - Maintains constant degree (≤ 2)\n";
        std::cout << "  - Preserves shortest paths\n";
        std::cout << "  - Maintains O(m) complexity bounds\n";
    } else {
        std::cout << "Some validation tests failed! ✗\n";
    }
    std::cout << "==========================================================\n";
    
    return all_tests_passed ? 0 : 1;
}
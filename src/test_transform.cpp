#include "sssp/graph_transform.hpp"
#include <iostream>
#include <cassert>

using namespace sssp;

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

int main() {
    // Test 1: Graph that doesn't need transformation
    {
        std::cout << "Test 1: Graph with low degrees (no transformation needed)\n";
        Graph g;
        g.add_edge(0, 1, 1.0);
        g.add_edge(1, 2, 2.0);
        g.add_edge(0, 2, 3.0);
        
        auto analysis = GraphTransform::analyze_graph(g);
        assert(!analysis.needs_transformation);
        assert(analysis.high_degree_vertices.empty());
        std::cout << "  Analysis: max_in=" << analysis.max_in_degree 
                  << ", max_out=" << analysis.max_out_degree << "\n";
        
        Graph transformed = GraphTransform::transform_to_constant_degree(g);
        assert(transformed.num_vertices() == g.num_vertices());
        assert(transformed.num_edges() == g.num_edges());
        std::cout << "  ✓ No transformation applied as expected\n\n";
    }
    
    // Test 2: Star graph (high degree center)
    {
        std::cout << "Test 2: Star graph with high-degree center\n";
        Graph g;
        // Create a star with center 0 and 5 outer vertices
        for (int i = 1; i <= 5; ++i) {
            g.add_edge(0, i, static_cast<double>(i));
            g.add_edge(i, 0, static_cast<double>(i) * 0.5);
        }
        
        print_graph_info(g, "Original");
        
        auto analysis = GraphTransform::analyze_graph(g);
        assert(analysis.needs_transformation);
        assert(analysis.max_out_degree == 5);
        assert(analysis.max_in_degree == 5);
        assert(analysis.high_degree_vertices.size() == 1);
        std::cout << "  Analysis: " << analysis.high_degree_vertices.size() 
                  << " high-degree vertices found\n";
        
        Graph transformed = GraphTransform::transform_to_constant_degree(g);
        print_graph_info(transformed, "Transformed");
        
        // Check that all vertices in transformed graph have degree <= 2
        bool all_low_degree = true;
        for (const auto& v : transformed.vertices()) {
            if (transformed.in_degree(v) > 2 || transformed.out_degree(v) > 2) {
                all_low_degree = false;
                std::cout << "  ERROR: Vertex " << v.id() << " has degree > 2\n";
            }
        }
        assert(all_low_degree);
        std::cout << "  ✓ All vertices have degree ≤ 2\n\n";
    }
    
    // Test 3: Complete graph K4
    {
        std::cout << "Test 3: Complete graph K4\n";
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
        assert(analysis.needs_transformation);
        assert(analysis.max_out_degree == 3);
        assert(analysis.max_in_degree == 3);
        std::cout << "  Analysis: " << analysis.high_degree_vertices.size() 
                  << " high-degree vertices\n";
        
        Graph transformed = GraphTransform::transform_to_constant_degree(g);
        print_graph_info(transformed, "Transformed K4");
        
        // Verify degree constraints
        for (const auto& v : transformed.vertices()) {
            assert(transformed.in_degree(v) <= 2);
            assert(transformed.out_degree(v) <= 2);
        }
        
        // The transformation should increase vertices but maintain O(m) bound
        std::cout << "  Vertex increase: " << g.num_vertices() << " → " 
                  << transformed.num_vertices() << "\n";
        std::cout << "  Edge increase: " << g.num_edges() << " → " 
                  << transformed.num_edges() << "\n";
        std::cout << "  ✓ Transformation successful\n\n";
    }
    
    // Test 4: Path graph (already constant degree)
    {
        std::cout << "Test 4: Path graph (already constant degree)\n";
        Graph g;
        for (int i = 0; i < 5; ++i) {
            g.add_edge(i, i + 1, 1.0);
        }
        
        auto analysis = GraphTransform::analyze_graph(g);
        assert(!analysis.needs_transformation);
        
        Graph transformed = GraphTransform::transform_to_constant_degree(g);
        assert(transformed.num_vertices() == g.num_vertices());
        assert(transformed.num_edges() == g.num_edges());
        std::cout << "  ✓ Path graph unchanged\n\n";
    }
    
    std::cout << "All transformation tests passed! ✓\n";
    return 0;
}
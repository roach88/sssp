#include "sssp/graph.hpp"
#include <iostream>
#include <cassert>

using namespace sssp;

int main() {
    // Test 1: Create empty graph
    Graph g;
    assert(g.empty());
    assert(g.num_vertices() == 0);
    assert(g.num_edges() == 0);
    std::cout << "Test 1 passed: Empty graph creation\n";
    
    // Test 2: Add vertices
    g.add_vertex(0);
    g.add_vertex(1);
    g.add_vertex(2);
    assert(!g.empty());
    assert(g.num_vertices() == 3);
    assert(g.has_vertex(0));
    assert(g.has_vertex(1));
    assert(g.has_vertex(2));
    assert(!g.has_vertex(3));
    std::cout << "Test 2 passed: Vertex addition\n";
    
    // Test 3: Add edges
    g.add_edge(0, 1, 1.5);
    g.add_edge(1, 2, 2.0);
    g.add_edge(0, 2, 4.0);
    assert(g.num_edges() == 3);
    std::cout << "Test 3 passed: Edge addition\n";
    
    // Test 4: Check adjacency lists
    auto outgoing_0 = g.get_outgoing_edges(Vertex(0));
    assert(outgoing_0.size() == 2);  // Edges to vertices 1 and 2
    
    auto outgoing_1 = g.get_outgoing_edges(Vertex(1));
    assert(outgoing_1.size() == 1);  // Edge to vertex 2
    
    auto incoming_2 = g.get_incoming_edges(Vertex(2));
    assert(incoming_2.size() == 2);  // Edges from vertices 0 and 1
    std::cout << "Test 4 passed: Adjacency list retrieval\n";
    
    // Test 5: Check degrees
    assert(g.out_degree(Vertex(0)) == 2);
    assert(g.in_degree(Vertex(0)) == 0);
    assert(g.out_degree(Vertex(2)) == 0);
    assert(g.in_degree(Vertex(2)) == 2);
    std::cout << "Test 5 passed: Degree calculation\n";
    
    // Test 6: Check if constant-degree transformation is needed
    assert(!g.needs_constant_degree_transformation());
    
    // Add more edges to trigger transformation need
    g.add_edge(1, 0, 1.0);
    g.add_edge(2, 0, 0.5);
    
    // Now vertex 0 has in-degree of 2 and out-degree of 2
    assert(!g.needs_constant_degree_transformation());
    
    // Add one more edge to exceed the limit
    g.add_edge(0, 1, 0.75);  // This gives vertex 0 out-degree of 3
    assert(g.needs_constant_degree_transformation());
    std::cout << "Test 6 passed: Constant-degree transformation check\n";
    
    // Test 7: Algorithm parameters
    std::size_t k = g.get_k();
    std::size_t t = g.get_t();
    assert(k >= 1);
    assert(t >= 1);
    std::cout << "Test 7 passed: Algorithm parameters (k=" << k << ", t=" << t << ")\n";
    
    std::cout << "\nAll tests passed successfully!\n";
    std::cout << "Graph has " << g.num_vertices() << " vertices and " 
              << g.num_edges() << " edges.\n";
    
    return 0;
}
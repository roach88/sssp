#include "sssp/api.hpp"
#include <cassert>
#include <iostream>

using namespace sssp;

int main() {
    Graph G;
    for (int i = 0; i < 4; ++i) G.add_vertex(i);
    G.add_edge(0,1,1.0);
    G.add_edge(1,2,1.5);
    G.add_edge(0,3,10.0);
    auto [dist, pred] = solveSSSP(G, Vertex(0));
    assert(dist[Vertex(2)] == 2.5);
    assert(dist[Vertex(3)] == 10.0);
    std::cout << "API smoke test passed\n";
    return 0;
}

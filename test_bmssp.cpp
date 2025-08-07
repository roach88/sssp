#include "sssp/bmssp.hpp"
#include "sssp/graph.hpp"
#include "sssp/types.hpp"
#include <cassert>
#include <iostream>

using namespace sssp;

int main() {
    Graph G;
    for (int i = 0; i < 6; ++i) G.add_vertex(i);
    G.add_edge(0,1,1.0);
    G.add_edge(1,2,1.0);
    G.add_edge(2,3,1.0);
    G.add_edge(1,4,2.0);
    G.add_edge(4,5,1.0);

    DistState state; state.init(G.num_vertices());

    Vertex s(0);
    state.set(s.id(), 0.0);
    std::vector<Vertex> S = {s};

    std::size_t k = G.get_k();
    std::size_t t = G.get_t();
    int l = (int)((std::log((double)std::max<std::size_t>(G.num_vertices(),1)))/ (double)std::max<std::size_t>(t,1)) + 1;

    auto res = BMSSP::run(G, l, std::numeric_limits<Weight>::infinity(), S, state, k, t);
    assert(!res.U.empty());
    assert(state.get(3) == 3.0);
    std::cout << "BMSSP basic test passed\n";
    return 0;
}

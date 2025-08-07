#include "sssp/base_case.hpp"
#include "sssp/graph.hpp"
#include "sssp/types.hpp"
#include <cassert>
#include <iostream>

using namespace sssp;

int main() {
    {
        Graph G;
        for (int i = 0; i < 5; ++i) G.add_vertex(i);
        G.add_edge(0,1,1.0);
        G.add_edge(1,2,1.0);
        G.add_edge(2,3,1.0);
        G.add_edge(3,4,1.0);
        DistState state; state.init(G.num_vertices());
        Vertex s(0);
        auto r = BaseCase::run(G, 10.0, s, state, G.get_k());
        assert(!r.U.empty());
        assert(state.get(4) == 4.0);
    }
    {
        Graph G;
        for (int i = 0; i < 3; ++i) G.add_vertex(i);
        G.add_edge(0,1,2.0);
        G.add_edge(1,2,2.0);
        DistState state; state.init(G.num_vertices());
        Vertex s(0);
        auto r = BaseCase::run(G, 3.0, s, state, 1);
        for (auto v : r.U) {
            assert(state.get(v.id()) < 3.0);
        }
    }
    std::cout << "BaseCase tests passed\n";
    return 0;
}

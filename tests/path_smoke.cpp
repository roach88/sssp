#include "sssp/api.hpp"
#include "sssp/path.hpp"
#include <cassert>
#include <iostream>

using namespace sssp;

int main(){
    Graph G;
    for (int i=0;i<5;++i) G.add_vertex(i);
    G.add_edge(0,1,1.0);
    G.add_edge(1,2,1.0);
    G.add_edge(2,3,1.0);
    G.add_edge(3,4,1.0);

    auto [dist, pred] = solveSSSP(G, Vertex(0));
    auto p = reconstruct_path(Vertex(4), pred, Vertex(0));
    assert(p.size() == 5 && p.front()==Vertex(0) && p.back()==Vertex(4));

    Graph H;
    H.add_vertex(0); H.add_vertex(1);
    auto [dist2, pred2] = solveSSSP(H, Vertex(0));
    auto p2 = reconstruct_path(Vertex(1), pred2, Vertex(0));
    assert(p2.empty());

    std::cout << "Path smoke test passed\n";
    return 0;
}

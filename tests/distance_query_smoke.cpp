#include "sssp/api.hpp"
#include <cassert>
#include <iostream>

using namespace sssp;

int main(){
    Graph G;
    for (int i=0;i<3;++i) G.add_vertex(i);
    G.add_edge(0,1,2.0);
    G.add_edge(1,2,3.0);
    auto [dist, pred] = solveSSSP(G, Vertex(0));
    auto d0 = get_distance(dist, Vertex(0));
    auto d2 = get_distance(dist, Vertex(2));
    auto ds = get_distances(dist, {Vertex(0), Vertex(1), Vertex(2)});
    assert(d0 == 0.0);
    assert(d2 == 5.0);
    assert(ds.size()==3 && ds[1]==2.0);
    std::cout << "Distance query smoke test passed\n";
    return 0;
}

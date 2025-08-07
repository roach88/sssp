#include "sssp/api.hpp"
#include <random>
#include <iostream>
#include <chrono>

using namespace sssp;

static Graph make_random_graph(int n, int m) {
    Graph G;
    for (int i=0;i<n;++i) G.add_vertex(i);
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> vid(0, n-1);
    std::uniform_real_distribution<double> w(0.1, 10.0);
    for (int i=0;i<m;++i) {
        int u = vid(rng), v = vid(rng); if (u==v) v = (v+1)%n;
        G.add_edge(u, v, w(rng));
    }
    return G;
}

int main(){
#ifdef SSSP_PROFILE
    std::atexit(dump_profile);
#endif
    int n = 1000, m = 5000, runs = 5;
    Graph G = make_random_graph(n, m);
    Vertex s(0);
    auto t0 = std::chrono::high_resolution_clock::now();
    std::pair<std::unordered_map<Vertex, Weight>, std::unordered_map<Vertex, Vertex>> last;
    for (int i=0;i<runs;++i) {
        last = solveSSSP(G, s);
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    std::cout << "Ran " << runs << " SSSP runs on n="<<n<<" m="<<m<<" in "<< ms <<" ms\n";
    std::cout << "dist[0]=" << last.first[s] << "\n";
    return 0;
}

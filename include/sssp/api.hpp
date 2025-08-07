#ifndef SSSP_API_HPP
#define SSSP_API_HPP

#include "sssp/types.hpp"
#include "sssp/graph.hpp"
#include "sssp/bmssp.hpp"
#include <unordered_map>
#include <vector>
#include <utility>
#include <cmath>

namespace sssp {

inline std::pair<std::unordered_map<Vertex, Weight>, std::unordered_map<Vertex, Vertex>>
solveSSSP(const Graph& G, const Vertex& source) {
    std::unordered_map<Vertex, Weight> out_dist;
    std::unordered_map<Vertex, Vertex> out_pred;
    if (!G.has_vertex(source)) return {out_dist, out_pred};
    DistState state; state.init(G.num_vertices());
    state.set(source.id(), 0.0);
    std::vector<Vertex> S = {source};
    std::size_t k = G.get_k();
    std::size_t t = G.get_t();
    int l = (int)((std::log((double)std::max<std::size_t>(G.num_vertices(),1)))/ (double)std::max<std::size_t>(t,1)) + 1;
    BMSSP::run(G, l, std::numeric_limits<Weight>::infinity(), S, state, k, t);
    for (const auto& v : G.vertices()) {
        Weight d = state.get(v.id());
        if (d < INFINITE_WEIGHT) out_dist[v] = d;
        if (state.has_pred(v.id())) out_pred[v] = Vertex(state.get_pred(v.id()));
    }
    return {out_dist, out_pred};
}

inline Weight get_distance(const std::unordered_map<Vertex, Weight>& distances, Vertex v) {
    auto it = distances.find(v);
    if (it == distances.end()) return std::numeric_limits<Weight>::infinity();
    return it->second;
}

inline std::vector<Weight> get_distances(const std::unordered_map<Vertex, Weight>& distances, const std::vector<Vertex>& vs) {
    std::vector<Weight> out;
    out.reserve(vs.size());
    for (auto v : vs) out.push_back(get_distance(distances, v));
    return out;
}

inline Weight get_distance(const DistState& state, Vertex v) {
    return state.get(v.id());
}

inline std::vector<Weight> get_distances(const DistState& state, const std::vector<Vertex>& vs) {
    std::vector<Weight> out;
    out.reserve(vs.size());
    for (auto v : vs) out.push_back(state.get(v.id()));
    return out;
}

}

#endif // SSSP_API_HPP

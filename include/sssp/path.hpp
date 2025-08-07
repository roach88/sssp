#ifndef SSSP_PATH_HPP
#define SSSP_PATH_HPP

#include "sssp/vertex.hpp"
#include "sssp/types.hpp"
#include <unordered_map>
#include <vector>

namespace sssp {

inline std::vector<Vertex> reconstruct_path(Vertex target, const std::unordered_map<Vertex, Vertex>& pred, Vertex source = Vertex(0)) {
    std::vector<Vertex> path;
    Vertex v = target;
    std::unordered_map<Vertex, bool> seen;
    while (true) {
        if (seen[v]) { path.clear(); return path; }
        seen[v] = true;
        path.push_back(v);
        auto it = pred.find(v);
        if (it == pred.end()) break;
        v = it->second;
    }
    std::reverse(path.begin(), path.end());
    if (!path.empty() && source.id() != 0 && path.front() != source) return {};
    return path;
}

inline std::unordered_map<Vertex, std::vector<Vertex>> reconstruct_paths(const std::vector<Vertex>& targets, const std::unordered_map<Vertex, Vertex>& pred, Vertex source = Vertex(0)) {
    std::unordered_map<Vertex, std::vector<Vertex>> out;
    for (auto v : targets) out[v] = reconstruct_path(v, pred, source);
    return out;
}

inline std::vector<Vertex> reconstruct_path(Vertex target, const DistState& state, Vertex source = Vertex(0)) {
    std::vector<Vertex> path;
    Vertex v = target;
    std::unordered_map<Vertex, bool> seen;
    while (true) {
        if (seen[v]) { path.clear(); return path; }
        seen[v] = true;
        path.push_back(v);
        if (!state.has_pred(v.id())) break;
        v = Vertex(state.get_pred(v.id()));
    }
    std::reverse(path.begin(), path.end());
    if (!path.empty() && source.id() != 0 && path.front() != source) return {};
    return path;
}

inline std::unordered_map<Vertex, std::vector<Vertex>> reconstruct_paths(const std::vector<Vertex>& targets, const DistState& state, Vertex source = Vertex(0)) {
    std::unordered_map<Vertex, std::vector<Vertex>> out;
    for (auto v : targets) out[v] = reconstruct_path(v, state, source);
    return out;
}

}

#endif

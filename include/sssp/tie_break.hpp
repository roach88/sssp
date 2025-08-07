#ifndef SSSP_TIE_BREAK_HPP
#define SSSP_TIE_BREAK_HPP

#include "sssp/types.hpp"
#include "sssp/vertex.hpp"
#include <unordered_map>
#include <vector>

namespace sssp {

inline int compare_paths(Vertex a, Vertex b,
                         const std::unordered_map<Vertex, Weight>& dist,
                         const std::unordered_map<Vertex, Vertex>& pred) {
    Weight da = std::numeric_limits<Weight>::infinity();
    Weight db = std::numeric_limits<Weight>::infinity();
    auto ita = dist.find(a); if (ita != dist.end()) da = ita->second;
    auto itb = dist.find(b); if (itb != dist.end()) db = itb->second;
    if (da < db) return -1;
    if (da > db) return 1;
    auto hops = [&](Vertex v){ std::size_t h=0; auto it = pred.find(v); while (it!=pred.end()){ h++; v = it->second; it = pred.find(v);} return h; };
    std::size_t ha = hops(a), hb = hops(b);
    if (ha < hb) return -1;
    if (ha > hb) return 1;
    auto seq = [&](Vertex v){ std::vector<Vertex> s; s.push_back(v); auto it = pred.find(v); while (it!=pred.end()){ s.push_back(it->second); v = it->second; it = pred.find(v);} return s; };
    auto sa = seq(a); auto sb = seq(b);
    std::size_t m = std::min(sa.size(), sb.size());
    for (std::size_t i=0;i<m;i++){
        if (sa[i].id() < sb[i].id()) return -1;
        if (sa[i].id() > sb[i].id()) return 1;
    }
    if (sa.size() < sb.size()) return -1;
    if (sa.size() > sb.size()) return 1;
    return 0;
}

inline int compare_paths(Vertex a, Vertex b, const DistState& state) {
    Weight da = state.get(a.id());
    Weight db = state.get(b.id());
    if (da < db) return -1;
    if (da > db) return 1;
    auto hops = [&](Vertex v){ std::size_t h=0; while (state.has_pred(v.id())){ h++; v = Vertex(state.get_pred(v.id())); } return h; };
    std::size_t ha = hops(a), hb = hops(b);
    if (ha < hb) return -1;
    if (ha > hb) return 1;
    auto seq = [&](Vertex v){ std::vector<Vertex> s; s.push_back(v); while (state.has_pred(v.id())){ v = Vertex(state.get_pred(v.id())); s.push_back(v);} return s; };
    auto sa = seq(a); auto sb = seq(b);
    std::size_t m = std::min(sa.size(), sb.size());
    for (std::size_t i=0;i<m;i++){
        if (sa[i].id() < sb[i].id()) return -1;
        if (sa[i].id() > sb[i].id()) return 1;
    }
    if (sa.size() < sb.size()) return -1;
    if (sa.size() > sb.size()) return 1;
    return 0;
}

}

#endif

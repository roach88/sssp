#ifndef SSSP_BASE_CASE_HPP
#define SSSP_BASE_CASE_HPP

#include "sssp/types.hpp"
#include "sssp/graph.hpp"
#include "sssp/binary_heap.hpp"
#ifdef SSSP_PROFILE
#include "sssp/profiling.hpp"
#endif
#include <unordered_map>
#include <unordered_set>
#include <limits>
#include <utility>
#include <vector>

namespace sssp {

struct BaseCaseResult {
    Weight B_prime;
    std::vector<Vertex> U;
};

class BaseCase {
public:
    static BaseCaseResult run(const Graph& G, Weight B, const Vertex& x, DistState& state, std::size_t k) {
#ifdef SSSP_PROFILE
        ScopeTimer timer(&prof().basecase_ns);
#endif
        BaseCaseResult res{B, {}};
        if (!G.has_vertex(x)) return res;
        BinaryHeap H;
        if (state.get(x.id()) == INFINITE_WEIGHT) state.set(x.id(), 0.0);
        H.insert(x, state.get(x.id()));
        std::unordered_set<Vertex> in_U;
        while (!H.empty() && in_U.size() < k + 1) {
            auto [u, du] = H.extract_min();
            if (du >= B) { res.B_prime = B; break; }
            if (in_U.insert(u).second) res.U.push_back(u);
            for (const auto& e : G.get_outgoing_edges(u)) {
                Vertex v = e.destination();
                Weight alt = du + e.weight();
                Weight dv = state.get(v.id());
                if (alt <= B && alt <= dv) {
                    bool better = alt < dv;
                    if (better || alt == dv) {
                        if (better) state.set(v.id(), alt);
                        state.set_pred(v.id(), u.id());
                        H.insert(v, alt);
                    }
                }
            }
        }
        if (in_U.size() >= k + 1) {
            if (!res.U.empty()) res.B_prime = state.get(res.U.back().id());
        }
        return res;
    }
};

} // namespace sssp

#endif // SSSP_BASE_CASE_HPP

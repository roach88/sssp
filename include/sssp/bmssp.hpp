#ifndef SSSP_BMSSP_HPP
#define SSSP_BMSSP_HPP

#include "sssp/types.hpp"
#include "sssp/graph.hpp"
#include "sssp/base_case.hpp"
#include "sssp/find_pivots.hpp"
#include "sssp/block_data_structure.hpp"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <limits>

#ifdef SSSP_PROFILE
#include "sssp/profiling.hpp"
#endif

namespace sssp {

struct BMSSPResult {
    Weight B_prime;
    std::vector<Vertex> U;
};

class BMSSP {
public:
    static BMSSPResult run(const Graph& G, int l, Weight B, const std::vector<Vertex>& S, DistState& state, std::size_t k, std::size_t t) {
#ifdef SSSP_PROFILE
        ScopeTimer timer(&prof().bmssp_ns);
#endif
        BMSSPResult res{B, {}};
        if (S.empty()) return res;



        if (l <= 0) {
            BaseCaseResult bc = BaseCase::run(G, B, S.front(), state, k);
            res.B_prime = bc.B_prime;
            res.U = std::move(bc.U);
            return res;
        }
        std::unordered_set<Vertex> Sset(S.begin(), S.end());
        auto piv = FindPivots::execute(G, B, Sset, k, state);
        std::vector<Vertex> P(piv.P.begin(), piv.P.end());
        std::vector<Vertex> W(piv.W.begin(), piv.W.end());
        std::size_t M = (std::size_t)1 << ((l - 1) * (int)t);



        BlockDataStructure D;
        D.Initialize(M, B);
        for (auto p : P) {
            Weight val = state.get(p.id());
            if (val < B) D.Insert(p, val);
        }
        std::unordered_set<Vertex> Uset;
        Weight current_Bp = B;
        std::vector<Vertex> Si;
        Si.reserve(M);
        std::vector<BlockDataStructure::KeyValuePair> Kbuf;
        Kbuf.reserve(16);

        while (!D.empty()) {

            auto pulled = D.Pull();
            Si.clear();
            Si.reserve(pulled.first.size());
            for (auto& kv : pulled.first) Si.push_back(kv.first);
            Weight Bi = pulled.second;

                        if (Si.empty()) break;
            BMSSPResult sub = run(G, l - 1, Bi, Si, state, k, t);
            current_Bp = std::min(current_Bp, sub.B_prime);



            for (auto u : sub.U) {
                if (Uset.insert(u).second) res.U.push_back(u);
                for (const auto& e : G.get_outgoing_edges(u)) {
                    const Vertex v = e.destination();
                    const Weight alt = state.get(u.id()) + e.weight();
                    const Weight dv = state.get(v.id());
                    if (alt < B && alt <= dv) {
                        const bool better = alt < dv;
                        if (better || alt == dv) {
                            if (better) state.set(v.id(), alt);
                            state.set_pred(v.id(), u.id());
                        }
                        D.Insert(v, alt);
                    } else if (alt >= current_Bp && alt < Bi) {
                        Kbuf.clear();
                        Kbuf.emplace_back(v, alt);
                        D.BatchPrepend(Kbuf);
                    }
                }
            }

                        if (Uset.size() > k * ((std::size_t)1 << (l * t))) break;
        }
        for (auto w : W) {
            if (Uset.insert(w).second) res.U.push_back(w);
        }
        res.B_prime = current_Bp;


        return res;
    }
};

} // namespace sssp

#endif // SSSP_BMSSP_HPP

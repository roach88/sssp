#ifndef SSSP_FIND_PIVOTS_HPP
#define SSSP_FIND_PIVOTS_HPP

#include "sssp/graph.hpp"
#include "sssp/types.hpp"
#include "sssp/vertex.hpp"
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <vector>
#include <algorithm>
#ifdef SSSP_PROFILE
#include "sssp/profiling.hpp"
#endif

namespace sssp {

/**
 * @brief FindPivots procedure for frontier reduction (Algorithm 1 from paper)
 * 
 * This procedure reduces the set S of "frontier" vertices to a smaller
 * set of "pivots" P, minimizing the number of useful vertices for recursive calls.
 * 
 * The procedure performs k steps of relaxation (similar to Bellman-Ford) and
 * constructs a directed forest to identify pivots.
 */
class FindPivots {
public:
    /**
     * @brief Result structure for FindPivots procedure
     */
    struct Result {
        std::unordered_set<Vertex> P;  // Set of pivots P ⊆ S, |P| ≤ |W|/k
        std::unordered_set<Vertex> W;  // Set of vertices W ⊆ Ũ which are complete
        
        Result() = default;
    };
    
    /**
     * @brief State for tracking distance estimates and predecessors
     */
    struct VertexState {
        Weight distance;
        Vertex predecessor;
        bool has_predecessor;
        bool in_W;
        
        VertexState() : distance(std::numeric_limits<Weight>::infinity()),
                        predecessor(0),
                        has_predecessor(false),
                        in_W(false) {}
    };
    
    /**
     * @brief Execute FindPivots procedure
     * 
     * @param graph The graph to operate on
     * @param B Upper bound on distances
     * @param S Set of frontier vertices
     * @param k Number of relaxation steps
     * @param d_hat Current distance estimates (global state)
     * @return Result containing pivots P and complete vertices W
     * 
     * Time Complexity: O(min{k²|S|, k|Ũ|})
     */
     static Result execute(const Graph& graph,
                           Weight B,
                           const std::unordered_set<Vertex>& S,
                           std::size_t k,
                           DistState& global) {
#ifdef SSSP_PROFILE
        ScopeTimer timer(&prof().findpivots_ns);
#endif
        Result result;
        
        // Step 1: Initialize W ← S and W₀ ← S
        result.W = S;
        std::unordered_set<Vertex> W_prev = S;
        
        // Track local state for vertices
        std::unordered_map<Vertex, VertexState> local;
        
        // Initialize distances for vertices in S
        for (const auto& v : S) {
            local[v].distance = global.get(v.id());
            local[v].in_W = true;
        }
        
        // Step 2: Perform k steps of relaxation
        for (std::size_t step = 0; step < k; ++step) {
            std::unordered_set<Vertex> W_current;
            
            // Relax edges from vertices in W_{i-1}
            for (const auto& u : W_prev) {
                if (!graph.has_vertex(u)) continue;
                for (const auto& edge : graph.get_outgoing_edges(u)) {
                    Vertex v = edge.destination();
                    Weight new_dist = local[u].distance + edge.weight();
                    if (new_dist < B) {
                        bool needs_update = false;
                        if (local.find(v) == local.end()) needs_update = true;
                        else if (new_dist < local[v].distance) needs_update = true;
                        if (needs_update) {
                            local[v].distance = new_dist;
                            local[v].predecessor = u;
                            local[v].has_predecessor = true;
                            if (!local[v].in_W) {
                                W_current.insert(v);
                                local[v].in_W = true;
                            }
                        }
                    }
                }
            }
            
            // Add W_current to W
            for (const auto& v : W_current) {
                result.W.insert(v);
            }
            
            // Step 3: Check if |W| exceeds k|S|
            if (result.W.size() > k * S.size()) {
                // Return immediately with P = S
                result.P = S;
                return result;
            }
            
            // Update W_prev for next iteration
            W_prev = W_current;
            
            // Early termination if no new vertices were added
            if (W_current.empty()) {
                break;
            }
        }
        
        // Step 4: Construct directed forest F from relaxed edges
        std::unordered_map<Vertex, std::vector<Vertex>> forest;  // Parent -> children
        std::unordered_map<Vertex, Vertex> parent;  // Child -> parent
        
        for (const auto& [v, vstate] : local) {
            if (vstate.has_predecessor && vstate.in_W) {
                Vertex pred = vstate.predecessor;
                forest[pred].push_back(v);
                parent[v] = pred;
            }
        }
        
        // Step 5: Identify pivots - roots of trees with at least k vertices
        std::unordered_map<Vertex, std::size_t> tree_sizes;
        
        // Find roots (vertices with no parent in W)
        std::unordered_set<Vertex> roots;
        for (const auto& v : result.W) {
            if (parent.find(v) == parent.end()) {
                roots.insert(v);
            }
        }
        
        // Count tree sizes using DFS
        for (const auto& root : roots) {
            std::size_t tree_size = count_tree_size(root, forest);
            tree_sizes[root] = tree_size;
            
            // Add to pivots if tree has at least k vertices
            if (tree_size >= k) {
                result.P.insert(root);
            }
        }
        
        // If no pivots were found, use original S
        if (result.P.empty()) {
            result.P = S;
        }
        
        // Update global distance estimates for vertices in W
        for (const auto& [v, vstate] : local) {
            if (vstate.in_W) {
                if (vstate.distance < global.get(v.id())) {
                    global.set(v.id(), vstate.distance);
                }
            }
        }
        
        return result;
    }
    
private:
    /**
     * @brief Count the size of a tree rooted at given vertex
     * 
     * @param root Root of the tree
     * @param forest Forest structure (parent -> children mapping)
     * @return Size of the tree
     */
    static std::size_t count_tree_size(const Vertex& root,
                                        const std::unordered_map<Vertex, std::vector<Vertex>>& forest) {
        std::size_t size = 1;  // Count the root itself
        
        auto it = forest.find(root);
        if (it != forest.end()) {
            for (const auto& child : it->second) {
                size += count_tree_size(child, forest);
            }
        }
        
        return size;
    }
    
    /**
     * @brief Alternative implementation using BFS for tree size counting
     * (Can be used if recursion depth becomes an issue)
     */
    static std::size_t count_tree_size_bfs(const Vertex& root,
                                            const std::unordered_map<Vertex, std::vector<Vertex>>& forest) {
        std::size_t size = 0;
        std::queue<Vertex> q;
        q.push(root);
        
        while (!q.empty()) {
            Vertex v = q.front();
            q.pop();
            size++;
            
            auto it = forest.find(v);
            if (it != forest.end()) {
                for (const auto& child : it->second) {
                    q.push(child);
                }
            }
        }
        
        return size;
    }
};

} // namespace sssp

#endif // SSSP_FIND_PIVOTS_HPP
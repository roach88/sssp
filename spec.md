# Development Specification Document: Custom SSSP Graph Algorithm Library

Project Title: Advanced Directed SSSP Algorithm Library
Version: 1.0
Date: October 26, 2023
Author: [Developer Name/Team]
Purpose: To implement a custom library for solving the Single-Source Shortest Paths problem on directed graphs, leveraging the novel O(m log^(2/3) n) deterministic algorithm. This library aims to provide faster context retrieval and reduced latency for applications requiring SSSP calculations, such as GraphRAG databases.

---

1. Algorithm Overview
   The core of this library will be an implementation of the BMSSP (Bounded Multi-Source Shortest Path) procedure, which operates as a recursive divide-and-conquer algorithm. It merges ideas from both Dijkstra's algorithm (through its BaseCase) and Bellman-Ford algorithm (via the FindPivots procedure).
   The algorithm's performance relies on strategically reducing the "frontier" of vertices needing consideration, thereby avoiding the Θ(n log n) sorting bottleneck inherent in traditional Dijkstra implementations.
   Key Parameters: The algorithm utilizes two main parameters:
   • k := ⌊log^(1/3)(n)⌋
   • t := ⌊log^(2/3)(n)⌋

---

2. Graph Requirements and Pre-processing
   2.1 Graph Representation
   • The library will handle directed graphs G = (V, E).
   • Edges must have real non-negative weights w: E → R≥0.
   • The algorithm operates within the comparison-addition model, meaning only comparisons and additions on edge weights are allowed, each consuming unit time.
   2.2 Constant-Degree Graph Transformation
   • Requirement: The algorithm assumes the input graph has constant in-degrees and out-degrees.
   • Transformation Process (if needed): 1. For each original vertex v in graph G, substitute it with a cycle of strongly connected, zero-weight vertices in the transformed graph G'. 2. For every incoming or outgoing neighbor w of v in G, create a specific vertex xvw on this cycle in G'. 3. For every original edge (u, v) in G with weight wuv, add a new directed edge from xuv to xvu in G' with the same weight wuv.
   • Outcome: This transformation preserves shortest paths and results in a graph G' with O(m) vertices and O(m) edges, where each vertex has an in-degree and out-degree of at most 2.
   2.3 Handling Ties (Total Order of Paths)
   • The algorithm's simplified presentation assumes all paths have different lengths (Assumption 2.1).
   • Implementation Detail: To handle real-world scenarios where ties in path lengths can occur, a total order for paths must be established. This involves representing each path as a tuple: ⟨length*l, num_vertices*α, vα, vα−1, ..., v1⟩ (note: vertices are in reversed order in the tuple).
   • Comparison Logic: Paths are sorted lexicographically: first by length l, then by α (number of vertices), and finally by the reversed sequence of vertices vα to v1. This allows O(1) time comparisons by only comparing endpoints u and v when l and α are tied.

---

3. Core Algorithmic Components
   3.1 (Algorithm 3) - The Main Recursive Procedure
   • Purpose: To find true distances to vertices v with d(v) < B where the shortest path visits some complete vertex in set S.
   • Input Parameters:
   ◦ l: Integer representing the current recursion level, l ∈ [0, ⌈(log n)/t⌉].
   ◦ B: An upper bound on distances to consider.
   ◦ S: A set of complete vertices from which paths are sought, |S| ≤ 2^(lt).
   • Output:
   ◦ B': A new boundary B' ≤ B.
   ◦ U: A set of complete vertices U whose shortest paths are found to be less than B' and visit some vertex in S.
   • Initial Call (Top Level): BMSSP(l = ⌈(log n)/t⌉, S = {s}, B = ∞).
   • Flow: 1. Base Case (l = 0): Calls BaseCase(B, S). 2. Recursive Step (l > 0):
   ▪ Calls FindPivots(B, S) to obtain a pivot set P and a set W of complete vertices.
   ▪ Initializes the specialized data structure D (from Lemma 3.3) with M := 2^((l-1)t).
   ▪ Inserts elements of P into D.
   ▪ Enters a loop that repeatedly:
   • Pulls a subset Si of M smallest-value keys from D and a boundary Bi.
   • Recursively calls BMSSP(l - 1, Bi, Si).
   • Relaxes edges (u, v) for all u ∈ Ui (where Ui is returned by the recursive call).
   • Inserts ⟨v, d̂[u] + wuv⟩ into D if d̂[u] + wuv ∈ [Bi, B).
   • Records ⟨v, d̂[u] + wuv⟩ in a set K if d̂[u] + wuv ∈ [B', Bi).
   • Batch Prepend K and relevant Si elements back into D.
   • Updates U.
   • Checks termination conditions: D is empty (successful) or |U| > k \* 2^(lt) (partial execution due to large workload).
   ▪ Final update of U with elements from W.
   3.2 (Algorithm 1) - Frontier Reduction
   • Purpose: To shrink the set S of "frontier" vertices to a smaller set of "pivots" P, reducing the number of useful vertices for recursive calls.
   • Input Parameters:
   ◦ B: An upper bound.
   ◦ S: A set of vertices.
   • Output:
   ◦ P: A set of pivots P ⊆ S, |P| ≤ |W|/k.
   ◦ W: A set of vertices W ⊆ Ũ which are complete after the procedure, |W| = O(k|S|).
   • Process: 1. Initializes W ← S and W0 ← S. 2. Performs k steps of relaxation (similar to Bellman-Ford) from vertices in W(i-1). 3. If |W| exceeds k|S| at any point, P is set to S and P, W are returned immediately. 4. Otherwise, constructs a directed forest F from relaxed edges. 5. Identifies pivots P: roots of trees in F that have at least k vertices.
   • Time Complexity: O(min{k^2|S|, k|Ũ|}).
   3.3 (Algorithm 2) - Lowest Level of Recursion
   • Purpose: To act as a "mini Dijkstra's algorithm" when l = 0.
   • Input Parameters:
   ◦ B: An upper bound.
   ◦ S: A singleton set {x} where x is complete.
   • Output:
   ◦ B': A boundary B' ≤ B.
   ◦ U: A set of complete vertices.
   • Process: 1. Initializes U0 ← S and a binary heap H with ⟨x, d̂[x]⟩. 2. While H is not empty and |U0| < k + 1:
   ▪ Extracts the minimum ⟨u, d̂[u]⟩ from H.
   ▪ Adds u to U0.
   ▪ Relaxes outgoing edges (u, v): updates d̂[v] and inserts/decreases key for v in H if d̂[u] + wuv < B. 3. Returns B' and U based on whether k+1 vertices were found.
   • Time Complexity: For l=0, it's C|U| log k.

---

4. Specialized Data Structures
   4.1 Main Data Structure (Lemma 3.3)
   • Purpose: An adaptive block-based linked list data structure for efficiently managing vertices on the frontier and partitioning problems.
   • Operations (Key/Value pairs: vertex/distance estimates):
   ◦ Initialize(M, B): Initializes D0 (empty) and D1 (single empty block with upper bound B). Sets parameter M. M is set to 2^((l-1)t) in BMSSP.
   ◦ Insert(key, value):
   ▪ Inserts or updates a pair. If key exists, only update if new value is smaller.
   ▪ Locates appropriate block using binary search tree on block upper bounds.
   ▪ Adds to linked list. If block exceeds size limit M, triggers a Split operation.
   ▪ Amortized Time: O(max{1, log(N/M)}). In BMSSP, this translates to O(t) time.
   ◦ Batch Prepend(L):
   ▪ Inserts a list L of key/value pairs where each new value is smaller than any value currently in the data structure.
   ▪ If |L| ≤ M, creates a new block for L at the beginning of D0.
   ▪ Otherwise, creates O(L/M) new blocks in D0.
   ▪ Handles multiple pairs with same key by keeping the smallest value.
   ▪ Amortized Time: O(|L| · max{1, log(|L|/M)}). In BMSSP, this translates to O(log k) per vertex in K.
   ◦ Pull():
   ▪ Returns a subset S' of M keys with the smallest values and a separating upper bound x.
   ▪ Collects a sufficient prefix of blocks from D0 and D1.
   ▪ Amortized Time: O(|S'|). In BMSSP, each pulled term's time can be amortized to insertion/batch prepend.
   • Internal Structure of D:
   ◦ Maintains two sequences of blocks: D0 (for batch prepends) and D1 (for inserts).
   ◦ Each block is a linked list of key/value pairs, containing at most M pairs.
   ◦ Blocks are maintained in sorted order by value.
   ◦ For D1 blocks, an upper bound for its elements is maintained.
   ◦ A self-balancing binary search tree (e.g., Red-Black Tree) dynamically maintains these upper bounds for D1 blocks.
   ◦ Split Operation: When a D1 block exceeds M elements, it's split into two new blocks by finding the median element in O(M) time. This ensures Θ(M) elements per block in D1 and O(N/M) total blocks in D1.
   4.2 Binary Heap
   • Used specifically in the BaseCase (Algorithm 2) for its Dijkstra-like operations of ExtractMin, Insert, and DecreaseKey.

---

5. Global Data and State Management
   • d̂[u] (Distance Estimates): A global variable storing the current sound estimate of the shortest path d(u) from source s to u (d̂[u] ≥ d(u)). Initially d̂[s] = 0 and d̂[v] = ∞ for v ≠ s. Updates are non-increasing: d̂[v] ← d̂[u] + wuv if d̂[u] + wuv is no greater than the old d̂[v].
   • Pred[v] (Predecessor): Records the predecessor vertex for each v to reconstruct the shortest path tree based on current d̂[·] values. Set to u when relaxing (u,v) and d̂[v] is updated.
   • Completeness Status: A vertex x is marked complete when d̂[x] = d(x), otherwise incomplete. A complete vertex remains complete.

---

6. Correctness Assumptions and Guarantees
   • Disjointness of Ui sets: The sets Ui returned by recursive calls are guaranteed to be disjoint. This is crucial for correctness and complexity analysis.
   • Edge Relaxation Condition: The condition d̂[u] + wuv ≤ d̂[v] (including equality) for relaxing an edge (u, v) is critical to allow an edge relaxed on a lower level to be re-used and potentially re-relaxed on upper levels of recursion.
   • Completeness Propagation: When BMSSP returns, the set U contains all required vertices (based on B') and all these vertices are complete.

---

7. Performance and Complexity
   • Overall Time Complexity: The deterministic algorithm achieves O(m log^(2/3) n) time.
   • Breakdown of Operations:
   ◦ FindPivots: O(nk) across all nodes at one depth of the recursion tree.
   ◦ D Data Structure Operations:
   ▪ Insertions of P into D: O((n log n)/k) total across all depths.
   ▪ Batch Prepend of K and Si into D: O(n · log k · (log n)/t) total.
   ▪ Direct Insertions via edge relaxations (Line 18 of Algo 3): O(m(log k + t)) = O(m log^(2/3) n) total. Each edge (u,v) only leads v to be directly inserted once.
   ▪ Insertions via K (Batch Prepend, Line 20 of Algo 3): O(m log k · (log n)/t) total.
   ◦ All these components contribute to the overall O(m log^(2/3) n) time bound when k = ⌊log^(1/3)(n)⌋ and t = ⌊log^(2/3)(n)⌋.

---

8. API Design (Example)
   A high-level function in the library might look like this:

# include "Graph.h" // Custom Graph representation

# include <map> // For distances and predecessors

/\*\*

- @brief Solves the Single-Source Shortest Paths problem using the custom O(m log^(2/3) n) algorithm.
- @param graph A pointer to the Graph object (must be directed with non-negative real weights).
- @param source The source vertex from which to compute shortest paths.
- @return A std::map mapping vertices to their shortest distances from the source,
-         and another map for predecessors to reconstruct paths.
-         Returns empty maps if the source is invalid or graph is null.

  _/
  std::pair<std::map<Vertex, double>, std::map<Vertex, Vertex>>
  solveSSSP(Graph_ graph, Vertex source);

// Internally, this function would orchestrate calls to BMSSP, FindPivots, BaseCase,
// and manage the D data structure, as well as handle graph transformation if needed.

---

9. Development Considerations
   • Memory Management: The recursive nature and large data structures (especially D) will require careful memory management to prevent leaks or excessive consumption, particularly for large graphs. Consider explicit stack management or iterative approaches if recursion depth becomes an issue for very large n [Conversation History].
   • Debugging: Given the complexity of the algorithm, robust logging, assertion checks, and visualization tools (if possible) will be invaluable for verifying correctness, especially for the FindPivots logic and D data structure operations [Conversation History].
   • Floating-Point Precision: When dealing with real-valued edge weights, be mindful of floating-point precision issues that might affect comparisons and tie-breaking. The Total Order of Paths approach helps mitigate some of these by providing a deterministic tie-breaking mechanism.
   • Constants Hiding in Big-O: While the asymptotic complexity is impressive, the constant factors (denoted by C in Lemma 3.12) can impact practical performance. Careful optimization of inner loops and data structure operations will be necessary.
   • Test Cases: Develop comprehensive test suites including sparse and dense graphs, various edge weight distributions, and graphs requiring the constant-degree transformation. Special attention should be paid to edge cases like disconnected components (vertices unreachable from source).

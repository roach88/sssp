#ifndef SSSP_GRAPH_HPP
#define SSSP_GRAPH_HPP

#include "sssp/types.hpp"
#include "sssp/vertex.hpp"
#include "sssp/edge.hpp"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <algorithm>
#include <stdexcept>

namespace sssp {

/**
 * @brief Represents a directed graph with non-negative edge weights
 * 
 * The Graph class uses an adjacency list representation for efficient
 * traversal and is optimized for the SSSP algorithm. It maintains both
 * outgoing and incoming edges for each vertex to support various graph
 * operations required by the algorithm.
 */
class Graph {
public:
    // Type aliases for internal data structures
    using EdgeList = std::vector<Edge>;
    using AdjacencyList = std::unordered_map<Vertex, EdgeList>;
    using VertexSet = std::unordered_set<Vertex>;
    
    // Constructor
    Graph() noexcept : num_vertices_(0), num_edges_(0), next_edge_id_(0) {}
    
    // Copy and move constructors
    Graph(const Graph&) = default;
    Graph(Graph&&) noexcept = default;
    
    // Copy and move assignment operators
    Graph& operator=(const Graph&) = default;
    Graph& operator=(Graph&&) noexcept = default;
    
    // Destructor
    ~Graph() = default;
    
    // Vertex operations
    void add_vertex(const Vertex& v) {
        if (!v.is_valid()) {
            throw std::invalid_argument("Cannot add invalid vertex to graph");
        }
        
        if (vertices_.find(v) == vertices_.end()) {
            vertices_.insert(v);
            outgoing_edges_[v] = EdgeList();
            incoming_edges_[v] = EdgeList();
            num_vertices_++;
        }
    }
    
    void add_vertex(VertexId id) {
        add_vertex(Vertex(id));
    }
    
    [[nodiscard]] bool has_vertex(const Vertex& v) const noexcept {
        return vertices_.find(v) != vertices_.end();
    }
    
    [[nodiscard]] bool has_vertex(VertexId id) const noexcept {
        return has_vertex(Vertex(id));
    }
    
    // Edge operations
    void add_edge(const Edge& e) {
        // Ensure both vertices exist
        add_vertex(e.source());
        add_vertex(e.destination());
        
        // Create edge with unique ID
        Edge edge_with_id(next_edge_id_++, e.source(), e.destination(), e.weight());
        
        // Add to adjacency lists
        outgoing_edges_[e.source()].push_back(edge_with_id);
        incoming_edges_[e.destination()].push_back(edge_with_id);
        edges_.push_back(edge_with_id);
        num_edges_++;
    }
    
    void add_edge(const Vertex& source, const Vertex& destination, Weight weight) {
        add_edge(Edge(source, destination, weight));
    }
    
    void add_edge(VertexId source_id, VertexId destination_id, Weight weight) {
        add_edge(Vertex(source_id), Vertex(destination_id), weight);
    }
    
    // Adjacency list retrieval
    [[nodiscard]] const EdgeList& get_outgoing_edges(const Vertex& v) const {
        auto it = outgoing_edges_.find(v);
        if (it == outgoing_edges_.end()) {
            static const EdgeList empty_list;
            return empty_list;
        }
        return it->second;
    }
    
    [[nodiscard]] const EdgeList& get_outgoing_edges(VertexId id) const {
        return get_outgoing_edges(Vertex(id));
    }
    
    [[nodiscard]] const EdgeList& get_incoming_edges(const Vertex& v) const {
        auto it = incoming_edges_.find(v);
        if (it == incoming_edges_.end()) {
            static const EdgeList empty_list;
            return empty_list;
        }
        return it->second;
    }
    
    [[nodiscard]] const EdgeList& get_incoming_edges(VertexId id) const {
        return get_incoming_edges(Vertex(id));
    }
    
    // Graph properties
    [[nodiscard]] std::size_t num_vertices() const noexcept { return num_vertices_; }
    [[nodiscard]] std::size_t num_edges() const noexcept { return num_edges_; }
    [[nodiscard]] bool empty() const noexcept { return num_vertices_ == 0; }
    
    [[nodiscard]] const VertexSet& vertices() const noexcept { return vertices_; }
    [[nodiscard]] const std::vector<Edge>& edges() const noexcept { return edges_; }
    
    // Degree queries
    [[nodiscard]] std::size_t out_degree(const Vertex& v) const {
        auto it = outgoing_edges_.find(v);
        return (it != outgoing_edges_.end()) ? it->second.size() : 0;
    }
    
    [[nodiscard]] std::size_t in_degree(const Vertex& v) const {
        auto it = incoming_edges_.find(v);
        return (it != incoming_edges_.end()) ? it->second.size() : 0;
    }
    
    [[nodiscard]] std::size_t degree(const Vertex& v) const {
        return out_degree(v) + in_degree(v);
    }
    
    // Check if graph needs constant-degree transformation
    [[nodiscard]] bool needs_constant_degree_transformation() const {
        const std::size_t MAX_DEGREE = 2;
        for (const auto& v : vertices_) {
            if (out_degree(v) > MAX_DEGREE || in_degree(v) > MAX_DEGREE) {
                return true;
            }
        }
        return false;
    }
    
    // Clear the graph
    void clear() noexcept {
        vertices_.clear();
        edges_.clear();
        outgoing_edges_.clear();
        incoming_edges_.clear();
        num_vertices_ = 0;
        num_edges_ = 0;
        next_edge_id_ = 0;
    }
    
    // Get algorithm parameters
    [[nodiscard]] std::size_t get_k() const {
        return compute_k(num_vertices_);
    }
    
    [[nodiscard]] std::size_t get_t() const {
        return compute_t(num_vertices_);
    }
    
    // Iterator support for vertices
    using vertex_iterator = VertexSet::const_iterator;
    [[nodiscard]] vertex_iterator begin() const noexcept { return vertices_.begin(); }
    [[nodiscard]] vertex_iterator end() const noexcept { return vertices_.end(); }
    
    // Iterator support for edges
    using edge_iterator = std::vector<Edge>::const_iterator;
    [[nodiscard]] edge_iterator edges_begin() const noexcept { return edges_.begin(); }
    [[nodiscard]] edge_iterator edges_end() const noexcept { return edges_.end(); }

private:
    VertexSet vertices_;                    // Set of all vertices
    std::vector<Edge> edges_;               // List of all edges
    AdjacencyList outgoing_edges_;         // Outgoing edges for each vertex
    AdjacencyList incoming_edges_;         // Incoming edges for each vertex
    std::size_t num_vertices_;             // Number of vertices
    std::size_t num_edges_;                // Number of edges
    EdgeId next_edge_id_;                  // Next available edge ID
};

} // namespace sssp

#endif // SSSP_GRAPH_HPP
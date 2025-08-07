#ifndef SSSP_EDGE_HPP
#define SSSP_EDGE_HPP

#include "sssp/types.hpp"
#include "sssp/vertex.hpp"
#include <stdexcept>

namespace sssp {

/**
 * @brief Represents a directed edge in the graph
 * 
 * The Edge class represents a directed edge from a source vertex to a
 * destination vertex with a non-negative weight. This class is designed
 * to be efficient for use in graph algorithms, particularly the SSSP
 * algorithm implementation.
 */
class Edge {
public:
    // Default constructor creates an invalid edge
    Edge() noexcept 
        : source_(), destination_(), weight_(INFINITE_WEIGHT), id_(0) {}
    
    // Constructor with source, destination, and weight
    Edge(const Vertex& source, const Vertex& destination, Weight weight)
        : source_(source), destination_(destination), weight_(weight), id_(0) {
        if (weight < 0) {
            throw std::invalid_argument("Edge weight must be non-negative");
        }
        if (!source.is_valid() || !destination.is_valid()) {
            throw std::invalid_argument("Edge must have valid source and destination vertices");
        }
    }
    
    // Constructor with all parameters including edge ID
    Edge(EdgeId id, const Vertex& source, const Vertex& destination, Weight weight)
        : source_(source), destination_(destination), weight_(weight), id_(id) {
        if (weight < 0) {
            throw std::invalid_argument("Edge weight must be non-negative");
        }
        if (!source.is_valid() || !destination.is_valid()) {
            throw std::invalid_argument("Edge must have valid source and destination vertices");
        }
    }
    
    // Copy and move constructors
    Edge(const Edge&) = default;
    Edge(Edge&&) noexcept = default;
    
    // Copy and move assignment operators
    Edge& operator=(const Edge&) = default;
    Edge& operator=(Edge&&) noexcept = default;
    
    // Destructor
    ~Edge() = default;
    
    // Getters
    [[nodiscard]] const Vertex& source() const noexcept { return source_; }
    [[nodiscard]] const Vertex& destination() const noexcept { return destination_; }
    [[nodiscard]] Weight weight() const noexcept { return weight_; }
    [[nodiscard]] EdgeId id() const noexcept { return id_; }
    
    [[nodiscard]] bool is_valid() const noexcept {
        return source_.is_valid() && destination_.is_valid() && weight_ >= 0;
    }
    
    // Setters (for algorithms that need to update weights)
    void set_weight(Weight new_weight) {
        if (new_weight < 0) {
            throw std::invalid_argument("Edge weight must be non-negative");
        }
        weight_ = new_weight;
    }
    
    void set_id(EdgeId new_id) noexcept {
        id_ = new_id;
    }
    
    // Comparison operators (based on weight for priority queues)
    [[nodiscard]] bool operator<(const Edge& other) const noexcept {
        return weight_ < other.weight_;
    }
    
    [[nodiscard]] bool operator>(const Edge& other) const noexcept {
        return weight_ > other.weight_;
    }
    
    [[nodiscard]] bool operator==(const Edge& other) const noexcept {
        return source_ == other.source_ && 
               destination_ == other.destination_ && 
               weight_ == other.weight_;
    }
    
    [[nodiscard]] bool operator!=(const Edge& other) const noexcept {
        return !(*this == other);
    }
    
    // Returns the "other" vertex given one endpoint
    [[nodiscard]] Vertex get_other_vertex(const Vertex& v) const {
        if (v == source_) {
            return destination_;
        } else if (v == destination_) {
            return source_;
        } else {
            throw std::invalid_argument("Vertex is not an endpoint of this edge");
        }
    }

private:
    Vertex source_;       // Source vertex of the directed edge
    Vertex destination_;  // Destination vertex of the directed edge
    Weight weight_;       // Non-negative weight of the edge
    EdgeId id_;          // Optional edge identifier
};

} // namespace sssp

#endif // SSSP_EDGE_HPP
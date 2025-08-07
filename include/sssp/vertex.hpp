#ifndef SSSP_VERTEX_HPP
#define SSSP_VERTEX_HPP

#include "sssp/types.hpp"
#include <functional>

namespace sssp {

/**
 * @brief Represents a vertex in the directed graph
 * 
 * The Vertex class is designed to be lightweight for efficient handling
 * of large graphs. It stores minimal information - just an identifier
 * and provides methods for comparison and hashing.
 */
class Vertex {
public:
    // Default constructor creates an invalid vertex
    Vertex() noexcept : id_(INVALID_VERTEX) {}
    
    // Explicit constructor with vertex ID
    explicit Vertex(VertexId id) noexcept : id_(id) {}
    
    // Copy and move constructors
    Vertex(const Vertex&) noexcept = default;
    Vertex(Vertex&&) noexcept = default;
    
    // Copy and move assignment operators
    Vertex& operator=(const Vertex&) noexcept = default;
    Vertex& operator=(Vertex&&) noexcept = default;
    
    // Destructor
    ~Vertex() = default;
    
    // Getters
    [[nodiscard]] VertexId id() const noexcept { return id_; }
    [[nodiscard]] bool is_valid() const noexcept { return id_ != INVALID_VERTEX; }
    
    // Comparison operators for ordering and equality
    [[nodiscard]] bool operator==(const Vertex& other) const noexcept {
        return id_ == other.id_;
    }
    
    [[nodiscard]] bool operator!=(const Vertex& other) const noexcept {
        return id_ != other.id_;
    }
    
    [[nodiscard]] bool operator<(const Vertex& other) const noexcept {
        return id_ < other.id_;
    }
    
    [[nodiscard]] bool operator<=(const Vertex& other) const noexcept {
        return id_ <= other.id_;
    }
    
    [[nodiscard]] bool operator>(const Vertex& other) const noexcept {
        return id_ > other.id_;
    }
    
    [[nodiscard]] bool operator>=(const Vertex& other) const noexcept {
        return id_ >= other.id_;
    }
    
    // Hash support for use in unordered containers
    struct Hash {
        std::size_t operator()(const Vertex& v) const noexcept {
            return std::hash<VertexId>{}(v.id_);
        }
    };

private:
    VertexId id_;  // Unique identifier for the vertex
};

} // namespace sssp

// Specialization of std::hash for Vertex
namespace std {
    template<>
    struct hash<sssp::Vertex> {
        std::size_t operator()(const sssp::Vertex& v) const noexcept {
            return std::hash<sssp::VertexId>{}(v.id());
        }
    };
}

#endif // SSSP_VERTEX_HPP
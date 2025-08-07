#ifndef SSSP_TYPES_HPP
#define SSSP_TYPES_HPP

#include <cstddef>
#include <cstdint>
#include <limits>

namespace sssp {

// Type definitions for vertex and edge identifiers
using VertexId = std::size_t;
using EdgeId = std::size_t;

// Type for edge weights (non-negative real numbers)
using Weight = double;

// Special values
constexpr VertexId INVALID_VERTEX = std::numeric_limits<VertexId>::max();
constexpr Weight INFINITE_WEIGHT = std::numeric_limits<Weight>::infinity();

// Algorithm parameters based on the paper
inline std::size_t compute_k(std::size_t n) {
    // k = floor(log^(1/3)(n))
    if (n <= 1) return 1;
    
    // Using change of base: log_2(n) / 3
    std::size_t log_n = 0;
    std::size_t temp = n;
    while (temp > 1) {
        temp >>= 1;
        log_n++;
    }
    
    // Approximate k = 2^(log_n / 3)
    std::size_t k = 1;
    for (std::size_t i = 0; i < log_n / 3; ++i) {
        k <<= 1;
    }
    return k > 0 ? k : 1;
}

inline std::size_t compute_t(std::size_t n) {
    // t = floor(log^(2/3)(n))
    if (n <= 1) return 1;
    
    // Using change of base: 2 * log_2(n) / 3
    std::size_t log_n = 0;
    std::size_t temp = n;
    while (temp > 1) {
        temp >>= 1;
        log_n++;
    }
    
    // Approximate t = 2^(2 * log_n / 3)
    std::size_t t = 1;
    for (std::size_t i = 0; i < (2 * log_n) / 3; ++i) {
        t <<= 1;
    }
    return t > 0 ? t : 1;
}

} // namespace sssp

#endif // SSSP_TYPES_HPP
# Advanced Directed SSSP Algorithm Library

A high-performance C++ implementation of the Single-Source Shortest Paths (SSSP) algorithm for directed graphs, achieving O(m log^(2/3) n) deterministic time complexity.

## Overview

This library implements a novel SSSP algorithm that breaks the traditional Θ(n log n) sorting bottleneck of Dijkstra's algorithm through the innovative BMSSP (Bounded Multi-Source Shortest Path) procedure. By combining ideas from both Dijkstra's and Bellman-Ford algorithms in a recursive divide-and-conquer approach, this implementation provides superior performance for large-scale graph processing applications.

### Key Features

- **O(m log^(2/3) n) Time Complexity**: Deterministic algorithm with proven theoretical bounds
- **Advanced Data Structures**: Specialized block-based linked list for efficient frontier management
- **Graph Transformation**: Automatic constant-degree transformation for arbitrary graphs
- **Cross-Platform**: CMake-based build system supporting Linux, macOS, and Windows
- **Modern C++17**: Leveraging modern language features for performance and maintainability

## Algorithm Components

### Core Procedures

1. **BMSSP (Algorithm 3)**: Main recursive divide-and-conquer procedure
2. **FindPivots (Algorithm 1)**: Frontier reduction for minimizing vertex sets
3. **BaseCase (Algorithm 2)**: Mini-Dijkstra's algorithm at recursion level 0

### Data Structures

- **Specialized Block-Based Structure**: Adaptive linked list with O(t) insertion time
- **Binary Heap**: Efficient support for Dijkstra-like operations
- **Self-Balancing BST**: Dynamic management of block upper bounds

## Requirements

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.14 or higher
- (Optional) Google Test for running unit tests

## Building

```bash
# Clone the repository
git clone <repository-url>
cd sssp

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the library
make -j$(nproc)

# Run tests (if available)
make test
```

### Build Options

- `CMAKE_BUILD_TYPE`: Set to `Debug` or `Release` (default: Release)
- `BUILD_TESTS`: Enable building tests (default: ON)
- `BUILD_EXAMPLES`: Enable building example programs (default: ON)

## Usage

```cpp
#include "sssp/sssp.hpp"

// Create and populate your graph
Graph graph;
// ... add vertices and edges ...

// Solve SSSP from source vertex
Vertex source = 0;
auto [distances, predecessors] = solveSSSP(&graph, source);

// Access shortest distance to any vertex
Vertex target = 42;
double distance = distances[target];

// Reconstruct shortest path
auto path = reconstructPath(predecessors, source, target);
```

## API Reference

### Core Functions

```cpp
std::pair<std::map<Vertex, double>, std::map<Vertex, Vertex>>
solveSSSP(Graph* graph, Vertex source);
```

Solves the Single-Source Shortest Paths problem.

**Parameters:**
- `graph`: Pointer to a directed graph with non-negative edge weights
- `source`: Source vertex for path computation

**Returns:**
- First: Map of vertices to their shortest distances from source
- Second: Predecessor map for path reconstruction

## Performance

The algorithm achieves O(m log^(2/3) n) time complexity where:
- m = number of edges
- n = number of vertices
- k = ⌊log^(1/3)(n)⌋
- t = ⌊log^(2/3)(n)⌋

### Benchmarks

Performance comparisons with standard implementations:

| Graph Size | Edges | SSSP Library | Dijkstra | Speedup |
|------------|-------|--------------|----------|---------|
| 10K nodes  | 100K  | TBD          | TBD      | TBD     |
| 100K nodes | 1M    | TBD          | TBD      | TBD     |
| 1M nodes   | 10M   | TBD          | TBD      | TBD     |

## Development

### Project Structure

```
sssp/
├── include/          # Header files
│   └── sssp/        # Public API headers
├── src/             # Implementation files
├── tests/           # Unit and integration tests
├── docs/            # Documentation
├── CMakeLists.txt   # Build configuration
└── README.md        # This file
```

### Contributing

Please see [CONTRIBUTING.md](docs/CONTRIBUTING.md) for development guidelines.

### Testing

Run the test suite:

```bash
cd build
ctest --verbose
```

## Theory

This implementation is based on the theoretical work presented in "Breaking the Sorting Barrier" which introduces a deterministic O(m log^(2/3) n) algorithm for SSSP on directed graphs. The algorithm operates within the comparison-addition model and handles real non-negative edge weights.

## License

[License information to be added]

## Authors

[Author information to be added]

## References

- Original Paper: "Breaking the Sorting Barrier" [Citation to be added]
- [Additional references]

## Status

This project is currently under active development. See the [task list](.taskmaster/tasks/) for current progress.
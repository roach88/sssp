# Advanced Directed SSSP Algorithm Library

A high-performance C++ implementation of the Single-Source Shortest Paths (SSSP) algorithm for directed graphs, achieving O(m log^(2/3) n) deterministic time complexity.

## Overview

This library implements a novel SSSP algorithm that breaks the traditional Θ(n log n) sorting bottleneck of Dijkstra's algorithm through the innovative BMSSP (Bounded Multi-Source Shortest Path) procedure. By combining ideas from both Dijkstra's and Bellman-Ford algorithms in a recursive divide-and-conquer approach, this implementation provides superior performance for large-scale graph processing applications.

### Key Features

- **O(m log^(2/3) n) Time Complexity**: Deterministic algorithm with proven theoretical bounds
- **Advanced Data Structures**: Specialized block-based linked list for efficient frontier management
- **Graph Transformation**: Automatic constant-degree transformation for arbitrary graphs
- **Profiling Built-in**: Optional timers for BaseCase, FindPivots, and BMSSP with SSSP_PROFILE
- **Modern C++17**: Leveraging modern language features for performance and maintainability

## Algorithm Components

### Core Procedures

1. **BMSSP (Algorithm 3)**: Main recursive divide-and-conquer procedure
2. **FindPivots (Algorithm 1)**: Frontier reduction for minimizing vertex sets
3. **BaseCase (Algorithm 2)**: Mini-Dijkstra's algorithm at recursion level 0

### Data Structures

- **Specialized Block-Based Structure (D)**: Adaptive linked-list blocks with O(t) insertion time and Pull/BatchPrepend operations
- **Binary Heap**: Efficient support for Dijkstra-like operations in BaseCase
- **Self-Balancing BST**: Dynamic management of block upper bounds for D1

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
cmake --build . -j

# Run tests
ctest --output-on-failure
```

### Profiling build

```bash
cmake -S . -B build-prof -DCMAKE_BUILD_TYPE=Release -DSSSP_PROFILE=ON -DBUILD_BENCHMARKS=ON
cmake --build build-prof -j
ctest --test-dir build-prof --output-on-failure
./build-prof/bench_sssp
```

### Build Options

- `CMAKE_BUILD_TYPE`: Set to `Debug` or `Release` (default: Release)
- `BUILD_TESTS`: Enable building tests (default: ON)
- `BUILD_EXAMPLES`: Enable building example programs (default: ON)

## Usage

```cpp
#include "sssp/api.hpp"
#include "sssp/path.hpp"

using namespace sssp;

Graph G;
// ... add vertices and edges ...

auto [dist, pred] = solveSSSP(G, Vertex(0));

auto d = get_distance(dist, Vertex(42));
auto path = reconstruct_path(Vertex(42), pred, Vertex(0));
```

If you need direct access to internal state for advanced workflows, use DistState:

```cpp
#include "sssp/types.hpp"
#include "sssp/bmssp.hpp"

DistState state; state.init(G.num_vertices());
state.set(0, 0.0);
std::vector<Vertex> S = {Vertex(0)};
auto res = BMSSP::run(G, /*l*/ 2, std::numeric_limits<Weight>::infinity(), S, state, G.get_k(), G.get_t());
```

## API Reference

### Core Functions

```cpp
std::pair<std::unordered_map<Vertex, Weight>, std::unordered_map<Vertex, Vertex>>
solveSSSP(const Graph& G, const Vertex& source);

Weight get_distance(const std::unordered_map<Vertex, Weight>& distances, Vertex v);
std::vector<Weight> get_distances(const std::unordered_map<Vertex, Weight>& distances, const std::vector<Vertex>& vs);
```

For internal workflows, DistState-based helpers are available:

```cpp
Weight get_distance(const DistState& state, Vertex v);
std::vector<Weight> get_distances(const DistState& state, const std::vector<Vertex>& vs);
```

## Performance

The algorithm achieves O(m log^(2/3) n) time complexity where:

- m = number of edges
- n = number of vertices
- k = ⌊log^(1/3)(n)⌋
- t = ⌊log^(2/3)(n)⌋

### Benchmarks

Build with profiling flags and run the included benchmark:

```bash
cmake -S . -B build-prof -DCMAKE_BUILD_TYPE=Release -DSSSP_PROFILE=ON -DBUILD_BENCHMARKS=ON
cmake --build build-prof -j
./build-prof/bench_sssp
```

Sample output:

```
Ran 5 SSSP runs on n=1000 m=5000 in X ms
dist[0]=0
SSSP profile (ms): basecase=0.01 findpivots=0.02 bmssp=0.07
```

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

## Authors

## References

- Original Paper: "Breaking the Sorting Barrier" [Citation to be added]
- [Additional references]

## Status

This project is currently under active development. See the [task list](.taskmaster/tasks/) for current progress.

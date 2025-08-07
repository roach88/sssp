#include "sssp/binary_heap.hpp"
#include <iostream>
#include <cassert>
#include <random>
#include <chrono>
#include <algorithm>
#include <queue>

using namespace sssp;

void test_initialization() {
    std::cout << "Test 1: Initialization\n";
    
    BinaryHeap heap;
    assert(heap.empty());
    assert(heap.size() == 0);
    
    BinaryHeap heap2(100);
    assert(heap2.empty());
    assert(heap2.size() == 0);
    assert(heap2.capacity() >= 100);
    
    std::cout << "  ✓ Initialization successful\n\n";
}

void test_insert_single() {
    std::cout << "Test 2: Single Insert Operations\n";
    
    BinaryHeap heap;
    
    // Insert first element
    assert(heap.insert(Vertex(1), 10.0));
    assert(!heap.empty());
    assert(heap.size() == 1);
    assert(heap.contains(Vertex(1)));
    assert(heap.get_distance(Vertex(1)) == 10.0);
    
    // Peek at minimum
    auto [min_v, min_d] = heap.peek_min();
    assert(min_v.id() == 1);
    assert(min_d == 10.0);
    
    // Insert more elements
    assert(heap.insert(Vertex(2), 5.0));
    assert(heap.insert(Vertex(3), 15.0));
    assert(heap.size() == 3);
    
    // Check new minimum
    auto [new_min_v, new_min_d] = heap.peek_min();
    assert(new_min_v.id() == 2);
    assert(new_min_d == 5.0);
    
    // Try to insert duplicate with larger distance (should not update)
    assert(!heap.insert(Vertex(2), 20.0));
    assert(heap.get_distance(Vertex(2)) == 5.0);
    
    // Insert duplicate with smaller distance (should update)
    assert(heap.insert(Vertex(3), 3.0));
    assert(heap.get_distance(Vertex(3)) == 3.0);
    
    // Check heap validity
    assert(heap.is_valid());
    
    std::cout << "  ✓ Insert operations work correctly\n\n";
}

void test_extract_min() {
    std::cout << "Test 3: ExtractMin Operations\n";
    
    BinaryHeap heap;
    
    // Insert elements in random order
    heap.insert(Vertex(5), 25.0);
    heap.insert(Vertex(2), 10.0);
    heap.insert(Vertex(8), 40.0);
    heap.insert(Vertex(1), 5.0);
    heap.insert(Vertex(3), 15.0);
    heap.insert(Vertex(7), 35.0);
    heap.insert(Vertex(4), 20.0);
    heap.insert(Vertex(6), 30.0);
    
    assert(heap.size() == 8);
    assert(heap.is_valid());
    
    // Extract elements - should come out in sorted order
    std::vector<Weight> extracted_distances;
    while (!heap.empty()) {
        auto [vertex, distance] = heap.extract_min();
        extracted_distances.push_back(distance);
        assert(heap.is_valid());
    }
    
    // Verify sorted order
    std::vector<Weight> expected = {5.0, 10.0, 15.0, 20.0, 25.0, 30.0, 35.0, 40.0};
    assert(extracted_distances == expected);
    
    std::cout << "  ✓ ExtractMin maintains heap property\n\n";
}

void test_decrease_key() {
    std::cout << "Test 4: DecreaseKey Operations\n";
    
    BinaryHeap heap;
    
    // Insert initial elements
    heap.insert(Vertex(1), 50.0);
    heap.insert(Vertex(2), 30.0);
    heap.insert(Vertex(3), 40.0);
    heap.insert(Vertex(4), 20.0);
    heap.insert(Vertex(5), 60.0);
    
    assert(heap.peek_min().first.id() == 4);  // Vertex 4 has min distance 20
    
    // Decrease key of vertex 3 to become new minimum
    assert(heap.decrease_key(Vertex(3), 10.0));
    assert(heap.peek_min().first.id() == 3);
    assert(heap.get_distance(Vertex(3)) == 10.0);
    assert(heap.is_valid());
    
    // Try to decrease key of non-existent vertex
    assert(!heap.decrease_key(Vertex(10), 5.0));
    
    // Try to "decrease" key to larger value (should fail)
    assert(!heap.decrease_key(Vertex(3), 15.0));
    assert(heap.get_distance(Vertex(3)) == 10.0);  // Unchanged
    
    // Multiple decreases
    assert(heap.decrease_key(Vertex(5), 8.0));
    assert(heap.peek_min().first.id() == 5);
    
    assert(heap.decrease_key(Vertex(1), 7.0));
    assert(heap.peek_min().first.id() == 1);
    
    assert(heap.is_valid());
    
    std::cout << "  ✓ DecreaseKey operations work correctly\n\n";
}

void test_build_heap() {
    std::cout << "Test 5: BuildHeap Operation\n";
    
    BinaryHeap heap;
    
    // Create a large set of entries
    std::vector<std::pair<Vertex, Weight>> entries;
    for (int i = 0; i < 100; ++i) {
        entries.emplace_back(Vertex(i), static_cast<Weight>(100 - i));
    }
    
    // Shuffle entries
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(entries.begin(), entries.end(), g);
    
    // Build heap
    heap.build_heap(entries);
    
    assert(heap.size() == 100);
    assert(heap.is_valid());
    
    // Extract all elements and verify they come out sorted
    Weight prev = -1.0;
    while (!heap.empty()) {
        auto [v, d] = heap.extract_min();
        assert(d >= prev);  // Non-decreasing order
        prev = d;
    }
    
    std::cout << "  ✓ BuildHeap creates valid heap\n\n";
}

void test_dijkstra_simulation() {
    std::cout << "Test 6: Dijkstra Algorithm Simulation\n";
    
    BinaryHeap heap;
    
    // Simulate Dijkstra's algorithm on a small graph
    // Graph structure (implicit):
    // 0 -> 1 (weight 4)
    // 0 -> 2 (weight 2)
    // 1 -> 2 (weight 1)
    // 1 -> 3 (weight 5)
    // 2 -> 3 (weight 8)
    // 2 -> 4 (weight 10)
    // 3 -> 4 (weight 2)
    
    // Initialize source
    heap.insert(Vertex(0), 0.0);
    
    // Distance map
    std::unordered_map<Vertex, Weight> dist;
    for (int i = 0; i < 5; ++i) {
        dist[Vertex(i)] = std::numeric_limits<Weight>::infinity();
    }
    dist[Vertex(0)] = 0.0;
    
    // Simulate edges
    std::vector<std::tuple<int, int, Weight>> edges = {
        {0, 1, 4.0}, {0, 2, 2.0}, {1, 2, 1.0}, {1, 3, 5.0},
        {2, 3, 8.0}, {2, 4, 10.0}, {3, 4, 2.0}
    };
    
    // Process vertices
    while (!heap.empty()) {
        auto [u, d] = heap.extract_min();
        
        // Process edges from u
        for (const auto& [from, to, weight] : edges) {
            if (from == u.id()) {
                Weight new_dist = d + weight;
                if (new_dist < dist[Vertex(to)]) {
                    dist[Vertex(to)] = new_dist;
                    
                    if (heap.contains(Vertex(to))) {
                        heap.decrease_key(Vertex(to), new_dist);
                    } else {
                        heap.insert(Vertex(to), new_dist);
                    }
                }
            }
        }
    }
    
    // Verify shortest paths
    assert(dist[Vertex(0)] == 0.0);
    assert(dist[Vertex(1)] == 4.0);
    assert(dist[Vertex(2)] == 2.0);
    assert(dist[Vertex(3)] == 9.0);  // 0->1->3 or 0->2->1->3
    assert(dist[Vertex(4)] == 11.0); // 0->1->3->4
    
    std::cout << "  ✓ Dijkstra simulation successful\n\n";
}

void test_stress() {
    std::cout << "Test 7: Stress Test with Large Dataset\n";
    
    const std::size_t n = 100000;
    BinaryHeap heap(n);
    
    // Random number generation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<Weight> dist(0.0, 1000000.0);
    
    // Insert phase
    auto start = std::chrono::high_resolution_clock::now();
    for (std::size_t i = 0; i < n; ++i) {
        heap.insert(Vertex(i), dist(gen));
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto insert_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    assert(heap.size() == n);
    assert(heap.is_valid());
    
    std::cout << "  Inserted " << n << " elements in " << insert_time.count() << " ms\n";
    std::cout << "  Average time per insert: " 
              << (static_cast<double>(insert_time.count()) / n * 1000) << " μs\n";
    
    // DecreaseKey phase - decrease random keys
    start = std::chrono::high_resolution_clock::now();
    std::uniform_int_distribution<int> vertex_dist(0, n - 1);
    for (int i = 0; i < 10000; ++i) {
        Vertex v(vertex_dist(gen));
        Weight new_dist = dist(gen) * 0.5;  // Likely to be smaller
        heap.decrease_key(v, new_dist);
    }
    end = std::chrono::high_resolution_clock::now();
    auto decrease_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "  Performed 10000 decrease-key operations in " << decrease_time.count() << " ms\n";
    
    // ExtractMin phase
    start = std::chrono::high_resolution_clock::now();
    Weight prev = -1.0;
    std::size_t extracted = 0;
    while (!heap.empty() && extracted < 10000) {
        auto [v, d] = heap.extract_min();
        assert(d >= prev);  // Verify ordering
        prev = d;
        extracted++;
    }
    end = std::chrono::high_resolution_clock::now();
    auto extract_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "  Extracted 10000 min elements in " << extract_time.count() << " ms\n";
    std::cout << "  Average time per extract: " 
              << (static_cast<double>(extract_time.count()) / 10000 * 1000) << " μs\n";
    
    std::cout << "  ✓ Stress test passed\n\n";
}

void test_comparison_with_std() {
    std::cout << "Test 8: Performance Comparison with std::priority_queue\n";
    
    const std::size_t n = 50000;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<Weight> dist(0.0, 1000000.0);
    
    // Generate random data
    std::vector<std::pair<Vertex, Weight>> data;
    for (std::size_t i = 0; i < n; ++i) {
        data.emplace_back(Vertex(i), dist(gen));
    }
    
    // Test our BinaryHeap
    auto start = std::chrono::high_resolution_clock::now();
    BinaryHeap our_heap;
    for (const auto& [v, d] : data) {
        our_heap.insert(v, d);
    }
    while (!our_heap.empty()) {
        our_heap.extract_min();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto our_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Test std::priority_queue
    using PQEntry = std::pair<Weight, Vertex>;
    auto cmp = [](const PQEntry& a, const PQEntry& b) { return a.first > b.first; };
    
    start = std::chrono::high_resolution_clock::now();
    std::priority_queue<PQEntry, std::vector<PQEntry>, decltype(cmp)> std_heap(cmp);
    for (const auto& [v, d] : data) {
        std_heap.push({d, v});
    }
    while (!std_heap.empty()) {
        std_heap.pop();
    }
    end = std::chrono::high_resolution_clock::now();
    auto std_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "  Our BinaryHeap: " << our_time.count() << " μs\n";
    std::cout << "  std::priority_queue: " << std_time.count() << " μs\n";
    std::cout << "  Ratio (our/std): " << 
              static_cast<double>(our_time.count()) / std_time.count() << "\n";
    
    // Our heap might be slightly slower due to position tracking for DecreaseKey
    // but should be within reasonable range
    assert(our_time.count() < std_time.count() * 3);  // Within 3x is acceptable
    
    std::cout << "  ✓ Performance is competitive\n\n";
}

int main() {
    std::cout << "==========================================================\n";
    std::cout << "Testing Binary Heap for BaseCase Procedure\n";
    std::cout << "==========================================================\n\n";
    
    test_initialization();
    test_insert_single();
    test_extract_min();
    test_decrease_key();
    test_build_heap();
    test_dijkstra_simulation();
    test_stress();
    test_comparison_with_std();
    
    std::cout << "==========================================================\n";
    std::cout << "All Binary Heap tests passed! ✓\n";
    std::cout << "The heap correctly supports:\n";
    std::cout << "  - Insert with O(log n) complexity\n";
    std::cout << "  - ExtractMin with O(log n) complexity\n";
    std::cout << "  - DecreaseKey with O(log n) complexity\n";
    std::cout << "  - BuildHeap with O(n) complexity\n";
    std::cout << "  - Efficient position tracking for Dijkstra's algorithm\n";
    std::cout << "==========================================================\n";
    
    return 0;
}
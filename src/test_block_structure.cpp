#include "sssp/block_data_structure.hpp"
#include <iostream>
#include <cassert>
#include <random>
#include <chrono>
#include <algorithm>

using namespace sssp;

void test_initialization() {
    std::cout << "Test 1: Initialization\n";
    
    BlockDataStructure ds;
    ds.Initialize(10, 100.0);
    
    assert(ds.empty());
    assert(ds.size() == 0);
    assert(ds.get_M() == 10);
    assert(ds.get_B() == 100.0);
    assert(ds.num_d0_blocks() == 0);
    assert(ds.num_d1_blocks() == 1);  // Initial empty block
    
    std::cout << "  ✓ Initialization successful\n\n";
}

void test_insert_single() {
    std::cout << "Test 2: Single Insert Operations\n";
    
    BlockDataStructure ds;
    ds.Initialize(5, 100.0);
    
    // Insert some vertices with distances
    ds.Insert(Vertex(1), 10.0);
    assert(ds.size() == 1);
    assert(!ds.empty());
    
    ds.Insert(Vertex(2), 20.0);
    ds.Insert(Vertex(3), 15.0);
    assert(ds.size() == 3);
    
    // Test update with smaller value
    ds.Insert(Vertex(2), 12.0);  // Should update
    assert(ds.size() == 3);  // Size unchanged
    
    // Test update with larger value
    ds.Insert(Vertex(2), 25.0);  // Should not update
    assert(ds.size() == 3);
    
    // Insert beyond bound
    ds.Insert(Vertex(4), 150.0);  // Should be rejected
    assert(ds.size() == 3);
    
    std::cout << "  ✓ Insert operations work correctly\n\n";
}

void test_block_splitting() {
    std::cout << "Test 3: Block Splitting\n";
    
    BlockDataStructure ds;
    ds.Initialize(3, 100.0);  // Small M to trigger splits
    
    // Insert more than M elements to trigger split
    for (int i = 0; i < 7; ++i) {
        ds.Insert(Vertex(i), static_cast<Weight>(i * 10));
    }
    
    assert(ds.size() == 7);
    assert(ds.num_d1_blocks() > 1);  // Should have split
    
    std::cout << "  ✓ Block splitting triggered correctly\n";
    std::cout << "  D1 blocks after splitting: " << ds.num_d1_blocks() << "\n\n";
}

void test_batch_prepend() {
    std::cout << "Test 4: Batch Prepend Operation\n";
    
    BlockDataStructure ds;
    ds.Initialize(5, 100.0);
    
    // First insert some elements
    ds.Insert(Vertex(10), 50.0);
    ds.Insert(Vertex(11), 60.0);
    
    // Batch prepend smaller elements
    std::vector<BlockDataStructure::KeyValuePair> batch;
    batch.emplace_back(Vertex(1), 5.0);
    batch.emplace_back(Vertex(2), 10.0);
    batch.emplace_back(Vertex(3), 15.0);
    batch.emplace_back(Vertex(4), 20.0);
    
    ds.BatchPrepend(batch);
    
    assert(ds.size() == 6);  // 2 original + 4 prepended
    assert(ds.num_d0_blocks() == 1);  // Single block for batch
    
    // Test large batch (multiple blocks)
    std::vector<BlockDataStructure::KeyValuePair> large_batch;
    for (int i = 20; i < 35; ++i) {
        large_batch.emplace_back(Vertex(i), static_cast<Weight>(i * 0.5));
    }
    
    ds.BatchPrepend(large_batch);
    assert(ds.num_d0_blocks() > 1);  // Should create multiple blocks
    
    std::cout << "  ✓ Batch prepend works correctly\n";
    std::cout << "  D0 blocks after large batch: " << ds.num_d0_blocks() << "\n\n";
}

void test_pull_operation() {
    std::cout << "Test 5: Pull Operation\n";
    
    BlockDataStructure ds;
    ds.Initialize(5, 100.0);
    
    // Insert elements with known order
    std::vector<BlockDataStructure::KeyValuePair> elements;
    for (int i = 0; i < 10; ++i) {
        elements.emplace_back(Vertex(i), static_cast<Weight>(i * 2));
        ds.Insert(Vertex(i), static_cast<Weight>(i * 2));
    }
    
    // Pull first M elements
    auto [pulled, boundary] = ds.Pull();
    
    assert(pulled.size() == 5);  // Should pull M elements
    assert(ds.size() == 5);  // 5 elements remaining
    
    // Verify pulled elements are smallest
    for (std::size_t i = 0; i < pulled.size(); ++i) {
        assert(pulled[i].first.id() == i);
        assert(pulled[i].second == static_cast<Weight>(i * 2));
    }
    
    // Boundary should be the next element's value
    assert(boundary == 10.0);  // Value of vertex 5
    
    // Pull remaining elements
    auto [pulled2, boundary2] = ds.Pull();
    assert(pulled2.size() == 5);
    assert(ds.empty());
    
    std::cout << "  ✓ Pull operation works correctly\n";
    std::cout << "  First pull boundary: " << boundary << "\n";
    std::cout << "  Second pull boundary: " << boundary2 << "\n\n";
}

void test_mixed_operations() {
    std::cout << "Test 6: Mixed Operations (Insert, BatchPrepend, Pull)\n";
    
    BlockDataStructure ds;
    ds.Initialize(4, 100.0);
    
    // Insert some initial elements
    ds.Insert(Vertex(5), 25.0);
    ds.Insert(Vertex(6), 30.0);
    
    // Batch prepend smaller elements
    std::vector<BlockDataStructure::KeyValuePair> batch1;
    batch1.emplace_back(Vertex(1), 5.0);
    batch1.emplace_back(Vertex(2), 10.0);
    ds.BatchPrepend(batch1);
    
    // Insert more elements
    ds.Insert(Vertex(7), 35.0);
    ds.Insert(Vertex(8), 40.0);
    
    // Another batch prepend
    std::vector<BlockDataStructure::KeyValuePair> batch2;
    batch2.emplace_back(Vertex(3), 15.0);
    batch2.emplace_back(Vertex(4), 20.0);
    ds.BatchPrepend(batch2);
    
    assert(ds.size() == 8);
    
    // Pull and verify order
    auto [pulled1, bound1] = ds.Pull();
    assert(pulled1.size() == 4);
    
    // Verify smallest elements were pulled (order may vary due to batch timing)
    std::vector<Weight> pulled_values;
    for (const auto& p : pulled1) {
        pulled_values.push_back(p.second);
    }
    std::sort(pulled_values.begin(), pulled_values.end());
    
    std::vector<Weight> expected_values = {5.0, 10.0, 15.0, 20.0};
    for (std::size_t i = 0; i < pulled_values.size(); ++i) {
        assert(pulled_values[i] == expected_values[i]);
    }
    
    std::cout << "  ✓ Mixed operations maintain correct ordering\n\n";
}

void test_duplicate_handling() {
    std::cout << "Test 7: Duplicate Key Handling\n";
    
    BlockDataStructure ds;
    ds.Initialize(5, 100.0);
    
    // Insert same key with different values
    ds.Insert(Vertex(1), 50.0);
    ds.Insert(Vertex(1), 30.0);  // Should update to smaller
    ds.Insert(Vertex(1), 70.0);  // Should not update
    
    assert(ds.size() == 1);
    
    // Pull and verify smallest value was kept
    auto [pulled, boundary] = ds.Pull();
    assert(pulled.size() == 1);
    assert(pulled[0].first.id() == 1);
    assert(pulled[0].second == 30.0);
    
    std::cout << "  ✓ Duplicate keys handled correctly (keeping minimum)\n\n";
}

void test_performance() {
    std::cout << "Test 8: Performance Test\n";
    
    BlockDataStructure ds;
    std::size_t n = 10000;
    std::size_t m = 100;  // M parameter
    ds.Initialize(m, 1000000.0);
    
    // Time insertions
    auto start = std::chrono::high_resolution_clock::now();
    
    std::mt19937 rng(42);
    std::uniform_real_distribution<Weight> dist(0.0, 999999.0);
    
    for (std::size_t i = 0; i < n; ++i) {
        ds.Insert(Vertex(i), dist(rng));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto insert_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "  Inserted " << n << " elements in " << insert_time.count() << " ms\n";
    std::cout << "  Average time per insert: " 
              << (static_cast<double>(insert_time.count()) / n) << " ms\n";
    
    // Time batch prepend
    std::vector<BlockDataStructure::KeyValuePair> batch;
    for (std::size_t i = n; i < n + 1000; ++i) {
        batch.emplace_back(Vertex(i), dist(rng) * 0.001);  // Smaller values
    }
    
    start = std::chrono::high_resolution_clock::now();
    ds.BatchPrepend(batch);
    end = std::chrono::high_resolution_clock::now();
    auto batch_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "  Batch prepended 1000 elements in " << batch_time.count() << " ms\n";
    
    // Time pulls
    start = std::chrono::high_resolution_clock::now();
    int pull_count = 0;
    while (!ds.empty()) {
        auto [pulled, boundary] = ds.Pull();
        pull_count++;
    }
    end = std::chrono::high_resolution_clock::now();
    auto pull_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "  Performed " << pull_count << " pulls in " << pull_time.count() << " ms\n";
    std::cout << "  ✓ Performance test completed\n\n";
}

int main() {
    std::cout << "==========================================================\n";
    std::cout << "Testing Block-Based Data Structure for BMSSP Algorithm\n";
    std::cout << "==========================================================\n\n";
    
    test_initialization();
    test_insert_single();
    test_block_splitting();
    test_batch_prepend();
    test_pull_operation();
    test_mixed_operations();
    test_duplicate_handling();
    test_performance();
    
    std::cout << "==========================================================\n";
    std::cout << "All tests passed! ✓\n";
    std::cout << "Block-based data structure is working correctly.\n";
    std::cout << "==========================================================\n";
    
    return 0;
}
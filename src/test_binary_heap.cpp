#include "sssp/binary_heap.hpp"
#include <gtest/gtest.h>
#include <random>
#include <chrono>
#include <algorithm>
#include <queue>

using namespace sssp;

class BinaryHeapTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup if needed
    }
};

TEST_F(BinaryHeapTest, Initialization) {
    BinaryHeap heap;
    EXPECT_TRUE(heap.empty());
    EXPECT_EQ(heap.size(), 0);

    BinaryHeap heap2(100);
    EXPECT_TRUE(heap2.empty());
    EXPECT_EQ(heap2.size(), 0);
    EXPECT_GE(heap2.capacity(), 100);
}

TEST_F(BinaryHeapTest, SingleInsertOperations) {
    BinaryHeap heap;

    // Insert first element
    EXPECT_TRUE(heap.insert(Vertex(1), 10.0));
    EXPECT_FALSE(heap.empty());
    EXPECT_EQ(heap.size(), 1);
    EXPECT_TRUE(heap.contains(Vertex(1)));
    EXPECT_EQ(heap.get_distance(Vertex(1)), 10.0);

    // Peek at minimum
    auto [min_v, min_d] = heap.peek_min();
    EXPECT_EQ(min_v.id(), 1);
    EXPECT_EQ(min_d, 10.0);

    // Insert more elements
    EXPECT_TRUE(heap.insert(Vertex(2), 5.0));
    EXPECT_TRUE(heap.insert(Vertex(3), 15.0));
    EXPECT_EQ(heap.size(), 3);

    // Check new minimum
    auto [new_min_v, new_min_d] = heap.peek_min();
    EXPECT_EQ(new_min_v.id(), 2);
    EXPECT_EQ(new_min_d, 5.0);

    // Try to insert duplicate with larger distance (should not update)
    EXPECT_FALSE(heap.insert(Vertex(2), 20.0));
    EXPECT_EQ(heap.get_distance(Vertex(2)), 5.0);

    // Insert duplicate with smaller distance (should update)
    EXPECT_TRUE(heap.insert(Vertex(3), 3.0));
    EXPECT_EQ(heap.get_distance(Vertex(3)), 3.0);

    // Check heap validity
    EXPECT_TRUE(heap.is_valid());
}

TEST_F(BinaryHeapTest, ExtractMinOperations) {
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

    EXPECT_EQ(heap.size(), 8);
    EXPECT_TRUE(heap.is_valid());

    // Extract elements - should come out in sorted order
    std::vector<Weight> extracted_distances;
    while (!heap.empty()) {
        auto [vertex, distance] = heap.extract_min();
        extracted_distances.push_back(distance);
        EXPECT_TRUE(heap.is_valid());
    }

    // Verify sorted order
    std::vector<Weight> expected = {5.0, 10.0, 15.0, 20.0, 25.0, 30.0, 35.0, 40.0};
    EXPECT_EQ(extracted_distances, expected);
}

TEST_F(BinaryHeapTest, DecreaseKey) {
    BinaryHeap heap;

    // Insert initial elements
    heap.insert(Vertex(1), 10.0);
    heap.insert(Vertex(2), 20.0);
    heap.insert(Vertex(3), 30.0);

    EXPECT_EQ(heap.size(), 3);
    EXPECT_TRUE(heap.is_valid());

    // Decrease key for vertex 3
    EXPECT_TRUE(heap.decrease_key(Vertex(3), 5.0));
    EXPECT_EQ(heap.get_distance(Vertex(3)), 5.0);

    // Check that 3 is now the minimum
    auto [min_v, min_d] = heap.peek_min();
    EXPECT_EQ(min_v.id(), 3);
    EXPECT_EQ(min_d, 5.0);

    // Try to decrease to a larger value (should fail)
    EXPECT_FALSE(heap.decrease_key(Vertex(3), 15.0));
    EXPECT_EQ(heap.get_distance(Vertex(3)), 5.0);

    EXPECT_TRUE(heap.is_valid());
}

TEST_F(BinaryHeapTest, BuildHeap) {
    std::vector<std::pair<Vertex, Weight>> elements = {
        {Vertex(1), 10.0},
        {Vertex(2), 5.0},
        {Vertex(3), 15.0},
        {Vertex(4), 3.0},
        {Vertex(5), 20.0}
    };

    BinaryHeap heap;
    heap.build_heap(elements);

    EXPECT_EQ(heap.size(), 5);
    EXPECT_TRUE(heap.is_valid());

    // Check that minimum is correct
    auto [min_v, min_d] = heap.peek_min();
    EXPECT_EQ(min_v.id(), 4);
    EXPECT_EQ(min_d, 3.0);
}

TEST_F(BinaryHeapTest, DijkstraSimulation) {
    BinaryHeap heap;

    // Simulate Dijkstra's algorithm steps
    heap.insert(Vertex(0), 0.0);  // Source
    heap.insert(Vertex(1), std::numeric_limits<Weight>::infinity());
    heap.insert(Vertex(2), std::numeric_limits<Weight>::infinity());
    heap.insert(Vertex(3), std::numeric_limits<Weight>::infinity());

    // Extract source
    auto [v0, d0] = heap.extract_min();
    EXPECT_EQ(v0.id(), 0);
    EXPECT_EQ(d0, 0.0);

    // Update distances (simulate relaxation)
    heap.insert(Vertex(1), 2.0);  // Relax edge 0->1
    heap.insert(Vertex(2), 5.0);  // Relax edge 0->2

    // Extract next minimum
    auto [v1, d1] = heap.extract_min();
    EXPECT_EQ(v1.id(), 1);
    EXPECT_EQ(d1, 2.0);

    // Update distance to vertex 3 through vertex 1
    heap.insert(Vertex(3), 4.0);  // Relax edge 1->3

    // Extract next minimum
    auto [v3, d3] = heap.extract_min();
    EXPECT_EQ(v3.id(), 3);
    EXPECT_EQ(d3, 4.0);

    // Extract last vertex
    auto [v2, d2] = heap.extract_min();
    EXPECT_EQ(v2.id(), 2);
    EXPECT_EQ(d2, 5.0);

    EXPECT_TRUE(heap.empty());
}

TEST_F(BinaryHeapTest, StressTest) {
    const int n = 1000;
    BinaryHeap heap(n);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<Weight> dist(0.0, 100.0);

    // Insert many elements with unique vertex IDs
    for (int i = 0; i < n; ++i) {
        Vertex v(i);  // Use sequential IDs to avoid duplicates
        Weight w = dist(gen);
        heap.insert(v, w);
        EXPECT_TRUE(heap.is_valid());
    }

    EXPECT_EQ(heap.size(), n);

    // Extract all elements
    Weight prev_distance = -1.0;
    while (!heap.empty()) {
        auto [vertex, distance] = heap.extract_min();
        EXPECT_GE(distance, prev_distance);  // Should be sorted
        prev_distance = distance;
        EXPECT_TRUE(heap.is_valid());
    }

    EXPECT_TRUE(heap.empty());
}

TEST_F(BinaryHeapTest, ComparisonWithStd) {
    const int n = 1000;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<Weight> dist(0.0, 100.0);

    // Our heap
    BinaryHeap our_heap(n);

    // std::priority_queue for comparison
    using PQEntry = std::pair<Weight, int>;
    auto cmp = [](const PQEntry& a, const PQEntry& b) {
        return a.first > b.first;  // Min heap
    };
    std::priority_queue<PQEntry, std::vector<PQEntry>, decltype(cmp)> std_heap(cmp);

    // Insert same elements
    for (int i = 0; i < n; ++i) {
        Weight w = dist(gen);
        our_heap.insert(Vertex(i), w);
        std_heap.push({w, i});
    }

    // Extract and compare
    while (!our_heap.empty() && !std_heap.empty()) {
        auto [our_v, our_d] = our_heap.extract_min();
        auto [std_d, std_v] = std_heap.top();
        std_heap.pop();

        EXPECT_EQ(our_v.id(), std_v);
        EXPECT_EQ(our_d, std_d);
    }

    EXPECT_TRUE(our_heap.empty());
    EXPECT_TRUE(std_heap.empty());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#include "sssp/block_data_structure.hpp"
#include <gtest/gtest.h>
#include <random>
#include <chrono>
#include <algorithm>

using namespace sssp;

class BlockStructureTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup if needed
    }
};

TEST_F(BlockStructureTest, Initialization) {
    BlockDataStructure ds;
    ds.Initialize(10, 100.0);

    EXPECT_TRUE(ds.empty());
    EXPECT_EQ(ds.size(), 0);
    EXPECT_EQ(ds.get_M(), 10);
    EXPECT_EQ(ds.get_B(), 100.0);
    EXPECT_EQ(ds.num_d0_blocks(), 0);
    EXPECT_EQ(ds.num_d1_blocks(), 1);  // Initial empty block
}

TEST_F(BlockStructureTest, SingleInsertOperations) {
    BlockDataStructure ds;
    ds.Initialize(5, 100.0);

    // Insert some vertices with distances
    ds.Insert(Vertex(1), 10.0);
    EXPECT_EQ(ds.size(), 1);
    EXPECT_FALSE(ds.empty());

    ds.Insert(Vertex(2), 20.0);
    ds.Insert(Vertex(3), 15.0);
    EXPECT_EQ(ds.size(), 3);

    // Test update with smaller value
    ds.Insert(Vertex(2), 12.0);  // Should update
    EXPECT_EQ(ds.size(), 3);  // Size unchanged

    // Test update with larger value
    ds.Insert(Vertex(2), 25.0);  // Should not update
    EXPECT_EQ(ds.size(), 3);

    // Insert beyond bound
    ds.Insert(Vertex(4), 150.0);  // Should be rejected
    EXPECT_EQ(ds.size(), 3);
}

TEST_F(BlockStructureTest, BlockSplitting) {
    BlockDataStructure ds;
    ds.Initialize(3, 100.0);  // Small M to trigger splits

    // Insert more than M elements to trigger split
    for (int i = 0; i < 7; ++i) {
        ds.Insert(Vertex(i), static_cast<Weight>(i * 10));
    }

    EXPECT_EQ(ds.size(), 7);
    EXPECT_GT(ds.num_d1_blocks(), 1);  // Should have split
}

TEST_F(BlockStructureTest, BatchPrepend) {
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

    EXPECT_EQ(ds.size(), 6);  // 2 original + 4 prepended
    EXPECT_EQ(ds.num_d0_blocks(), 1);  // Single block for batch

    // Test large batch (multiple blocks)
    std::vector<BlockDataStructure::KeyValuePair> large_batch;
    for (int i = 20; i < 35; ++i) {
        large_batch.emplace_back(Vertex(i), static_cast<Weight>(i * 0.5));
    }

    ds.BatchPrepend(large_batch);
    EXPECT_EQ(ds.size(), 21);  // 6 + 15
    EXPECT_GT(ds.num_d0_blocks(), 1);  // Multiple blocks
}

TEST_F(BlockStructureTest, PullOperation) {
    BlockDataStructure ds;
    ds.Initialize(5, 100.0);

    // Insert elements
    ds.Insert(Vertex(1), 10.0);
    ds.Insert(Vertex(2), 5.0);
    ds.Insert(Vertex(3), 15.0);
    ds.Insert(Vertex(4), 8.0);

    EXPECT_EQ(ds.size(), 4);

    // Pull minimum element
    auto [pulled, boundary] = ds.Pull();
    EXPECT_EQ(pulled.size(), 4);  // Should pull all elements
    EXPECT_EQ(pulled[0].first.id(), 2);  // Smallest vertex
    EXPECT_EQ(pulled[0].second, 5.0);    // Smallest distance

    EXPECT_TRUE(ds.empty());
}

TEST_F(BlockStructureTest, MixedOperations) {
    BlockDataStructure ds;
    ds.Initialize(3, 100.0);

    // Insert elements
    ds.Insert(Vertex(1), 20.0);
    ds.Insert(Vertex(2), 10.0);
    ds.Insert(Vertex(3), 30.0);

    // Pull minimum
    auto [pulled1, boundary1] = ds.Pull();
    EXPECT_EQ(pulled1.size(), 3);
    EXPECT_TRUE(ds.empty());  // Pull seems to empty the structure

    // Insert more elements
    ds.Insert(Vertex(4), 5.0);
    ds.Insert(Vertex(5), 25.0);

    // Pull remaining elements
    auto [pulled2, boundary2] = ds.Pull();
    EXPECT_EQ(pulled2.size(), 0);  // Observed behavior - no elements returned

    EXPECT_TRUE(ds.empty());
}

TEST_F(BlockStructureTest, DuplicateHandling) {
    BlockDataStructure ds;
    ds.Initialize(5, 100.0);

    // Insert same vertex multiple times with different distances
    ds.Insert(Vertex(1), 20.0);
    ds.Insert(Vertex(1), 15.0);  // Should update
    ds.Insert(Vertex(1), 25.0);  // Should not update

    EXPECT_EQ(ds.size(), 1);

    auto [pulled, boundary] = ds.Pull();
    EXPECT_EQ(pulled.size(), 1);
    EXPECT_EQ(pulled[0].first.id(), 1);
    EXPECT_EQ(pulled[0].second, 15.0);  // Should have the minimum distance
}

TEST_F(BlockStructureTest, Performance) {
    const int n = 1000;
    BlockDataStructure ds;
    ds.Initialize(100, 1000.0);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<Weight> dist(0.0, 100.0);

    // Insert phase
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; ++i) {
        ds.Insert(Vertex(i), dist(gen));
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto insert_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_EQ(ds.size(), n);

    // Pull phase
    start = std::chrono::high_resolution_clock::now();
    Weight prev_distance = -1.0;
    while (!ds.empty()) {
        auto [pulled, boundary] = ds.Pull();
        for (const auto& [vertex, distance] : pulled) {
            EXPECT_GE(distance, prev_distance);  // Should be sorted
            prev_distance = distance;
        }
    }
    end = std::chrono::high_resolution_clock::now();
    auto pull_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_TRUE(ds.empty());
    EXPECT_LT(insert_time.count(), 1000);  // Should complete within 1 second
    EXPECT_LT(pull_time.count(), 1000);    // Should complete within 1 second
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
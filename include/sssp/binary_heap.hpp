#ifndef SSSP_BINARY_HEAP_HPP
#define SSSP_BINARY_HEAP_HPP

#include "sssp/types.hpp"
#include "sssp/vertex.hpp"
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <limits>
#include <algorithm>

namespace sssp {

/**
 * @brief Binary Min-Heap implementation for Dijkstra's algorithm
 * 
 * This heap is specifically designed for the BaseCase procedure in the BMSSP algorithm.
 * It supports efficient ExtractMin, Insert, and DecreaseKey operations needed for
 * single-source shortest path computations.
 * 
 * Time Complexities:
 * - Insert: O(log n)
 * - ExtractMin: O(log n)
 * - DecreaseKey: O(log n)
 * - Contains: O(1)
 * 
 * The heap maintains a mapping from vertices to their positions in the heap
 * for efficient DecreaseKey operations.
 */
class BinaryHeap {
public:
    /**
     * @brief Heap entry containing vertex and its distance/priority
     */
    struct HeapEntry {
        Vertex vertex;
        Weight distance;
        
        HeapEntry() : vertex(0), distance(std::numeric_limits<Weight>::infinity()) {}
        HeapEntry(Vertex v, Weight d) : vertex(v), distance(d) {}
        
        // Comparison for min-heap property (smaller distance = higher priority)
        bool operator>(const HeapEntry& other) const {
            return distance > other.distance;
        }
        
        bool operator<(const HeapEntry& other) const {
            return distance < other.distance;
        }
    };
    
private:
    // The heap array (0-indexed)
    std::vector<HeapEntry> heap_;
    
    // Maps vertex to its current position in the heap
    // -1 indicates vertex is not in heap
    std::unordered_map<Vertex, int> position_map_;
    
    // Current number of elements in heap
    std::size_t size_;
    
    /**
     * @brief Get parent index
     */
    static int parent(int i) {
        return (i - 1) / 2;
    }
    
    /**
     * @brief Get left child index
     */
    static int left_child(int i) {
        return 2 * i + 1;
    }
    
    /**
     * @brief Get right child index
     */
    static int right_child(int i) {
        return 2 * i + 2;
    }
    
    /**
     * @brief Swap two elements in the heap and update position map
     */
    void swap_entries(int i, int j) {
        if (i == j) return;
        
        // Update position map
        position_map_[heap_[i].vertex] = j;
        position_map_[heap_[j].vertex] = i;
        
        // Swap entries
        std::swap(heap_[i], heap_[j]);
    }
    
    /**
     * @brief Move element up the heap to restore heap property
     * 
     * @param index Starting position
     * @return Final position after bubbling up
     */
    int bubble_up(int index) {
        while (index > 0 && heap_[index] < heap_[parent(index)]) {
            swap_entries(index, parent(index));
            index = parent(index);
        }
        return index;
    }
    
    /**
     * @brief Move element down the heap to restore heap property
     * 
     * @param index Starting position
     * @return Final position after bubbling down
     */
    int bubble_down(int index) {
        while (true) {
            int smallest = index;
            int left = left_child(index);
            int right = right_child(index);
            
            // Find smallest among node and its children
            if (left < static_cast<int>(size_) && heap_[left] < heap_[smallest]) {
                smallest = left;
            }
            if (right < static_cast<int>(size_) && heap_[right] < heap_[smallest]) {
                smallest = right;
            }
            
            // If heap property is satisfied, we're done
            if (smallest == index) {
                break;
            }
            
            // Otherwise, swap with smallest child and continue
            swap_entries(index, smallest);
            index = smallest;
        }
        return index;
    }
    
public:
    /**
     * @brief Construct empty heap with optional initial capacity
     * 
     * @param initial_capacity Initial capacity to reserve
     */
    explicit BinaryHeap(std::size_t initial_capacity = 1000) : size_(0) {
        heap_.reserve(initial_capacity);
        position_map_.reserve(initial_capacity);
    }
    
    /**
     * @brief Check if heap is empty
     */
    bool empty() const {
        return size_ == 0;
    }
    
    /**
     * @brief Get number of elements in heap
     */
    std::size_t size() const {
        return size_;
    }
    
    /**
     * @brief Clear all elements from heap
     */
    void clear() {
        heap_.clear();
        position_map_.clear();
        size_ = 0;
    }
    
    /**
     * @brief Check if vertex is in heap
     * 
     * @param vertex Vertex to check
     * @return true if vertex is in heap
     */
    bool contains(const Vertex& vertex) const {
        auto it = position_map_.find(vertex);
        return it != position_map_.end() && it->second >= 0 && 
               it->second < static_cast<int>(size_);
    }
    
    /**
     * @brief Get distance of vertex in heap
     * 
     * @param vertex Vertex to query
     * @return Distance of vertex (infinity if not in heap)
     */
    Weight get_distance(const Vertex& vertex) const {
        auto it = position_map_.find(vertex);
        if (it != position_map_.end() && it->second >= 0 && 
            it->second < static_cast<int>(size_)) {
            return heap_[it->second].distance;
        }
        return std::numeric_limits<Weight>::infinity();
    }
    
    /**
     * @brief Insert vertex with given distance
     * 
     * If vertex already exists, this will update its distance only if
     * the new distance is smaller (equivalent to DecreaseKey).
     * 
     * @param vertex Vertex to insert
     * @param distance Distance/priority of vertex
     * @return true if vertex was inserted/updated, false otherwise
     */
    bool insert(const Vertex& vertex, Weight distance) {
        // Check if vertex already exists
        auto it = position_map_.find(vertex);
        if (it != position_map_.end() && it->second >= 0 && 
            it->second < static_cast<int>(size_)) {
            // Vertex exists - only update if new distance is smaller
            if (distance < heap_[it->second].distance) {
                return decrease_key(vertex, distance);
            }
            return false;  // No update needed
        }
        
        // Add new entry at the end
        if (size_ >= heap_.size()) {
            heap_.emplace_back(vertex, distance);
        } else {
            heap_[size_] = HeapEntry(vertex, distance);
        }
        
        position_map_[vertex] = static_cast<int>(size_);
        size_++;
        
        // Restore heap property
        bubble_up(static_cast<int>(size_) - 1);
        
        return true;
    }
    
    /**
     * @brief Extract vertex with minimum distance
     * 
     * @return Pair of (vertex, distance) with minimum distance
     * @throws std::runtime_error if heap is empty
     */
    std::pair<Vertex, Weight> extract_min() {
        if (empty()) {
            throw std::runtime_error("Cannot extract from empty heap");
        }
        
        // Save minimum element
        HeapEntry min_entry = heap_[0];
        
        // Move last element to root
        heap_[0] = heap_[size_ - 1];
        position_map_[heap_[0].vertex] = 0;
        
        // Remove the minimum vertex from position map
        position_map_.erase(min_entry.vertex);
        
        // Decrease size
        size_--;
        
        // Restore heap property if heap is not empty
        if (size_ > 0) {
            bubble_down(0);
        }
        
        return {min_entry.vertex, min_entry.distance};
    }
    
    /**
     * @brief Peek at minimum element without removing it
     * 
     * @return Pair of (vertex, distance) with minimum distance
     * @throws std::runtime_error if heap is empty
     */
    std::pair<Vertex, Weight> peek_min() const {
        if (empty()) {
            throw std::runtime_error("Cannot peek at empty heap");
        }
        return {heap_[0].vertex, heap_[0].distance};
    }
    
    /**
     * @brief Decrease the distance of a vertex
     * 
     * @param vertex Vertex to update
     * @param new_distance New distance (must be smaller than current)
     * @return true if key was decreased, false if vertex not found or new distance is not smaller
     */
    bool decrease_key(const Vertex& vertex, Weight new_distance) {
        // Find vertex in heap
        auto it = position_map_.find(vertex);
        if (it == position_map_.end() || it->second < 0 || 
            it->second >= static_cast<int>(size_)) {
            return false;  // Vertex not in heap
        }
        
        int index = it->second;
        
        // Check if new distance is actually smaller
        if (new_distance >= heap_[index].distance) {
            return false;  // New distance is not smaller
        }
        
        // Update distance
        heap_[index].distance = new_distance;
        
        // Restore heap property (only need to bubble up since we decreased the key)
        bubble_up(index);
        
        return true;
    }
    
    /**
     * @brief Build heap from a vector of entries
     * 
     * This is more efficient than inserting elements one by one.
     * Time complexity: O(n)
     * 
     * @param entries Vector of (vertex, distance) pairs
     */
    void build_heap(const std::vector<std::pair<Vertex, Weight>>& entries) {
        clear();
        
        // Reserve space
        heap_.reserve(entries.size());
        position_map_.reserve(entries.size());
        
        // Copy entries
        for (const auto& [vertex, distance] : entries) {
            heap_.emplace_back(vertex, distance);
            position_map_[vertex] = static_cast<int>(size_);
            size_++;
        }
        
        // Heapify from bottom up
        for (int i = static_cast<int>(size_) / 2 - 1; i >= 0; i--) {
            bubble_down(i);
        }
    }
    
    /**
     * @brief Validate heap property (for testing/debugging)
     * 
     * @return true if heap property is satisfied
     */
    bool is_valid() const {
        // Check heap property
        for (std::size_t i = 0; i < size_; i++) {
            std::size_t left = left_child(i);
            std::size_t right = right_child(i);
            
            if (left < size_ && heap_[i].distance > heap_[left].distance) {
                return false;
            }
            if (right < size_ && heap_[i].distance > heap_[right].distance) {
                return false;
            }
        }
        
        // Check position map consistency
        for (std::size_t i = 0; i < size_; i++) {
            auto it = position_map_.find(heap_[i].vertex);
            if (it == position_map_.end() || it->second != static_cast<int>(i)) {
                return false;
            }
        }
        
        return true;
    }
    
    /**
     * @brief Get current capacity of heap
     */
    std::size_t capacity() const {
        return heap_.capacity();
    }
    
    /**
     * @brief Reserve capacity for heap
     * 
     * @param new_capacity New capacity to reserve
     */
    void reserve(std::size_t new_capacity) {
        heap_.reserve(new_capacity);
        position_map_.reserve(new_capacity);
    }
};

} // namespace sssp

#endif // SSSP_BINARY_HEAP_HPP
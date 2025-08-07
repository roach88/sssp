#ifndef SSSP_BLOCK_DATA_STRUCTURE_HPP
#define SSSP_BLOCK_DATA_STRUCTURE_HPP

#include "sssp/types.hpp"
#include "sssp/vertex.hpp"
#include <map>
#include <list>
#include <unordered_map>
#include <vector>
#include <memory>
#include <algorithm>
#include <limits>

namespace sssp {

/**
 * @brief Specialized block-based data structure for frontier management
 * 
 * This implements the data structure from Lemma 3.3 of the paper,
 * designed for efficient management of vertices on the frontier
 * and partitioning problems in the BMSSP algorithm.
 * 
 * The structure maintains two sequences of blocks:
 * - D0: For batch prepend operations (prepended elements)
 * - D1: For insert operations (general insertions)
 * 
 * Each block contains at most M key-value pairs.
 */
class BlockDataStructure {
public:
    using Key = Vertex;
    using Value = Weight;
    using KeyValuePair = std::pair<Key, Value>;
    
private:
    /**
     * @brief Internal block structure
     * 
     * Each block is a linked list of key-value pairs,
     * containing at most M pairs, maintained in sorted order by value.
     */
    struct Block {
        std::list<KeyValuePair> elements;
        Value upper_bound;  // Upper bound for elements in this block (D1 only)
        
        Block() : upper_bound(std::numeric_limits<Value>::infinity()) {}
        explicit Block(Value bound) : upper_bound(bound) {}
        
        bool empty() const { return elements.empty(); }
        std::size_t size() const { return elements.size(); }
        
        // Get minimum value in block
        Value min_value() const {
            return elements.empty() ? std::numeric_limits<Value>::infinity() 
                                    : elements.front().second;
        }
        
        // Get maximum value in block
        Value max_value() const {
            return elements.empty() ? std::numeric_limits<Value>::infinity()
                                    : elements.back().second;
        }
    };
    
    using BlockPtr = std::shared_ptr<Block>;
    
    // Parameters
    std::size_t M_;  // Maximum elements per block
    Value B_;        // Global upper bound
    
    // Two sequences of blocks
    std::list<BlockPtr> D0_;  // Blocks from batch prepend operations
    std::list<BlockPtr> D1_;  // Blocks from insert operations
    
    // Self-balancing BST for D1 block upper bounds (using std::map as Red-Black tree)
    std::map<Value, typename std::list<BlockPtr>::iterator> d1_upper_bounds_;
    
    // Hash map for tracking minimum value per key (for duplicate handling)
    std::unordered_map<Key, Value> key_min_values_;
    
    // Statistics
    std::size_t total_elements_;
    
public:
    /**
     * @brief Initialize the data structure
     * 
     * @param M Maximum number of elements per block (set to 2^((l-1)t) in BMSSP)
     * @param B Global upper bound for values
     */
    void Initialize(std::size_t M, Value B) {
        M_ = std::max(std::size_t(1), M);  // Ensure M is at least 1
        B_ = B;
        
        // Clear existing data
        D0_.clear();
        D1_.clear();
        d1_upper_bounds_.clear();
        key_min_values_.clear();
        total_elements_ = 0;
        
        // Create initial empty block in D1 with upper bound B
        auto initial_block = std::make_shared<Block>(B);
        D1_.push_back(initial_block);
        d1_upper_bounds_[B] = std::prev(D1_.end());
    }
    
    /**
     * @brief Check if the data structure is empty
     */
    bool empty() const {
        return total_elements_ == 0;
    }
    
    /**
     * @brief Get total number of elements
     */
    std::size_t size() const {
        return total_elements_;
    }
    
    /**
     * @brief Get the parameter M
     */
    std::size_t get_M() const {
        return M_;
    }
    
    /**
     * @brief Get the global upper bound B
     */
    Value get_B() const {
        return B_;
    }
    
    /**
     * @brief Get number of blocks in D0
     */
    std::size_t num_d0_blocks() const {
        return D0_.size();
    }
    
    /**
     * @brief Get number of blocks in D1
     */
    std::size_t num_d1_blocks() const {
        return D1_.size();
    }
    
protected:
    /**
     * @brief Find the D1 block for a given value using BST
     * 
     * @param value The value to find block for
     * @return Iterator to the appropriate block in D1
     */
    typename std::list<BlockPtr>::iterator find_d1_block_for_value(Value value) {
        // Use lower_bound to find first block with upper_bound >= value
        auto it = d1_upper_bounds_.lower_bound(value);
        if (it != d1_upper_bounds_.end()) {
            return it->second;
        }
        // Should not happen if data structure is properly initialized
        return D1_.end();
    }
    
    /**
     * @brief Update key tracking for duplicate handling
     * 
     * @param key The key to update
     * @param value The new value
     * @return true if this is a new minimum for the key
     */
    bool update_key_tracking(const Key& key, Value value) {
        auto it = key_min_values_.find(key);
        if (it == key_min_values_.end()) {
            key_min_values_[key] = value;
            return true;
        }
        if (value < it->second) {
            it->second = value;
            return true;
        }
        return false;
    }
    
    /**
     * @brief Check if a key-value pair should be kept
     * 
     * @param key The key
     * @param value The value
     * @return true if this is the minimum value for the key
     */
    bool should_keep_pair(const Key& key, Value value) const {
        auto it = key_min_values_.find(key);
        return it != key_min_values_.end() && it->second == value;
    }
    
    // Friend class for testing
    friend class BlockDataStructureTest;
    
public:
    /**
     * @brief Batch prepend a list of key-value pairs
     * 
     * Inserts a list L where each value is smaller than any current value.
     * Creates new blocks in D0 at the beginning.
     * Handles duplicates by keeping smallest value per key.
     * 
     * Amortized Time: O(|L| * max{1, log(|L|/M)})
     * In BMSSP context: O(log k) per vertex
     * 
     * @param pairs List of key-value pairs to prepend
     */
    void BatchPrepend(const std::vector<KeyValuePair>& pairs) {
        if (pairs.empty()) {
            return;
        }
        
        // Filter pairs to keep only minimum value per key
        std::unordered_map<Key, Value> min_per_key;
        for (const auto& [key, value] : pairs) {
            if (value < B_) {  // Only consider values less than bound
                auto it = min_per_key.find(key);
                if (it == min_per_key.end() || value < it->second) {
                    min_per_key[key] = value;
                }
            }
        }
        
        // Create sorted list of unique pairs
        std::vector<KeyValuePair> sorted_pairs; sorted_pairs.reserve(pairs.size());
        for (const auto& [key, value] : min_per_key) {
            // Only add if this is a new minimum for the key
            if (update_key_tracking(key, value)) {
                sorted_pairs.emplace_back(key, value);
            }
        }
        
        if (sorted_pairs.empty()) {
            return;
        }
        
        // Sort by value
        std::sort(sorted_pairs.begin(), sorted_pairs.end(),
                  [](const KeyValuePair& a, const KeyValuePair& b) {
                      return a.second < b.second;
                  });
        
        // Create blocks for D0
        if (sorted_pairs.size() <= M_) {
            // Single block case
            auto new_block = std::make_shared<Block>();
            for (const auto& pair : sorted_pairs) {
                new_block->elements.push_back(pair);
            }
            D0_.push_front(new_block);
        } else {
            // Multiple blocks case
            std::list<BlockPtr> new_blocks;
            
            for (std::size_t i = 0; i < sorted_pairs.size(); i += M_) {
                auto new_block = std::make_shared<Block>();
                std::size_t end = std::min(i + M_, sorted_pairs.size());
                
                for (std::size_t j = i; j < end; ++j) {
                    new_block->elements.push_back(sorted_pairs[j]);
                }
                
                new_blocks.push_back(new_block);
            }
            
            // Prepend all new blocks to D0 (in reverse order to maintain value ordering)
            for (auto it = new_blocks.rbegin(); it != new_blocks.rend(); ++it) {
                D0_.push_front(*it);
            }
        }
        
        total_elements_ += sorted_pairs.size();
    }
    
    /**
     * @brief Pull M smallest elements from the data structure
     * 
     * Returns a subset of M keys with smallest values and a boundary.
     * Collects from prefix of D0 and D1 blocks.
     * 
     * Amortized Time: O(|S'|)
     * 
     * @return Pair of (elements pulled, upper bound for these elements)
     */
    std::pair<std::vector<KeyValuePair>, Value> Pull() {
        std::vector<KeyValuePair> result;
        result.reserve(M_);
        Value boundary = B_;
        
        if (empty()) {
            return {result, boundary};
        }
        
        // Track blocks to remove after pulling
        std::vector<typename std::list<BlockPtr>::iterator> d0_to_remove; d0_to_remove.reserve(8);
        std::vector<typename std::list<BlockPtr>::iterator> d1_to_remove; d1_to_remove.reserve(8);
        
        // First, collect from D0 blocks
        for (auto d0_it = D0_.begin(); d0_it != D0_.end() && result.size() < M_; ++d0_it) {
            BlockPtr block = *d0_it;
            std::size_t to_take = std::min(M_ - result.size(), block->elements.size());
            
            auto elem_it = block->elements.begin();
            for (std::size_t i = 0; i < to_take; ++i, ++elem_it) {
                result.push_back(*elem_it);
            }
            if (to_take == block->elements.size()) {
                d0_to_remove.push_back(d0_it);
            } else {
                block->elements.erase(block->elements.begin(), elem_it);
                if (result.size() >= M_) {
                    boundary = block->min_value();
                }
                break;
            }
        }
        
        // If we haven't collected enough, continue with D1
        if (result.size() < M_) {
            for (auto d1_it = D1_.begin(); d1_it != D1_.end() && result.size() < M_; ++d1_it) {
                BlockPtr block = *d1_it;
                
                if (block->empty()) {
                    continue;  // Skip empty blocks
                }
                
                std::size_t to_take = std::min(M_ - result.size(), block->elements.size());
                
                auto elem_it = block->elements.begin();
                for (std::size_t i = 0; i < to_take; ++i, ++elem_it) {
                    result.push_back(*elem_it);
                }
                if (to_take == block->elements.size()) {
                    d1_to_remove.push_back(d1_it);
                } else {
                    block->elements.erase(block->elements.begin(), elem_it);
                    if (result.size() >= M_) {
                        boundary = block->min_value();
                    }
                    break;
                }
            }
        }
        
        // Remove fully consumed blocks from D0
        for (auto it : d0_to_remove) {
            D0_.erase(it);
        }
        
        // Remove fully consumed blocks from D1 and update BST
        for (auto it : d1_to_remove) {
            Value old_bound = (*it)->upper_bound;
            d1_upper_bounds_.erase(old_bound);
            D1_.erase(it);
        }
        
        // Update total elements and key tracking
        total_elements_ -= result.size();
        for (const auto& [key, value] : result) {
            key_min_values_.erase(key);
        }
        
        // Set boundary if we didn't pull full M elements
        if (result.size() < M_ && !empty()) {
            // Find next minimum value
            Value next_min = B_;
            
            if (!D0_.empty() && !D0_.front()->empty()) {
                next_min = std::min(next_min, D0_.front()->min_value());
            }
            
            for (const auto& block : D1_) {
                if (!block->empty()) {
                    next_min = std::min(next_min, block->min_value());
                    break;
                }
            }
            
            boundary = next_min;
        } else if (result.size() == M_ && boundary == B_) {
            // We pulled exactly M elements, set boundary to next element value
            if (!D0_.empty() && !D0_.front()->empty()) {
                boundary = D0_.front()->min_value();
            } else {
                for (const auto& block : D1_) {
                    if (!block->empty()) {
                        boundary = block->min_value();
                        break;
                    }
                }
            }
        }
        
        return {result, boundary};
    }
    
    /**
     * @brief Insert or update a key-value pair
     * 
     * If the key exists, only update if new value is smaller.
     * Locates appropriate block using BST on block upper bounds.
     * If block exceeds size M, triggers a Split operation.
     * 
     * Amortized Time: O(max{1, log(N/M)})
     * In BMSSP context: O(t) time
     * 
     * @param key The vertex to insert
     * @param value The distance estimate
     */
    void Insert(const Key& key, Value value) {
        // Only proceed if value is less than global bound B
        if (value >= B_) {
            return;
        }
        
        // Check if we should update this key (only if new value is smaller)
        if (!update_key_tracking(key, value)) {
            return;  // Existing value is smaller, don't insert
        }
        
        // Find appropriate D1 block for this value
        auto block_it = find_d1_block_for_value(value);
        if (block_it == D1_.end()) {
            return;  // Should not happen with proper initialization
        }
        
        BlockPtr block = *block_it;
        
        // Remove any existing occurrence of this key in the block
        auto old_it = std::find_if(block->elements.begin(), block->elements.end(),
                                    [&key](const KeyValuePair& kv) { 
                                        return kv.first == key; 
                                    });
        if (old_it != block->elements.end()) {
            block->elements.erase(old_it);
            total_elements_--;
        }
        auto insert_pos = std::lower_bound(block->elements.begin(), block->elements.end(), value,
                                           [](const KeyValuePair& kv, Value v){ return kv.second < v; });
        block->elements.insert(insert_pos, std::make_pair(key, value));
        total_elements_++;
        
        // Check if block needs splitting (exceeds M elements)
        if (block->size() > M_) {
            split_block(block_it);
        }
    }
    
private:
    /**
     * @brief Split a D1 block that exceeds size M
     * 
     * Splits block into two roughly equal parts by finding median.
     * Updates BST with new block bounds.
     * 
     * @param block_it Iterator to the block in D1 list
     */
    void split_block(typename std::list<BlockPtr>::iterator block_it) {
        BlockPtr block = *block_it;
        if (block->size() <= M_) {
            return;  // No need to split
        }
        
        // Find median position
        std::size_t mid = block->size() / 2;
        auto mid_it = block->elements.begin();
        std::advance(mid_it, mid);
        auto new_block = std::make_shared<Block>();
        new_block->upper_bound = block->upper_bound;
        new_block->elements.splice(new_block->elements.begin(), block->elements,
                                    mid_it, block->elements.end());
        
        // Update upper bound of first block
        Value old_upper = block->upper_bound;
        block->upper_bound = new_block->min_value();
        
        // Update BST: remove old bound, add new bounds
        d1_upper_bounds_.erase(old_upper);
        
        // Insert new block after current block
        auto new_block_it = D1_.insert(std::next(block_it), new_block);
        
        // Update BST with new bounds
        d1_upper_bounds_[block->upper_bound] = block_it;
        d1_upper_bounds_[new_block->upper_bound] = new_block_it;
    }
};

} // namespace sssp

#endif // SSSP_BLOCK_DATA_STRUCTURE_HPP
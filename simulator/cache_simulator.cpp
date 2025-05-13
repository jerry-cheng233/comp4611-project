#include "cache_simulator.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cassert>
#include "../utils.h"

uint32_t currentTime = 0;
const uint32_t mainMemoryAccessTime = 100;

/**
 * @brief Constructs a cache level
 *
 * @param cacheSize Total cache size in bytes
 * @param blockSize Size of each cache block in bytes
 * @param associativity Number of blocks per set (1 means direct-mapped)
 * @param accessTime Access time (constant) for this cache level
 */
CacheLevel::CacheLevel(uint32_t cacheSize, uint32_t blockSize, uint32_t associativity, uint32_t accessTime) {
    config.cacheSize = cacheSize;
    config.blockSize = blockSize;
    config.associativity = associativity;
    config.accessTime = accessTime;
    nextLevel = nullptr;

    config.numBlocks = cacheSize / blockSize;
    config.numSets = config.numBlocks / associativity;
    config.offsetBits = log2(blockSize);
    config.indexBits = log2(config.numSets);

    // Initialise sets and blocks (important to allocate all space on init)
    sets.resize(config.numSets);
    for (auto& set : sets) {
        set.blocks.resize(associativity, CacheBlock(this));
    }
}

/**
 * @brief Extracts the tag bits from an address
 *
 * @param address Memory address
 * @return Tag bits
 */
uint32_t CacheLevel::getTag(uint32_t address) const {
    TODO_IMPLEMENT_ME;
}

/**
 * @brief Extracts the set index bits from an address
 *
 * @param address Memory address
 * @return Set index
 */
uint32_t CacheLevel::getSetIndex(uint32_t address) const {
    TODO_IMPLEMENT_ME;
}

/**
 * @brief Extracts the offset bits from an address
 *
 * @param address Memory address
 * @return Block offset
 */
uint32_t CacheLevel::getOffset(uint32_t address) const {
    return address & ((1 << config.offsetBits) - 1);
}

/**
 * @brief Searches for a block in the cache.
 *
 * @param address Memory address to search for
 * @return Pointer to the block if found, nullptr otherwise
 *  If the block is found, but it is invalid (Check using the valid bit)
 *  then still return nullptr
 */
CacheBlock* CacheLevel::findBlock(uint32_t address) {
    uint32_t tag = getTag(address);
    auto& set = sets[getSetIndex(address)];

    TODO_IMPLEMENT_ME;
}

/**
 * @brief Finds an invalid block (if available), or the least recently used block in a set
 *
 * Note that if there are unused (invalid) blocks, it prioritises returning those.
 *
 * @param setIndex Index of the set to search
 * @return Reference to invalid block, if no invalid block
*   return the reference to the least recently used block
 */
CacheBlock& CacheLevel::findInvalidOrOldestBlock(uint32_t setIndex) {

    auto& set = sets[setIndex];
    CacheBlock* oldestCandidate = &set.blocks[0];
    TODO_IMPLEMENT_ME;

}

/**
 * @brief The high-level method to simulate a read/write action on this level of cache
 *
 * On cache hits, the read/write operation is directly performed in this level.
 *
 * On cache misses, this tries to fetch the data from the next level (or main memory),
 * bring it into this level (using the updateOrInsertBlock method), and then perform the cache hit operation.
 *
 * @param address Memory address to access
 * @param isWrite Whether this is a write operation
 * @return Total access time for this operation
 */
uint32_t CacheLevel::access(uint32_t address, bool isWrite) {
    stats.accesses++;
    uint32_t totalTime = 0;

    CacheBlock* block = findBlock(address);
    if (block) {
        // On hit:
        // - Update the block's LRU status (using updateLRU())
        // - Mark the block dirty if it's a write operation (i.e. isWrite is true)
        // - Add the access time and return
        stats.hits++;
        TODO_IMPLEMENT_ME;
        return totalTime;
    }

    // On miss:

    stats.misses++;

    // Read from the next level using nextlevel->access(,)
    // Remember to accumulate totalTime with the access time of the next level
    if (nextLevel) {
        TODO_IMPLEMENT_ME;
    } else {
        // Access main memory if no next level
        totalTime += mainMemoryAccessTime;
    }

    // Bring the data into this level
    totalTime += updateOrInsertBlock(address, false);

    block = findBlock(address);
    assert(block && "Block should exist after bringing in data");

    // Finally, read/write from/to the brought-in block:
    // - Add the access time (in config.accessTime) to "totalTime"
    // - Update the block's LRU status
    // - Mark the block dirty if it's a write operation
    TODO_IMPLEMENT_ME;

    return totalTime;
}

/**
 * @brief The internal function to update or insert a block in the cache level, handling eviction if necessary
 *
 * If the updated block is already in this level, it will be updated (simulates the writing action).
 * If the block is not present in this level, it will be inserted.
 * This function will always incur access time for this level.
 *
 * For evictions, the non-inclusive write-back/eviction behaviour is implemented:
 * - If eviction is needed, checks if the evicted block is dirty
 * - If dirty, writes back to the next level (or memory)
 * - If clean, only copies to next/lower level if the next/lower level doesn't already have it (note that main memory always has all data)
 *
 * @param address Memory address in the block
 * @param isDirty Whether to mark the block as dirty or clean
 * @return Total access time for this operation
 */
uint32_t CacheLevel::updateOrInsertBlock(uint32_t address, bool isDirty) {
    uint32_t totalTime = 0;

    // NOTE: When implementing this function, you can get inspiration from the access function.
    

    CacheBlock* existingBlock = findBlock(address);
    if (existingBlock) {
        assert((isDirty || !existingBlock->dirty) && "BUG: Existing dirty line can't be updated to become clean");
        // Implement the actions for existing blocks:
        TODO_IMPLEMENT_ME;
    }

    // If the block is not present in this level, it will be inserted.

    // First check if we need to evict a block to make room.
    CacheBlock& targetBlock = findInvalidOrOldestBlock(getSetIndex(address));
    if (targetBlock.valid) {
        // Eviction needed - implement the eviction actions:
        TODO_IMPLEMENT_ME;
    }

    // Now we have a free slot in the cache. Insert the new block:
    TODO_IMPLEMENT_ME;

    return totalTime;
}

/**
 * @brief Initializes the cache simulator with the specified configuration
 *
 * Creates a one or two-level cache hierarchy based on the provided parameters.
 *
 * @param twoLevel Whether to use a two-level cache hierarchy
 * @param l1Size Size of L1 cache in bytes
 * @param l1BlockSize Size of L1 cache blocks in bytes
 * @param l1Associativity Associativity of L1 cache
 * @param l2Size Size of L2 cache in bytes (only used if twoLevel is true)
 * @param l2BlockSize Size of L2 cache blocks in bytes (only used if twoLevel is true)
 * @param l2Associativity Associativity of L2 cache (only used if twoLevel is true)
 */
void CacheSimulator::initCache(bool twoLevel,
                               uint32_t l1Size, uint32_t l1BlockSize, uint32_t l1Associativity,
                               uint32_t l2Size, uint32_t l2BlockSize, uint32_t l2Associativity) {
    totalAccessTime = 0;
    totalAccesses = 0;

    cacheLevels.clear();
    // Important, because we don't want them to be moved and pointers/references invalidated
    cacheLevels.reserve(twoLevel ? 2 : 1);

    cacheLevels.emplace_back(l1Size, l1BlockSize, l1Associativity, 1);

    if (twoLevel) {
        cacheLevels.emplace_back(l2Size, l2BlockSize, l2Associativity, 10);
        cacheLevels[0].setNextLevel(&cacheLevels[1]);
    }
}

/**
 * @brief Simulates a single memory access through the cache hierarchy
 *
 * Delegates to the first level cache to handle the access, which will
 * propagate to lower levels as needed according to the non-inclusive
 * write-back read-allocate write-allocate policy.
 *
 * @param address Memory address to access
 * @param isWrite Whether this is a write operation
 */
void CacheSimulator::accessMemory(uint32_t address, bool isWrite) {
    totalAccesses++;
    uint32_t accessTime = cacheLevels[0].access(address, isWrite);
    totalAccessTime += accessTime;
}

/**
 * @brief Processes a sequence of memory access records through the cache hierarchy
 *
 * Iterates through each memory access record in the trace and simulates
 * the cache access for each one, updating statistics along the way.
 *
 * @param trace Vector of memory access records (address and operation type)
 */
void CacheSimulator::processTrace(const std::vector<MemoryAccessRecord>& trace) {
    for (const auto& record : trace) {
        accessMemory(record.address, record.isWrite);
    }
}

/**
 * @brief Returns the hit rate for the L1 cache
 *
 * @return L1 cache hit rate
 */
double CacheSimulator::getL1HitRate() const {
    assert(!cacheLevels.empty() && "No cache installed");
    return cacheLevels[0].getStats().getHitRate();
}

/**
 * @brief Returns the hit rate for the L2 cache
 *
 * @return L2 cache hit rate
 */
double CacheSimulator::getL2HitRate() const {
    assert(cacheLevels.size() > 1 && "L2 cache not installed");
    return cacheLevels[1].getStats().getHitRate();
}

/**
 * @brief Returns the overall hit rate for the cache hierarchy
 *
 * @return Overall hit rate
 */
double CacheSimulator::getOverallHitRate() const {
    assert(!cacheLevels.empty() && "No cache installed");

    uint64_t totalMisses = 0;
    if (cacheLevels.size() > 1)
        totalMisses = cacheLevels[1].getStats().misses;
    else
        totalMisses = cacheLevels[0].getStats().misses;

    // Overall hit rate = 1 - miss rate
    return totalAccesses > 0 ? 1.0 - (static_cast<double>(totalMisses) / totalAccesses) : -1.0;
}

/**
 * @brief Returns the average memory access time
 *
 * @return Average access time in nanoseconds, or -1 if not available
 */
double CacheSimulator::getAverageAccessTime() const {
    return totalAccesses > 0 ? static_cast<double>(totalAccessTime) / totalAccesses : -1.0;
}

/**
 * @brief Prints detailed statistics about the cache performance
 *
 * Outputs information about cache configuration, hit/miss rates,
 * and memory access times to standard output.
 */
void CacheSimulator::printStats() const {
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "--------- Cache Statistics ---------" << std::endl;

    for (size_t i = 0; i < cacheLevels.size(); i++) {
        auto& level = cacheLevels[i];
        auto& config = level.getConfig();
        auto& stats = level.getStats();

        std::cout << "Level " << i + 1 << ":" << std::endl;
        std::cout << "  " << config.cacheSize / 1024 << "KiB size, " << config.blockSize << "B block size, " << config.associativity << "-way set associative" << std::endl;
        std::cout << "  Accesses/Hits/Misses: " << stats.accesses << "/" << stats.hits << "/" << stats.misses << " (Hit rate: " << stats.getHitRate() * 100 << "%)" << std::endl;
        std::cout << "  Clean/Dirty Evictions: " << stats.cleanEvictions << "/" << stats.dirtyEvictions << std::endl;

        std::cout << std::endl;
    }


    std::cout << "General Statistics:" << std::endl;
    std::cout << "  Overall Hit Rate: " << getOverallHitRate() * 100 << "%" << std::endl;
    std::cout << "  Total Accesses: " << totalAccesses << std::endl;
    std::cout << "  Total Access Time: " << totalAccessTime << " nanoseconds" << std::endl;
    std::cout << "  Average Access Time: " << getAverageAccessTime() << " nanoseconds" << std::endl;
    std::cout << "====================================" << std::endl;
}

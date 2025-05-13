#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <unordered_map>
#include <list>
#include "memory_tracer.h"

// Global time variable for LRU tracking
extern uint32_t currentTime;
extern const uint32_t mainMemoryAccessTime;  // Access time for main memory read/write

// Configuration for a single level of cache
struct CacheLevelConfig {
    uint32_t cacheSize;      // in bytes
    uint32_t blockSize;      // in bytes
    uint32_t associativity;  // 1 for direct mapped, others for n-way set associative
    uint32_t accessTime;     // Access time needed for this level on reading/writing

    // Calculated fields (to be computed in constructor)
    uint32_t numSets;
    uint32_t numBlocks;
    uint32_t indexBits;
    uint32_t offsetBits;
};

// Statistics for a single level of cache
struct CacheLevelStats {
    uint64_t accesses = 0;
    uint64_t hits = 0;
    uint64_t misses = 0;
    uint64_t dirtyEvictions = 0;
    uint64_t cleanEvictions = 0;

    double getHitRate() const {
        return accesses > 0 ? static_cast<double>(hits) / accesses : 0.0;
    }
};

struct CacheBlock;
class CacheLevel;

/**
 * @brief Represents a single level in the cache hierarchy (e.g., the L1 or the L2 cache)
 *
 * Implements a configurable cache level with parametrised size,
 * block size, and associativity. Handles memory accesses, block
 * replacement using LRU policy, and tracks statistics.
 */
class CacheLevel {
public:
    CacheLevelConfig config;
    CacheLevelStats stats;
    CacheLevel* nextLevel;

    struct CacheSet {
        std::vector<CacheBlock> blocks;
    };
    std::vector<CacheSet> sets;

    CacheLevel(uint32_t cacheSize, uint32_t blockSize, uint32_t associativity, uint32_t accessTime);
    void setNextLevel(CacheLevel* next) { nextLevel = next; }

    const CacheLevelStats& getStats() const { return stats; }
    const CacheLevelConfig& getConfig() const { return config; }

    uint32_t getTag(uint32_t address) const;
    uint32_t getSetIndex(uint32_t address) const;
    uint32_t getOffset(uint32_t address) const;

    uint32_t access(uint32_t address, bool isWrite);
    uint32_t updateOrInsertBlock(uint32_t address, bool isDirty);
    CacheBlock* findBlock(uint32_t address);
    CacheBlock& findInvalidOrOldestBlock(uint32_t setIndex);

    friend struct CacheBlock;
};

struct CacheBlock {
    CacheLevel* level;
    uint32_t address = 0;
    bool valid = false;
    bool dirty = false;
    uint32_t lastAccessed = 0;

    CacheBlock(CacheLevel* level) : level(level) {}

    // Updates the last accessed time
    void updateLRU() { lastAccessed = ++currentTime; }

    uint32_t getTag() const { return level->getTag(address); }
    uint32_t getSetIndex() const { return level->getSetIndex(address); }
    uint32_t getOffset() const { return level->getOffset(address); }
};

class CacheSimulator {
public:
    uint64_t totalAccessTime = 0;
    uint64_t totalAccesses = 0;

    std::vector<CacheLevel> cacheLevels;

    void initCache(bool twoLevel,
                  uint32_t l1Size, uint32_t l1BlockSize, uint32_t l1Associativity,
                  uint32_t l2Size = 0, uint32_t l2BlockSize = 0, uint32_t l2Associativity = 0);
    void processTrace(const std::vector<MemoryAccessRecord>& trace);
    void accessMemory(uint32_t address, bool isWrite);

    double getL1HitRate() const;
    double getL2HitRate() const;
    double getOverallHitRate() const;
    uint64_t getTotalAccessTime() const { return totalAccessTime; }
    double getAverageAccessTime() const;

    void printStats() const;

    void resetStats() {
        totalAccessTime = 0;
        totalAccesses = 0;
        for (auto& level : cacheLevels)
            level.stats = CacheLevelStats();
    }
};

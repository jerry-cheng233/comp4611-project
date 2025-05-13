#include <iostream>
#include <cmath>
#include <vector>
#include <sstream>
#include "../simulator/cache_simulator.h"

#define FAIL_IF(condition, ...) \
    if (condition) { \
        errors << "  - " << __VA_ARGS__ << std::endl; \
        passed = false; \
    }

void printTestResult(const std::string& testName, bool passed, const std::string& errorDetails = "") {
    std::cout << testName << ": " << (passed ? "✅ PASSED" : "❌ FAILED") << std::endl;
    if (!passed && !errorDetails.empty()) {
        std::cout << errorDetails;
    }
}

// Test for getTag function
bool testGetTag(std::stringstream& errors) {
    struct TestCase {
        uint32_t cacheSize;
        uint32_t blockSize;
        uint32_t associativity;
        uint32_t address;
        uint32_t expectedTag;
    };

    std::vector<TestCase> testCases = {
        // cacheSize, blockSize, associativity, address, expected tag
        {256, 32, 1, 0x12345678, 0x123456},
        {1024, 64, 2, 0x0, 0x0},
        {1024, 64, 2, 0x00000040, 0x0},
        {1024, 64, 2, 0x00000200, 0x1},
        {1024, 64, 2, 0x00F00000, 0x7800},
        {1024, 64, 2, 0xABCDEF00, 0x55E6F7},
        {4096, 128, 4, 0x12345678, 0x48D15},
        {8192, 64, 8, 0x12345678, 0x48D15},
        {16384, 256, 16, 0xABCDEF00, 0x2AF37B},
    };

    bool passed = true;
    for (const auto& testCase : testCases) {
        CacheLevel cache(testCase.cacheSize, testCase.blockSize, testCase.associativity, 1);
        uint32_t actualTag = cache.getTag(testCase.address);

        FAIL_IF(actualTag != testCase.expectedTag,
            "Failed for config (size=" << testCase.cacheSize << ", blocksize=" << testCase.blockSize
            << ", assoc=" << testCase.associativity << "), address 0x" << std::hex << testCase.address
            << ": Expected tag 0x" << testCase.expectedTag
            << ", got 0x" << actualTag << std::dec)
    }

    return passed;
}

// Test for getSetIndex function
bool testGetSetIndex(std::stringstream& errors) {
    CacheLevel cache(1024, 64, 2, 1);

    std::vector<std::pair<uint32_t, uint32_t>> testCases = {
        {0x00000000, 0},
        {0x00000040, 1},
        {0x00000080, 2},
        {0x000000C0, 3},
        {0x00000100, 4},
        {0x00000140, 5},
        {0x00000180, 6},
        {0x000001C0, 7},
        {0x00000200, 0},
        {0xABCDEF00, 4},
    };

    bool passed = true;
    for (const auto& testCase : testCases) {
        uint32_t address = testCase.first;
        uint32_t expectedIndex = testCase.second;
        uint32_t actualIndex = cache.getSetIndex(address);

        FAIL_IF(actualIndex != expectedIndex,
            "Failed for address 0x" << std::hex << address
            << ": Expected set index " << std::dec << expectedIndex
            << ", got " << actualIndex)
    }

    return passed;
}

// Test for findBlock function
bool testFindBlock(std::stringstream& errors) {
    CacheLevel cache(1024, 64, 2, 1);  // 1KB cache, 64B blocks, 2-way, 8 sets
    bool passed = true;

    CacheBlock* block = cache.findBlock(0x00000000);
    FAIL_IF(block != nullptr, "Block at address 0x00000000 should not be found in empty cache")

    // Insert some blocks
    cache.updateOrInsertBlock(0x00000000, false);  // Set 0, Block 0
    cache.updateOrInsertBlock(0x00000040, false);  // Set 1, Block 0
    cache.updateOrInsertBlock(0x00000080, false);  // Set 2, Block 0

    // Test finding inserted blocks
    block = cache.findBlock(0x00000000);
    FAIL_IF(!block, "Failed to find block at address 0x00000000 after insertion")

    block = cache.findBlock(0x00000040);
    FAIL_IF(!block, "Failed to find block at address 0x00000040 after insertion")

    block = cache.findBlock(0x00000080);
    FAIL_IF(!block, "Failed to find block at address 0x00000080 after insertion")

    // Test addresses with same tag but different offset (should still find the block)
    block = cache.findBlock(0x00000001);
    FAIL_IF(!block, "Failed to find block at address 0x00000001 (same as 0x00000000 except offset)")

    block = cache.findBlock(0x0000003F);
    FAIL_IF(!block, "Failed to find block at address 0x0000003F (same as 0x00000000 except offset)")

    // Test address in uninitialised set
    block = cache.findBlock(0x000000C0);  // Set 3, not inserted
    FAIL_IF(block != nullptr, "Block at address 0x000000C0 should not be found (set not used)")

    // Test address with same set but different tag
    block = cache.findBlock(0x00000200);  // Maps to set 0 but different tag
    FAIL_IF(block != nullptr, "Block at address 0x00000200 should not be found (different tag)")

    // Test eviction
    cache.updateOrInsertBlock(0x00000200, false);  // Set 0, Block 1
    cache.updateOrInsertBlock(0x00000400, false);  // Set 0, should evict Block 0 (0x00000000)

    block = cache.findBlock(0x00000000);
    FAIL_IF(block != nullptr, "Block at address 0x00000000 should not be found after eviction")

    block = cache.findBlock(0x00000200);
    FAIL_IF(!block, "Failed to find block at address 0x00000200 after insertion")

    block = cache.findBlock(0x00000400);
    FAIL_IF(!block, "Failed to find block at address 0x00000400 after insertion")

    return passed;
}

// Test for findInvalidOrOldestBlock function
bool testFindInvalidOrOldestBlock(std::stringstream& errors) {
    CacheLevel cache(1024, 64, 4, 1);  // 1KB cache, 64B blocks, 4-way, 4 sets
    bool passed = true;

    // Test 1: Should return invalid block when available
    uint32_t setIndex = 0;
    CacheBlock* block = &cache.findInvalidOrOldestBlock(setIndex);
    FAIL_IF(block->valid, "Should return an invalid block in fresh cache")

    // Mark blocks in set 0 as valid and set up LRU
    for (int i = 0; i < 3; i++)
        cache.updateOrInsertBlock(0x00000000 + (i * 0x1000), false);

    // Should still have one invalid block in set 0
    block = &cache.findInvalidOrOldestBlock(setIndex);
    FAIL_IF(block->valid, "Should return the remaining invalid block even with valid blocks present")

    // Now fill the last block in set 0
    cache.updateOrInsertBlock(0x00000000 + 0x3000, false);

    // Test 2: Should return oldest block when all blocks are valid
    // Access blocks in reverse order to set up LRU
    for (int i = 3; i >= 0; i--)
        cache.access(0x00000000 + (i * 0x1000), false);

    // Block at 0x3000 should now be the oldest
    block = &cache.findInvalidOrOldestBlock(setIndex);
    FAIL_IF(!block->valid, "Block should be valid")
    FAIL_IF(cache.getTag(block->address) != cache.getTag(0x3000),
                   "Oldest block should be at 0x3000, but found " << std::hex << block->address)

    cache.access(0x00000000 + 0x3000, false);
    block = &cache.findInvalidOrOldestBlock(setIndex);
    FAIL_IF(!block->valid, "Block should be valid")
    FAIL_IF(cache.getTag(block->address) != cache.getTag(0x2000),
                   "Oldest block should be at 0x2000, but found " << std::hex << block->address)

    // Since set 1 is still empty, should return an invalid block
    setIndex = 1;
    block = &cache.findInvalidOrOldestBlock(setIndex);
    FAIL_IF(block->valid, "Should return an invalid block in fresh set")

    return passed;
}

// Test for findBlock and updateOrInsertBlock functions
bool testFindAndUpdateBlock(std::stringstream& errors) {
    CacheLevel cache(1024, 64, 2, 1);
    bool passed = true;

    cache.updateOrInsertBlock(0x00000000, false);
    CacheBlock* block = cache.findBlock(0x00000000);

    FAIL_IF(!block, "Failed to find block at address 0x00000000")
    FAIL_IF(block && !block->valid, "Block should be valid")
    FAIL_IF(block && block->dirty, "Block should not be dirty")

    cache.updateOrInsertBlock(0x00000200, true);
    block = cache.findBlock(0x00000200);

    FAIL_IF(!block, "Failed to find block at address 0x00000200")
    FAIL_IF(block && !block->valid, "Block should be valid")
    FAIL_IF(block && !block->dirty, "Block should be dirty")

    block = cache.findBlock(0x00001000);

    FAIL_IF(block != nullptr, "Should not find block at address 0x00001000")

    return passed;
}

// Test for LRU replacement policy
bool testLRUReplacement(std::stringstream& errors) {
    CacheLevel cache(256, 64, 2, 1);
    bool passed = true;

    cache.updateOrInsertBlock(0x00000000, false);  // Set 0, Block 0
    cache.updateOrInsertBlock(0x00000200, false);  // Set 0, Block 1

    uint32_t hitAccessTime = cache.access(0x00000000, false);
    FAIL_IF(hitAccessTime != 1, "Expected hit access time of 1, got " << hitAccessTime)

    cache.updateOrInsertBlock(0x00000400, false);  // Set 0, should replace Block 1 (least recently used)

    CacheBlock* block = cache.findBlock(0x00000000);
    FAIL_IF(block == nullptr, "Block at address 0x00000000 should still be in cache")

    block = cache.findBlock(0x00000200);
    FAIL_IF(block != nullptr, "Block at address 0x00000200 should have been evicted")

    block = cache.findBlock(0x00000400);
    FAIL_IF(block == nullptr, "Block at address 0x00000400 should be in cache")

    return passed;
}

// Test for complex LRU replacement patterns
bool testComplexLRUReplacement(std::stringstream& errors) {
    CacheLevel cache(256, 64, 4, 1);
    bool passed = true;

    // All in set 0
    cache.updateOrInsertBlock(0x00000000, false);
    cache.updateOrInsertBlock(0x00000200, false);
    cache.updateOrInsertBlock(0x00000400, false);
    cache.updateOrInsertBlock(0x00000600, false);

    // Update LRU
    cache.access(0x00000600, false);
    cache.access(0x00000400, false);
    cache.access(0x00000200, false);

    // Insert a new block to set 0, should evict the oldest (0x00000000)
    cache.updateOrInsertBlock(0x00000800, false);

    CacheBlock* block = cache.findBlock(0x00000000);
    FAIL_IF(block != nullptr, "Block at address 0x00000000 should have been evicted")

    block = cache.findBlock(0x00000200);
    FAIL_IF(block == nullptr, "Block at address 0x00000200 should still be in cache")

    cache.access(0x00000200, false);
    cache.access(0x00000600, false);
    cache.access(0x00000800, false);
    // 0x00000400 is now the least recently used

    // Insert new block, should evict 0x00000400
    cache.updateOrInsertBlock(0x00000A00, false);

    block = cache.findBlock(0x00000400);
    FAIL_IF(block != nullptr, "Block at address 0x00000400 should have been evicted")

    return passed;
}

// Test for the access function and hit/miss counting
bool testAccess(std::stringstream& errors) {
    CacheLevel cache(1024, 64, 2, 1);
    bool passed = true;

    uint32_t accessTime = cache.access(0x00000000, false);

    FAIL_IF(accessTime != mainMemoryAccessTime + 1 + 1,
        "Expected access time " << mainMemoryAccessTime + 1 + 1 << ", got " << accessTime)

    const CacheLevelStats& stats = cache.getStats();
    FAIL_IF(stats.misses != 1, "First access should be a miss")

    cache.updateOrInsertBlock(0x00000000, false);

    accessTime = cache.access(0x00000000, false);

    FAIL_IF(stats.hits < 1, "Second access should be a hit")
    FAIL_IF(stats.accesses != 2, "Expected 2 accesses, got " << stats.accesses)
    FAIL_IF(stats.hits != 1, "Expected 1 hit, got " << stats.hits)
    FAIL_IF(stats.misses != 1, "Expected 1 miss, got " << stats.misses)

    return passed;
}

// Test for read and write operations
bool testReadWriteAccess(std::stringstream& errors) {
    CacheLevel cache(1024, 64, 2, 1);
    bool passed = true;

    // First read (miss)
    uint32_t missAccessTime = cache.access(0x00000000, false);

    auto expected = mainMemoryAccessTime + 1 + 1;
    FAIL_IF(missAccessTime != expected, "Expected miss access time of " << expected << ", got " << missAccessTime)

    // First write to same address (hit, but makes block dirty)
    uint32_t hitAccessTime = cache.access(0x00000000, true);
    FAIL_IF(hitAccessTime != 1, "Expected hit access time of 1, got " << hitAccessTime)

    CacheBlock* block = cache.findBlock(0x00000000);
    FAIL_IF(!block || !block->dirty, "Block should be marked dirty after write")

    // Force eviction of dirty block by filling set
    cache.access(0x00000200, false);
    cache.access(0x00000600, false);

    block = cache.findBlock(0x00000000);
    FAIL_IF(block != nullptr, "Block at 0x00000000 should have been evicted")

    const CacheLevelStats& stats = cache.getStats();
    FAIL_IF(stats.accesses != 4, "Expected 4 accesses, got " << stats.accesses)

    return passed;
}

// Test for dirty block eviction and writebacks
bool testEviction(std::stringstream& errors) {
    CacheLevel cache(256, 64, 1, 1); // Direct-mapped, 4 blocks total
    bool passed = true;

    cache.updateOrInsertBlock(0x00000000, false);  // Set 0, clean
    cache.updateOrInsertBlock(0x00000040, false);  // Set 1, clean
    cache.updateOrInsertBlock(0x00000080, false);  // Set 2, clean
    cache.updateOrInsertBlock(0x000000C0, true);   // Set 3, dirty

    // Evict a clean block
    uint32_t cleanEvictUpsertTime = cache.updateOrInsertBlock(0x00000100, false);  // Set 0

    FAIL_IF(cleanEvictUpsertTime != 1, "Expected clean evict upsert time of 1, got " << cleanEvictUpsertTime)

    CacheBlock* block = cache.findBlock(0x00000000);
    FAIL_IF(block != nullptr, "Block at address 0x00000000 should have been evicted")

    const CacheLevelStats& stats1 = cache.getStats();
    uint32_t initialDirtyEvictions = stats1.dirtyEvictions;

    // Evict dirty block
    uint32_t dirtyEvictUpsertTime = cache.updateOrInsertBlock(0x000001C0, false);  // Set 3

    FAIL_IF(dirtyEvictUpsertTime != 1+mainMemoryAccessTime+1, "Expected dirty evict upsert time of " << 1+mainMemoryAccessTime+1 << ", got " << dirtyEvictUpsertTime)

    block = cache.findBlock(0x000000C0);
    FAIL_IF(block != nullptr, "Block at address 0x000000C0 should have been evicted")

    const CacheLevelStats& stats2 = cache.getStats();
    FAIL_IF(stats2.dirtyEvictions <= initialDirtyEvictions,
        "Expected dirty evictions count to increase after evicting dirty block")

    return passed;
}

// Test for multi-level cache interactions
bool testMultiLevelCache(std::stringstream& errors) {
    CacheSimulator simulator;
    bool passed = true;

    // Initialize with two levels (L1 has 2 sets, L2 has 4)
    simulator.initCache(true, 256, 64, 2, 1024, 64, 4);

    // First access - miss in both levels
    simulator.accessMemory(0x00000000, false);

    // Verify that the block is in L1 after the access
    CacheBlock* blockInL1 = simulator.cacheLevels[0].findBlock(0x00000000);
    FAIL_IF(blockInL1 == nullptr, "Block 0x00000000 should be in L1 after first access")

    // Also verify that it's in L2
    CacheBlock* blockInL2 = simulator.cacheLevels[1].findBlock(0x00000000);
    FAIL_IF(blockInL2 == nullptr, "Block 0x00000000 should be in L2 after first access")

    // Check the stats for the first access
    FAIL_IF(simulator.cacheLevels[0].getStats().misses != 1,
        "Expected 1 miss in L1 for first access, got " << simulator.cacheLevels[0].getStats().misses)
    FAIL_IF(simulator.cacheLevels[1].getStats().misses != 1,
        "Expected 1 miss in L2 for first access, got " << simulator.cacheLevels[1].getStats().misses)

    // Get initial average time
    double initialAvgTime = simulator.getAverageAccessTime();

    // Second access - hit in L1
    simulator.accessMemory(0x00000000, false);

    FAIL_IF(simulator.cacheLevels[0].getStats().hits != 1,
        "Expected 1 hit in L1 for second access, got " << simulator.cacheLevels[0].getStats().hits)

    // Verify that second access was faster
    double secondAvgTime = simulator.getAverageAccessTime();
    FAIL_IF(secondAvgTime >= initialAvgTime,
        "Expected second access to lower average time, initial: " << initialAvgTime
        << ", after hit: " << secondAvgTime)

    double l1HitRate = simulator.getL1HitRate();
    FAIL_IF(std::abs(l1HitRate - 0.5) > 0.01,
        "Expected L1 hit rate 0.5, got " << l1HitRate)

    // Access addresses to evict from L1 but keep in L2
    // Each set in L1 can hold 2 blocks, L2 can hold 4
    simulator.accessMemory(0x00001000, false);
    simulator.accessMemory(0x00001100, false);

    // Verify that the original block is evicted from L1
    blockInL1 = simulator.cacheLevels[0].findBlock(0x00000000);
    FAIL_IF(blockInL1 != nullptr, "Block 0x00000000 should have been evicted from L1")

    // But still present in L2
    blockInL2 = simulator.cacheLevels[1].findBlock(0x00000000);
    FAIL_IF(blockInL2 == nullptr, "Block 0x00000000 should still be in L2")

    // Access original address again - should be L1 miss, L2 hit
    simulator.accessMemory(0x00000000, false);

    // Verify L2 hit stats
    FAIL_IF(simulator.cacheLevels[1].getStats().hits != 1,
        "Expected 1 hit in L2, got " << simulator.cacheLevels[1].getStats().hits)

    double l2HitRate = simulator.getL2HitRate();
    FAIL_IF(l2HitRate <= 0,
        "L2 hit rate should be positive, got " << l2HitRate)

    return passed;
}

// Test fully associative caches
bool testFullyAssociativeCache(std::stringstream& errors) {
    CacheLevel cache(256, 64, 4, 1);  // 4 blocks total, fully associative (all in one set)
    bool passed = true;

    for (int i = 0; i < 4; i++) {
        uint32_t addr = i * 0x1000;
        cache.updateOrInsertBlock(addr, false);
    }

    for (int i = 0; i < 4; i++) {
        uint32_t addr = i * 0x1000;
        CacheBlock* block = cache.findBlock(addr);
        FAIL_IF(block == nullptr, "Block at address 0x" << std::hex << addr << " should be in cache" << std::dec)
    }

    // Set up LRU
    cache.access(0x0000, false);
    cache.access(0x2000, false);
    cache.access(0x3000, false);
    // 0x1000 is now least recently used

    // Add a new block, should evict 0x1000
    cache.updateOrInsertBlock(0x4000, false);

    CacheBlock* block = cache.findBlock(0x1000);
    FAIL_IF(block != nullptr, "Block at address 0x1000 should have been evicted")

    block = cache.findBlock(0x4000);
    FAIL_IF(block == nullptr, "Block at address 0x4000 should be in cache")

    return passed;
}

// Test for the complete memory access simulation
bool testMemoryAccess(std::stringstream& errors) {
    CacheSimulator simulator;
    bool passed = true;

    simulator.initCache(true, 1024, 64, 2, 2048, 64, 4);

    simulator.accessMemory(0x00000000, false);
    double averageAccessTime = simulator.getAverageAccessTime();
    double expectedTime = mainMemoryAccessTime + 10 + 10 + 1 + 1;
    FAIL_IF(std::abs(averageAccessTime - expectedTime) > 0.01,
        "Expected average access time " << expectedTime << ", got " << averageAccessTime)

    simulator.accessMemory(0x00000000, false);
    double hitRate = simulator.getL1HitRate();
    double expectedHitRate = 0.5;  // 1 hit out of 2 accesses
    FAIL_IF(std::abs(hitRate - expectedHitRate) > 0.01,
        "Expected L1 hit rate " << expectedHitRate << ", got " << hitRate)

    double overallHitRate = simulator.getOverallHitRate();
    double expectedOverallHitRate = 0.5;  // 1 hit out of 2 accesses
    FAIL_IF(std::abs(overallHitRate - expectedOverallHitRate) > 0.01,
        "Expected overall hit rate " << expectedOverallHitRate << ", got " << overallHitRate)

    return passed;
}

// Test Scenario 1 from README
bool testScenario1(std::stringstream& errors) {
    bool passed = true;
    CacheSimulator simulator;
    simulator.initCache(false, 1024, 64, 1);

    simulator.accessMemory(0x1234, false);

    uint32_t expectedTime = 102;
    FAIL_IF(simulator.getTotalAccessTime() != expectedTime,
        "Expected access time of " << expectedTime << "ns for Scenario 1, got " << simulator.getTotalAccessTime() << "ns")

    const CacheLevelStats& l1Stats = simulator.cacheLevels[0].getStats();
    FAIL_IF(l1Stats.misses != 1, "Expected 1 miss in L1 for Scenario 1, got " << l1Stats.misses)
    FAIL_IF(l1Stats.accesses != 1, "Expected 1 access in L1 for Scenario 1, got " << l1Stats.accesses)

    return passed;
}

// Test Scenario 2 from README
bool testScenario2(std::stringstream& errors) {
    bool passed = true;
    CacheSimulator simulator;
    simulator.initCache(true, 64, 64, 1, 64, 64, 1);

    // Set up
    simulator.cacheLevels[0].updateOrInsertBlock(0x5678, false);
    simulator.cacheLevels[1].updateOrInsertBlock(0x1234, false);

    simulator.resetStats();

    simulator.accessMemory(0x1234, false);

    uint32_t expectedTime = 23;
    FAIL_IF(simulator.getTotalAccessTime() != expectedTime,
        "Expected access time of " << expectedTime << "ns for Scenario 2, got " << simulator.getTotalAccessTime() << "ns")

    CacheBlock* blockInL1 = simulator.cacheLevels[0].findBlock(0x1234);
    CacheBlock* blockInL2 = simulator.cacheLevels[1].findBlock(0x5678);
    FAIL_IF(blockInL1 == nullptr, "Block 0x1234 should be in L1 after access")
    FAIL_IF(blockInL2 == nullptr, "Block 0x5678 should be in L2 after eviction from L1")

    return passed;
}

// Test Scenario 3 from README
bool testScenario3(std::stringstream& errors) {
    bool passed = true;
    CacheSimulator simulator;
    simulator.initCache(true, 64, 64, 1, 64, 64, 1);

    // Set up
    simulator.cacheLevels[0].updateOrInsertBlock(0x5678, true);
    simulator.cacheLevels[1].updateOrInsertBlock(0x1234, true);

    simulator.resetStats();

    simulator.accessMemory(0x0000, true);

    uint32_t expectedTime = 243;
    FAIL_IF(simulator.getTotalAccessTime() != expectedTime,
        "Expected access time of " << expectedTime << "ns for Scenario 3, got " << simulator.getTotalAccessTime() << "ns")

    CacheBlock* blockInL1 = simulator.cacheLevels[0].findBlock(0x0000);
    CacheBlock* blockInL2 = simulator.cacheLevels[1].findBlock(0x5678);
    FAIL_IF(blockInL1 == nullptr, "Block 0x0000 should be in L1 after access")
    FAIL_IF(!blockInL1 || !blockInL1->dirty, "Block 0x0000 in L1 should be dirty after write operation")
    FAIL_IF(blockInL2 == nullptr, "Block 0x5678 should be in L2 after eviction from L1")
    FAIL_IF(!blockInL2 || !blockInL2->dirty, "Block 0x5678 in L2 should be dirty after eviction from L1")

    return passed;
}

int main() {
    std::cout << "Running cache simulator unit tests..." << std::endl;

    int passedTests = 0;
    int totalTests = 16;

    std::stringstream errors;

    bool tagTestPassed = testGetTag(errors);
    printTestResult("getTag Test", tagTestPassed, errors.str());
    if (tagTestPassed) passedTests++;

    errors.str("");
    bool indexTestPassed = testGetSetIndex(errors);
    printTestResult("getSetIndex Test", indexTestPassed, errors.str());
    if (indexTestPassed) passedTests++;

    errors.str("");
    bool findBlockTestPassed = testFindBlock(errors);
    printTestResult("findBlock Test", findBlockTestPassed, errors.str());
    if (findBlockTestPassed) passedTests++;

    errors.str("");
    bool findInvalidOrOldestBlockTestPassed = testFindInvalidOrOldestBlock(errors);
    printTestResult("findInvalidOrOldestBlock Test", findInvalidOrOldestBlockTestPassed, errors.str());
    if (findInvalidOrOldestBlockTestPassed) passedTests++;

    errors.str("");
    bool findAndUpdateBlockTestPassed = testFindAndUpdateBlock(errors);
    printTestResult("findBlock and updateOrInsertBlock Test", findAndUpdateBlockTestPassed, errors.str());
    if (findAndUpdateBlockTestPassed) passedTests++;

    errors.str("");
    bool lruTestPassed = testLRUReplacement(errors);
    printTestResult("LRU Replacement Test", lruTestPassed, errors.str());
    if (lruTestPassed) passedTests++;

    errors.str("");
    bool complexLruTestPassed = testComplexLRUReplacement(errors);
    printTestResult("Complex LRU Replacement Test", complexLruTestPassed, errors.str());
    if (complexLruTestPassed) passedTests++;

    errors.str("");
    bool readWriteTestPassed = testReadWriteAccess(errors);
    printTestResult("Read/Write Access Test", readWriteTestPassed, errors.str());
    if (readWriteTestPassed) passedTests++;

    errors.str("");
    bool accessTestPassed = testAccess(errors);
    printTestResult("access Test", accessTestPassed, errors.str());
    if (accessTestPassed) passedTests++;

    errors.str("");
    bool evictionTestPassed = testEviction(errors);
    printTestResult("Block Eviction Test", evictionTestPassed, errors.str());
    if (evictionTestPassed) passedTests++;

    errors.str("");
    bool multiLevelTestPassed = testMultiLevelCache(errors);
    printTestResult("Multi-Level Cache Test", multiLevelTestPassed, errors.str());
    if (multiLevelTestPassed) passedTests++;

    errors.str("");
    bool fullyAssociativeTestPassed = testFullyAssociativeCache(errors);
    printTestResult("Fully Associative Cache Test", fullyAssociativeTestPassed, errors.str());
    if (fullyAssociativeTestPassed) passedTests++;

    errors.str("");
    bool memoryAccessTestPassed = testMemoryAccess(errors);
    printTestResult("accessMemory Test", memoryAccessTestPassed, errors.str());
    if (memoryAccessTestPassed) passedTests++;

    errors.str("");
    bool scenario1TestPassed = testScenario1(errors);
    printTestResult("Scenario 1 from README Test", scenario1TestPassed, errors.str());
    if (scenario1TestPassed) passedTests++;

    errors.str("");
    bool scenario2TestPassed = testScenario2(errors);
    printTestResult("Scenario 2 from README Test", scenario2TestPassed, errors.str());
    if (scenario2TestPassed) passedTests++;

    errors.str("");
    bool scenario3TestPassed = testScenario3(errors);
    printTestResult("Scenario 3 from README Test", scenario3TestPassed, errors.str());
    if (scenario3TestPassed) passedTests++;

    std::cout << std::endl;
    auto emoji = passedTests == totalTests ? "✅" : "❌";
    std::cout << "Summary: " << passedTests << " out of " << totalTests << " tests passed. " << emoji << std::endl;
    std::cout << "NOTE: Unit tests are non-exhaustive. Passing all tests doesn't 100% mean your implementation is correct." << std::endl;

    return passedTests == totalTests ? 0 : 1;
}
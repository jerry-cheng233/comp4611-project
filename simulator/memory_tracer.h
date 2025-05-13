#pragma once
#include <vector>
#include <string>
#include <cstdint>

struct MemoryAccessRecord {
    uint32_t timestamp;
    uint32_t address;
    bool isWrite;
};

class MemoryTracer {
public:
    uint32_t currentTimestamp = 0;
    std::vector<MemoryAccessRecord> trace;

    void recordAccess(void* addr, bool isWrite);
    void saveTrace(const std::string& filename);
};

#include "memory_tracer.h"
#include <fstream>
#include <bitset>  // Windows needs this

void MemoryTracer::recordAccess(void* addr, bool isWrite) {
    // 32-bit addresses are enough for us
    MemoryAccessRecord record = {currentTimestamp++, static_cast<uint32_t>(reinterpret_cast<uintptr_t>(addr)), isWrite};
    trace.push_back(record);
}

void MemoryTracer::saveTrace(const std::string& filename) {
    std::ofstream outFile(filename);

    if (!outFile) {
        throw std::runtime_error("Could not open file for writing: " + filename);
    }

    for (const auto& record : trace) {
        outFile << record.timestamp << "\t"
                << std::hex << "0x" << record.address << " "
                << "0b" << std::bitset<32>(record.address) << std::dec << " "
                << (record.isWrite ? "W" : "R") << "\n";
    }

    outFile.close();
}

#include <iostream>
#include <string>
#include "simulator/memory_tracer.h"
#include "simulator/cache_simulator.h"
#include <cstdint>
#include "utils.h"

extern void simpleArrayAccess(MemoryTracer& tracer);
extern void simpleListAccess(MemoryTracer& tracer);
extern void stridedAccessA(MemoryTracer& tracer);
extern void strideAccessB(MemoryTracer& tracer);
extern void naiveImpl(MemoryTracer& tracer);
extern void tiledImplConfigurable(MemoryTracer& tracer, uint32_t num_tiles);

void runTask1() {
    std::cout << "Running Task 1: Arrays and Linked Lists\n";

    // Generate memory traces
    MemoryTracer tracer1;
    simpleArrayAccess(tracer1);
    tracer1.saveTrace("array_trace.txt");
    std::cout << "Array trace saved to array_trace.txt" << std::endl;

    MemoryTracer tracer2;
    simpleListAccess(tracer2);
    tracer2.saveTrace("list_trace.txt");
    std::cout << "Linked list trace saved to list_trace.txt" << std::endl;

    // Simulate with different cache configurations
    CacheSimulator simulator;

    // Configuration 1: 32KiB L1-only cache, 32 bytes block size, direct mapped
    std::cout << "\nSimulating with 32KiB L1-only cache, 32 bytes block size, direct mapped:\n";
    simulator.initCache(false, 32 * 1024, 32, 1);

    std::cout << "\nArray Workload:\n";
    simulator.processTrace(tracer1.trace);
    simulator.printStats();

    // Reset simulator
    simulator.initCache(false, 32 * 1024, 32, 1);

    std::cout << "\nLinked List Workload:\n";
    simulator.processTrace(tracer2.trace);
    simulator.printStats();

    // Configuration 2: Two-level cache
    // L1: 32KiB, 32 bytes block size, 4-way set associative
    // L2: 64KiB, 64 bytes block size, 8-way set associative
    std::cout << "\nSimulating with two-level cache:" << std::endl;
    simulator.initCache(true, 32 * 1024, 32, 4, 64 * 1024, 64, 8);

    std::cout << "\nArray Workload:" << std::endl;
    simulator.processTrace(tracer1.trace);
    simulator.printStats();

    // Reset simulator
    simulator.initCache(true, 32 * 1024, 32, 4, 64 * 1024, 64, 8);

    std::cout << "\nLinked List Workload:\n";
    simulator.processTrace(tracer2.trace);
    simulator.printStats();
}

void runTask2() {
    std::cout << "Running Task 2: Steps and Set Associativity\n";

    // Generate memory traces
    MemoryTracer tracer1;
    stridedAccessA(tracer1);
    tracer1.saveTrace("strideA_trace.txt");
    std::cout << "Stride A trace saved to strideA_trace.txt\n";

    MemoryTracer tracer2;
    strideAccessB(tracer2);
    tracer2.saveTrace("strideB_trace.txt");
    std::cout << "Stride B trace saved to strideB_trace.txt\n";

    // Simulate with 4-way set associative cache
    CacheSimulator simulator;

    std::cout << "\nSimulating with 32KiB L1-only cache, 64 bytes block size, 4-way set associative:\n";

    std::cout << "\nStride A Workload:\n";
    simulator.initCache(false, 32 * 1024, 64, 4);
    simulator.processTrace(tracer1.trace);
    simulator.printStats();
    uint64_t strideATime = simulator.getTotalAccessTime();

    std::cout << "\nStride B Workload:\n";
    simulator.initCache(false, 32 * 1024, 64, 4);
    simulator.processTrace(tracer2.trace);
    simulator.printStats();
    uint64_t strideBTime = simulator.getTotalAccessTime();

    // Determine which workload is slower (higher total access time)
    bool isStrideASlower = strideATime > strideBTime;
    std::string slowerWorkload = isStrideASlower ? "Stride A" : "Stride B";

    std::cout << "\nPerformance comparison:" << std::endl;
    std::cout << "  Stride A - Total Access Time: " << strideATime << " ns" << std::endl;
    std::cout << "  Stride B - Total Access Time: " << strideBTime << " ns" << std::endl;
    std::cout << "  Slower workload: " << slowerWorkload << std::endl;

    std::cout << "\nImproved configuration for the slower workload:\n";

    // ---------------------------------------------
    // Please call simulator.initCache with the improved configuration here:
    TODO_IMPLEMENT_ME;
    // ---------------------------------------------

    std::cout << "\n" << slowerWorkload << " Workload with improved configuration:\n";
    if (isStrideASlower) {
        simulator.processTrace(tracer1.trace);
    } else {
        simulator.processTrace(tracer2.trace);
    }
    simulator.printStats();
}

void runTask3() {
    std::cout << "Running Task 3: Cache-Efficient Code\n";

    // Generate memory traces
    MemoryTracer tracerNaive;
    naiveImpl(tracerNaive);
    // tracerNaive.saveTrace("matrix_vector_naive_trace.txt");

    // Generate traces for different tiling sizes
    MemoryTracer tracerTiled2;
    tiledImplConfigurable(tracerTiled2, 2);
    // tracerTiled2.saveTrace("matrix_vector_tiled2_trace.txt");

    MemoryTracer tracerTiled4;
    tiledImplConfigurable(tracerTiled4, 4);
    // tracerTiled4.saveTrace("matrix_vector_tiled4_trace.txt");

    MemoryTracer tracerTiled8;
    tiledImplConfigurable(tracerTiled8, 8);
    // tracerTiled8.saveTrace("matrix_vector_tiled8_trace.txt");

    MemoryTracer tracerTiled16;
    tiledImplConfigurable(tracerTiled16, 16);
    // tracerTiled16.saveTrace("matrix_vector_tiled16_trace.txt");

    // Simulate with L1-only 4-way set associative cache
    CacheSimulator simulator;

    std::cout << "\nSimulating with 32KiB L1-only cache, 64 bytes block size, 4-way set associative:\n";

    // Naive implementation
    simulator.initCache(false, 32 * 1024, 64, 4);
    std::cout << "\nNaive Implementation:\n";
    simulator.processTrace(tracerNaive.trace);
    simulator.printStats();

    // Tiled implementations
    simulator.initCache(false, 32 * 1024, 64, 4);
    std::cout << "\nTiled Implementation (2 tiles):\n";
    simulator.processTrace(tracerTiled2.trace);
    simulator.printStats();

    simulator.initCache(false, 32 * 1024, 64, 4);
    std::cout << "\nTiled Implementation (4 tiles):\n";
    simulator.processTrace(tracerTiled4.trace);
    simulator.printStats();

    simulator.initCache(false, 32 * 1024, 64, 4);
    std::cout << "\nTiled Implementation (8 tiles):\n";
    simulator.processTrace(tracerTiled8.trace);
    simulator.printStats();

    simulator.initCache(false, 32 * 1024, 64, 4);
    std::cout << "\nTiled Implementation (16 tiles):\n";
    simulator.processTrace(tracerTiled16.trace);
    simulator.printStats();
}

void showMenu() {
    std::cout << "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
    std::cout << "Cache Simulator - Main Menu\n";
    std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
    std::cout << "1. Run Task 1: Arrays and Linked Lists\n";
    std::cout << "2. Run Task 2: Steps and Set Associativity\n";
    std::cout << "3. Run Task 3: Cache-Efficient Code (OPTIONAL, NOT GRADED)\n";
    std::cout << "Others -> Exit\n";
    std::cout << "Enter your choice: ";
}

int main() {
    int choice;

    do {
        showMenu();
        if (!(std::cin >> choice) || choice < 1 || choice > 3) {
            std::cout << "Exiting...\n";
            break;
        }

        switch (choice) {
            case 1:
                runTask1();
                break;
            case 2:
                runTask2();
                break;
            case 3:
                runTask3();
                break;
        }
    } while (true);

    return 0;
}

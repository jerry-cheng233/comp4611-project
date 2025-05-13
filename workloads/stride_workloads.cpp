#include "../simulator/memory_tracer.h"
#include <cstdint>

static void strideAccessHelper(MemoryTracer& tracer, size_t stride, size_t iterations) {
    const size_t SIZE = 16384;
    uint32_t* arr = new uint32_t[SIZE];

    // Setup: Initialize array
    for (size_t i = 0; i < SIZE; i++) {
        arr[i] = static_cast<uint32_t>(i);
    }

    // Actual workload: Perform repeated accesses with the specified stride
    for (size_t iteration = 0; iteration < iterations; iteration++) {
        for (size_t i = 0; i < SIZE; i += stride) {
            arr[i] += 1;
            tracer.recordAccess(&arr[i], false);
            tracer.recordAccess(&arr[i], true);
        }
    }

    delete[] arr;
}

void stridedAccessA(MemoryTracer& tracer) {
    const size_t STRIDE = 2048;
    strideAccessHelper(tracer, STRIDE, 50);
}

void strideAccessB(MemoryTracer& tracer) {
    const size_t STRIDE = 2064;
    strideAccessHelper(tracer, STRIDE, 50);
}

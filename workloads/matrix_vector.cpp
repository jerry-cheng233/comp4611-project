#include <iostream>
#include <vector>
#include <cstdint>
#include "../simulator/memory_tracer.h"
#include "../utils.h"

constexpr uint32_t ROWS = 1024;
constexpr uint32_t COLS = 8192;
constexpr uint32_t VEC_SIZE = 8192;

// Naive implementation of matrix-vector multiplication
// This implementation accesses the matrix row by row and multiplies with vector
void naiveImpl(MemoryTracer& tracer) {
    std::cout << "Running naive matrix-vector multiplication..." << std::endl;

    // Initialise matrix and vectors (note that concrete values don't matter)
    std::vector<uint32_t> matrix(ROWS * COLS, 1);  // Initialize with 1s
    std::vector<uint32_t> vector(VEC_SIZE, 2);     // Initialize with 2s
    std::vector<uint32_t> result(ROWS, 0);         // Result vector

    for (uint32_t i = 0; i < ROWS; i++) {
        uint32_t sum = 0;
        for (uint32_t j = 0; j < COLS; j++) {
            // Record read access to matrix element
            tracer.recordAccess(&matrix[i * COLS + j], false);

            // Record read access to vector element
            tracer.recordAccess(&vector[j], false);

            // Perform the multiplication and accumulate
            sum += matrix[i * COLS + j] * vector[j];
        }

        // Record write access to result element
        tracer.recordAccess(&result[i], true);
        result[i] = sum;
    }

    std::cout << "Naive implementation completed." << std::endl;
}

// Tiled implementation with configurable number of tiles
void tiledImplConfigurable(MemoryTracer& tracer, uint32_t num_tiles) {
    std::cout << "Running tiled matrix-vector multiplication with " << num_tiles << " tiles..." << std::endl;

    // Initialise matrix and vectors (note that concrete values don't matter)
    std::vector<uint32_t> matrix(ROWS * COLS, 1);  // Initialize with 1s
    std::vector<uint32_t> vector(VEC_SIZE, 2);     // Initialize with 2s
    std::vector<uint32_t> result(ROWS, 0);         // Result vector

    uint32_t tile_width = COLS / num_tiles;

    TODO_IMPLEMENT_ME;

    std::cout << "Tiled implementation completed with " << num_tiles << " tiles." << std::endl;
}

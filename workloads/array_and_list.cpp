#include "../simulator/memory_tracer.h"
#include <cstdlib>
#include <cstdint>
#include <memory>

#ifdef _WIN32
#include <malloc.h>
#endif

// Array access
void simpleArrayAccess(MemoryTracer& tracer) {
    const int N = 1000;
    const size_t PAGE_SIZE = 4096;

    // Allocate page-aligned for convenience
    size_t allocation_size = N * sizeof(uint32_t);
    if (allocation_size % PAGE_SIZE != 0) {
        allocation_size = (allocation_size / PAGE_SIZE + 1) * PAGE_SIZE;
    }

    uint32_t* arr;
#ifdef _WIN32
    arr = static_cast<uint32_t*>(_aligned_malloc(allocation_size, PAGE_SIZE));
#else
    arr = static_cast<uint32_t*>(aligned_alloc(PAGE_SIZE, allocation_size));
#endif

    // Workload
    for (int i = 0; i < N; i++) {
        arr[i] = i + 1;
        tracer.recordAccess(&arr[i], false);
        tracer.recordAccess(&arr[i], true);
    }

    // Teardown
#ifdef _WIN32
    _aligned_free(arr);
#else
    free(arr);
#endif
}

// Linked list access
void simpleListAccess(MemoryTracer& tracer) {

    struct Node {
        int data;
        Node* next;
    };

    const int N = 1000;
    Node* head = nullptr;
    Node* tail = nullptr;

    // Setup: Create a linked list
    for (int i = 0; i < N; i++) {
        Node* node = new Node{i, nullptr};
        if (head == nullptr) {
            head = node;
            tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
    }

    // Actual workload: Traverse
    Node* current = head;
    while (current != nullptr) {
        tracer.recordAccess(&current->data, false);
        tracer.recordAccess(&current->next, false);
        current = current->next;
    }

    // Teardown
    while (head != nullptr) {
        Node* node = head;
        head = head->next;
        delete node;
    }
}

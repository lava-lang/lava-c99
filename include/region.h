#ifndef LAVA_REGION_H
#define LAVA_REGION_H

#include <stdlib.h>
#include <stdio.h>
#include "util.h"

typedef struct MemRegion {
//    struct MemRegion* next;
    size_t capacity;
    size_t len;
    void* data;
} MemRegion;

static size_t GLOBAL_REGION_CAPACITY = 10000;
static MemRegion GLOBAL_REGION = {0};

void initGlobalRegion(void* data) {
    GLOBAL_REGION.capacity = GLOBAL_REGION_CAPACITY;
    GLOBAL_REGION.data = data;
}

void* allocRegion(MemRegion* region, size_t allocationSize) {
    //ASSERT(region->len + allocationSize <= region->capacity, "Memory Region out of memory!");
    if (region->len + allocationSize > region->capacity) {
        //TODO
        fprintf(stderr, "Memory Region out of memory!");
        exit(1);
    }
    void* ptr = &region->data[region->len];
    region->len += allocationSize;
    return ptr;
}

void* allocGlobal(size_t allocationSize) {
    return allocRegion(&GLOBAL_REGION, allocationSize);
}

#define RALLOC(ELEMENTS, SIZE) rcallocSafe(ELEMENTS, SIZE, __FILE__, __LINE__)

static void* rcallocSafe(size_t elements, size_t size, char* file, int line) {
    void* ptr = allocGlobal(elements * size);
    checkPtr(ptr, size, file, line, false);
    return ptr;
}

void freeGlobalRegion() {
    FREE(GLOBAL_REGION.data);
}

#endif //LAVA_REGION_H

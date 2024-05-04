#ifndef LAVA_REGION_API_H
#define LAVA_REGION_API_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct MemRegion {
    size_t capacity;
    size_t len;
    void* data;
} MemRegion;

static size_t GLOBAL_REGION_CAPACITY = 10000;
static MemRegion GLOBAL_REGION = {0};

void initGlobalRegion() {
    GLOBAL_REGION.capacity = GLOBAL_REGION_CAPACITY;
    GLOBAL_REGION.data = calloc(1, GLOBAL_REGION_CAPACITY);
}

void* allocRegion(MemRegion* region, size_t allocationSize) {
    if (region->len + allocationSize > region->capacity) {
        fprintf(stderr, "Memory Region out of memory!");
        exit(1);
    }
    void* ptr = &region->data[region->len];
    region->len += allocationSize;
    return ptr;
}

void clearRegion(MemRegion* region) {
    region->len = 0;
    memset(region->data, 0, region->capacity); //Technically not required, but just in case
}

void* allocGlobalRegion(size_t allocationSize) {
    return allocRegion(&GLOBAL_REGION, allocationSize);
}

void clearGlobalRegion() {
    clearRegion(&GLOBAL_REGION);
}

void freeGlobalRegion() {
    free(GLOBAL_REGION.data);
}
#endif //LAVA_REGION_API_H
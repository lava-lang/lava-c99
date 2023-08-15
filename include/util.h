#ifndef LAVA_UTIL_H
#define LAVA_UTIL_H
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "lexer.h"

#define DEBUG_MODE 1
#define BASIC_MODE 1
#define DEBUG_ALLOCS 0

#if DEBUG_MODE == 1
#define INFO(MSG, ...) \
    printf("Lava - "); \
    printf(MSG, __VA_ARGS__); \
    printf("\n");
#else
    #define INFO(MSG, ...)
#endif

#define LAVA(MSG, ERR, ...) \
fprintf(stderr, ERR); \
fprintf(stderr, MSG, __VA_ARGS__); \
fprintf(stderr, "\n%s:%i\n",__FILE__,__LINE__); \

#define PANIC(MSG, ...) \
LAVA(MSG, "Lava Error: ", __VA_ARGS__); \
exit(EXIT_FAILURE); \

#if BASIC_MODE == 1
    #define ASSERT(EX, MSG, ...) \
    if ((EX)) { PANIC(MSG, __VA_ARGS__) };
#else
    #define ASSERT(EX, MSG, ...)
#endif

#if DEBUG_MODE == 1
    #define DEBUG(MSG, ...) \
    INFO(MSG, __VA_ARGS__)
#else
    #define DEBUG(MSG, ...)
#endif

#if BASIC_MODE == 1 || DEBUG_MODE == 1
    #define BASIC(MSG, ...) \
    printf("Lava: "); \
    printf(MSG, __VA_ARGS__); \
    printf("\n");
#else
    #define BASIC(MSG, ...)
#endif

static size_t ALLOC_COUNT = 0;
static size_t FREE_COUNT = 0;

static void checkPtr(void* ptr, size_t size, char* file, int line, bool debug) {
    if (!ptr) {
        PANIC("Could not allocate %zu bytes!\n", size)
    }
    if (!debug) return;
    #if DEBUG_ALLOCS == 1
        fprintf(stderr, "Allocation at: ");
        fprintf(stderr, "%s:%i\n", file, line);
    #endif
}

static void* mallocSafe(size_t size, char* file, int line) {
    void* ptr = malloc(size);
    checkPtr(ptr, size, file, line, true);
    ALLOC_COUNT++;
    return ptr;
}

static void* callocSafe(size_t elements, size_t size, char* file, int line) {
    void* ptr = calloc(elements, size);
    checkPtr(ptr, size, file, line, true);
    ALLOC_COUNT++;
    return ptr;
}

static void* reallocSafe(void* ptr, size_t size) {
    void* newPtr = realloc(ptr, size);
    if (!newPtr) {
        free(ptr);
        PANIC("Could not reallocate %zu bytes!\n", size);
    }
    return newPtr;
}

static void freeSafe(void* ptr) {
    if (!ptr) return;
    FREE_COUNT++;
    free(ptr);
}

#define MALLOC(SIZE) mallocSafe(SIZE, __FILE__, __LINE__)
#define CALLOC(ELEMENTS, SIZE) callocSafe(ELEMENTS, SIZE, __FILE__, __LINE__)
#define REALLOC(PTR, SIZE) reallocSafe(PTR, SIZE)
#define FREE(PTR) freeSafe(PTR)

#define packed __attribute__((__packed__))

//TODO replace use with StringBuffer structure that
// has bigger initial size and reallocs when needed
char* mallocStr(char* source) {
    size_t size = strlen(source) + 1;
    char* result = MALLOC(size * sizeof(char));
    ASSERT(result == NULL, "Cannot allocate %zu bytes!", size);
    strcpy(result, source); //TODO null terminator?
    return result;
}

char* concatNStr(char* a, char* b, size_t lenB) {
    size_t newSize = strlen(a) + lenB + 1;
    a = REALLOC(a, newSize * sizeof(char));
    ASSERT(a == NULL, "Cannot allocate %zu bytes!", newSize);
    return strncat(a, b, lenB);
}

char* concatStr(char* a, char* b) {
//    return concatNStr(a, b, strlen(b));
    size_t newSize = strlen(a) + strlen(b) + 1;
    a = REALLOC(a, newSize * sizeof(char));
    ASSERT(a == NULL, "Cannot allocate %zu bytes!", newSize);
    return strcat(a, b);
}
#endif //LAVA_UTIL_H
#ifndef LAVA_UTIL_H
#define LAVA_UTIL_H
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "lexer.h"

#define INFO(MSG, ...) \
printf("Lava: "); \
printf(MSG, __VA_ARGS__); \
printf("\n"); \

//TODO print line and pos with code
#define PANIC(MSG, ...) \
fprintf(stderr, "%s:%i - Lava Compiler Error:\n",__FILE__,__LINE__); \
fprintf(stderr, MSG, __VA_ARGS__); \
exit(1); \

//TODO Only run if IFDEF is defined
#define ASSERT(EX, MSG, ...) \
if((EX)) { PANIC(MSG, __VA_ARGS__) };

char* charToStr(char c) {
    char* str = malloc(sizeof(char) * 2);
    str[0] = c;
    str[1] = '\0';
    return str;
}

char* mallocStr(char* source) {
    size_t size = strlen(source) + 1;
    char* result = malloc(size * sizeof(char));
    ASSERT(result == NULL, "Cannot allocate %zu bytes!", size);
    strcpy(result, source);
    return result;
}

char* concatStr(char* a, char* b) {
    size_t newSize = strlen(a) + strlen(b) + 1;
    a = realloc(a, newSize * sizeof(char));
    ASSERT(a == NULL, "Cannot allocate %zu bytes!", newSize);
    return strcat(a, b);
}
#endif //LAVA_UTIL_H

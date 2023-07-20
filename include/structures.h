#ifndef LAVA_STRUCTURES_H
#define LAVA_STRUCTURES_H

#include <stdlib.h>
#include <string.h>
#include "util.h"

typedef struct DynArray {
    size_t len;
    int elementSize;
    void** elements;
} DynArray;

DynArray* arrayInit(int elementSize) {
    DynArray* array = CALLOC(1, sizeof(DynArray));
    array->elementSize = elementSize;
    array->len = 0;
    return array;
}

void arrayFree(DynArray* array) {
    for (int i = 0; i < array->len; ++i) {
        FREE(array->elements[i]);
    }
    FREE(array);
}

void arrayAppend(DynArray* array, void* element) {
    array->len++;
    array->elements = REALLOC(array->elements, (array->len + 1) * array->elementSize);
    array->elements[array->len - 1] = element;
}

typedef struct OutputBuffer {
    char* prefix;
    char* code;
    size_t tab;
} OutputBuffer;

OutputBuffer* bufferInit() {
    OutputBuffer* buffer = CALLOC(1, sizeof(OutputBuffer));
    buffer->code = CALLOC(2, sizeof(char));
    buffer->code[0] = '\0';
    buffer->prefix = CALLOC(2, sizeof(char));
    buffer->prefix[0] = '\0';
    buffer->tab = 0;
    return buffer;
}

void bufferFree(OutputBuffer* buffer) {
    FREE(buffer->code);
    FREE(buffer->prefix);
    FREE(buffer);
}

//TODO support multiple strings
void bufferAppend(OutputBuffer* buffer, char* value) {
    buffer->code = concatStr(buffer->code, value);
}

void bufferAppendView(OutputBuffer* buffer, StrView* view) {
    buffer->code = concatNStr(buffer->code, view->start, view->len);
}

void bufferIndent(OutputBuffer* buffer) {
    buffer->tab++;
}

void bufferUnindent(OutputBuffer* buffer) {
    if ((buffer->tab - 1) < 0) {
        PANIC("Uneven tab indentation!", NULL);
    }
    buffer->tab--;
}

void bufferAppendIndent(OutputBuffer* buffer) {
    if (buffer->tab > 0) {
        char* tabs = MALLOC((buffer->tab * sizeof(char)) + 1);
        for (size_t t = 0; t < buffer->tab; ++t) {
            tabs[t] = '\t';
        }
        tabs[buffer->tab] = '\0';
        bufferAppend(buffer, tabs);
        FREE(tabs);
    }
}

void bufferAppendPrefix(OutputBuffer* buffer, char* value) {
    buffer->prefix = concatStr(buffer->prefix, value);
}

void bufferAppendPrefixView(OutputBuffer* buffer, StrView* view) {
    buffer->prefix = concatNStr(buffer->prefix, view->start, view->len);
}

char* bufferBuild(OutputBuffer* buffer) {
    return (buffer->prefix = concatStr(buffer->prefix, buffer->code));
}
#endif
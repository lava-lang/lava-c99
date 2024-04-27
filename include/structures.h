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
    FILE* fpPrefix;
    FILE* fpCode;
    bool prefix;
    size_t tab;
} OutputBuffer;

OutputBuffer* bufferInit(FILE* fpPrefix, FILE* fpCode) {
    OutputBuffer* buffer = CALLOC(1, sizeof(OutputBuffer));
    buffer->fpPrefix = fpPrefix;
    buffer->fpCode = fpCode;
    buffer->prefix = true;
    buffer->tab = 0;
    return buffer;
}

void bufferFree(OutputBuffer* buffer) {
    FREE(buffer);
}

//TODO support multiple strings
void bufferAppend(OutputBuffer* buffer, char* value) {
    if (buffer->prefix == true) {
        fputs(value, buffer->fpPrefix);
    } else {
        fputs(value, buffer->fpCode);
    }
}

void bufferAppendView(OutputBuffer* buffer, StrView* view) {
    for (size_t i = 0; i < view->len; ++i) {
        char c = *(view->start + i);
        if (c == '\0') continue;
        if (buffer->prefix == true) {
            fputc(c, buffer->fpPrefix);
        } else {
            fputc(c, buffer->fpCode);
        }
    }
}

void bufferPrefix(OutputBuffer* buffer) {
    buffer->prefix = true;
}

void bufferCode(OutputBuffer* buffer) {
    buffer->prefix = false;
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
#endif
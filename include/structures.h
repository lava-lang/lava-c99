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
    FILE* fp;
    size_t tab;
} OutputBuffer;

OutputBuffer* bufferInit(FILE* fp) {
    OutputBuffer* buffer = CALLOC(1, sizeof(OutputBuffer));
    buffer->tab = 0;
    buffer->fp = fp;
    return buffer;
}

void bufferFree(OutputBuffer* buffer) {
    FREE(buffer);
}

//TODO support multiple strings
void bufferAppend(OutputBuffer* buffer, char* value) {
    fputs(value, buffer->fp);
}

void bufferAppendView(OutputBuffer* buffer, StrView* view) {
    for (size_t i = 0; i < view->len; ++i) {
        char c = *(view->start + i);
        if (c == '\0') continue;
        fputc(c, buffer->fp);
    }
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
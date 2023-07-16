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
    char* code;
    DynArray* imports;
    DynArray* definitions;
    size_t tab;
} OutputBuffer;

OutputBuffer* bufferInit() {
    OutputBuffer* buffer = CALLOC(1, sizeof(OutputBuffer));
    buffer->code = CALLOC(2, sizeof(char));
    buffer->code[0] = '\0';
    buffer->imports = arrayInit(sizeof(char *));
    buffer->definitions = arrayInit(sizeof(char *));
    buffer->tab = 0;
    return buffer;
}

void bufferFree(OutputBuffer* buffer) {
    FREE(buffer->code);
    arrayFree(buffer->imports);
    arrayFree(buffer->definitions);
    FREE(buffer);
}

void bufferAppend(OutputBuffer* buffer, char* value) {
    buffer->code = concatStr(buffer->code, value);
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
    }
}

void bufferAddImport(OutputBuffer* buffer, char* value) {
//    char* prefix = mallocStr("#include ");
//    value = concatStr(prefix, value); //Append include
//    value = concatStr(value, "\n");
    arrayAppend(buffer->imports, mallocStr(value));
}

void bufferAddDef(OutputBuffer* buffer, char* value) {
    value = concatStr(value, "\n");
    arrayAppend(buffer->definitions, value);
}

char* bufferBuild(OutputBuffer* buffer) {
    char* code = mallocStr("\0");
    for (size_t i = 0; i < buffer->imports->len; ++i) {
        code = concatStr(buffer->imports->elements[i], code);
    }
    if (buffer->definitions->len > 0) {
        code = concatStr(code, "\n");
        for (size_t i = 0; i < buffer->definitions->len; ++i) {
            code = concatStr(code, buffer->definitions->elements[i]);
        }
    }
    return (buffer->code = concatStr(code, buffer->code));
}
#endif
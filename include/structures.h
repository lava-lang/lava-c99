#ifndef LAVA_STRUCTURES_H
#define LAVA_STRUCTURES_H

#include <stdlib.h>
#include <string.h>
#include "util.h"

typedef struct List {
    size_t len;
    int elementSize;
    void** elements;
} List;

List* listInit(int elementSize) {
    List* list = CALLOC(1, sizeof(List));
    list->elementSize = elementSize;
    list->len = 0;
    return list;
}

void listFree(List* list) {
    free(list->elements);
    free(list);
}

void listAppend(List* list, void* element) {
    list->len++;
    list->elements = REALLOC(list->elements, (list->len + 1) * list->elementSize);
    list->elements[list->len - 1] = element;
}

typedef struct OutputBuffer {
    char* code;
    List* imports;
    size_t tab;
} OutputBuffer;

OutputBuffer* bufferInit() {
    OutputBuffer* buffer = CALLOC(1, sizeof(OutputBuffer));
    buffer->code = CALLOC(2, sizeof(char));
    buffer->code[0] = '\0';
    buffer->imports = listInit(sizeof(char*));
    buffer->tab = 0;
    return buffer;
}

void bufferFree(OutputBuffer* buffer) {
    listFree(buffer->imports);
    free(buffer->code);
    free(buffer);
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
    char* prefix = mallocStr("#include ");
    value = concatStr(prefix, value); //Append include
    value = concatStr(value, "\n");
    listAppend(buffer->imports, value);
}

char* bufferBuild(OutputBuffer* buffer) {
    for (size_t i = 0; i < buffer->imports->len; ++i) {
        buffer->code = concatStr(buffer->imports->elements[i], buffer->code);
    }
    return buffer->code;
}
#endif
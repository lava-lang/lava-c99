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
    char* bootstrap;
    size_t tab;
} OutputBuffer;

OutputBuffer* bufferInit() {
    OutputBuffer* buffer = CALLOC(1, sizeof(OutputBuffer));
    buffer->code = CALLOC(2, sizeof(char));
    buffer->code[0] = '\0';
    buffer->bootstrap = CALLOC(2, sizeof(char)); //TODO mayne this should be Set?
    buffer->bootstrap[0] = '\0';
    buffer->tab = 0;
    return buffer;
}

void bufferFree(OutputBuffer* buffer) {
    free(buffer->bootstrap);
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
    if (strstr(buffer->bootstrap, value) == NULL) { //Avoid duplicate entries
        char* prefix = mallocStr("#include ");
        value = concatStr(prefix, value); //Append include
        size_t initialSize = strlen(buffer->bootstrap);
        size_t importLen = strlen(value);
        size_t newSize = initialSize + importLen + (initialSize > 1 ? 2 : 3);
        buffer->bootstrap = REALLOC(buffer->bootstrap, newSize * sizeof(char));
        for (int i = 0; i < importLen; ++i) {
            buffer->bootstrap[initialSize + i - (initialSize > 1 ? 1 : 0)] = value[i];
        }
        buffer->bootstrap[newSize - 3] = '\n';
        buffer->bootstrap[newSize - 2] = '\n';
        buffer->bootstrap[newSize - 1] = '\0';
    }
}

char* bufferBuild(OutputBuffer* buffer) {
    buffer->bootstrap = concatStr(buffer->bootstrap, buffer->code);
    return buffer->bootstrap;
}
#endif
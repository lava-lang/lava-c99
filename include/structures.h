#ifndef LAVA_STRUCTURES_H
#define LAVA_STRUCTURES_H

#include <stdlib.h>
#include <string.h>

typedef struct List {
    int len;
    int elementSize;
    void** elements;
} List;

List* listInit(int elementSize) {
    List* list = calloc(1, sizeof(List));
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
    list->elements = realloc(list->elements, (list->len + 1) * list->elementSize);
    list->elements[list->len - 1] = element;
}

typedef struct OutputBuffer {
    char* code;
    List* imports;
} OutputBuffer;

OutputBuffer* bufferInit() {
    OutputBuffer* buffer = calloc(1, sizeof(OutputBuffer));
    buffer->code = calloc(2, sizeof(char));
    buffer->code[0] = '\0';
    //TODO mayne this should be Set?
    buffer->imports = listInit(sizeof(char));
    return buffer;
}

void bufferFree(OutputBuffer* buffer) {
    listFree(buffer->imports);
    free(buffer->code);
    free(buffer);
}

void bufferAppend(OutputBuffer* buffer, char* value) {
    size_t newSize = strlen(buffer->code) + strlen(value) + 2;
    buffer->code = realloc(buffer->code, newSize * sizeof(char));
    strcat(buffer->code, value);
}

void bufferAddImport(OutputBuffer* buffer, char* import) {
    for (int i = 0; i < buffer->imports->len; ++i) {
        if (strcmp((char*) buffer->imports->elements[i], import) == 0) {
            //Avoid duplicate entries
            return;
        }
    }
    listAppend(buffer->imports, import);
}
#endif
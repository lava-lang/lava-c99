#ifndef LAVA_FILE_H
#define LAVA_FILE_H

#include <stdlib.h>
#include <stdio.h>

char* read_file(char* filename) {
    char* buffer = 0;
    long length;
    FILE* f = fopen (filename, "rb");

    if (f) {
        fseek (f, 0, SEEK_END);
        length = ftell (f);
        fseek (f, 0, SEEK_SET);
        buffer = calloc (length, length);
        if (buffer) {
            fread (buffer, 1, length, f);
        }
        fclose (f);
    } else {
        fprintf(stderr, "Could not open file!");
        exit(1);
    }

    return buffer;
}
#endif
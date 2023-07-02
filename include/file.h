#ifndef LAVA_FILE_H
#define LAVA_FILE_H

#include <stdlib.h>
#include <stdio.h>

char* read_file(char* filename) {
    char* buffer;
    long length;
    FILE* f = fopen (filename, "rb");
    ASSERT(!f, "Could not open %s!", filename);

    fseek (f, 0, SEEK_END);
    length = ftell (f);
    fseek (f, 0, SEEK_SET);
    buffer = calloc (length, length);
    if (buffer) {
        fread (buffer, 1, length, f);
    }
    fclose (f);

    return buffer;
}

void write_file(const char* filepath, const char* data) {
    FILE *fp = fopen(filepath, "w");
    if (fp != NULL) {
        fputs(data, fp);
        fclose(fp);
    }
}
#endif
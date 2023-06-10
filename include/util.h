#ifndef LAVA_UTIL_H
#define LAVA_UTIL_H

#include <stdlib.h>

char* charToStr(char c) {
    char* str = malloc(sizeof(char) * 2);
    str[0] = c;
    str[1] = '\0';
    return str;
}
#endif
#ifndef LAVA_STRVIEW_H
#define LAVA_STRVIEW_H

#include <stdlib.h>
#include "util.h"

typedef struct StrView {
    char* start;
    size_t len;
} StrView;

static StrView EMPTY_VIEW = {"", 0};

void printView(StrView* view, char* suffix) {
    printf("%.*s%s", (int) view->len, view->start, suffix);
}

bool viewCmp(StrView* view, char* other) {
    size_t otherLen = strlen(other);
    return strncmp(view->start, other, view->len < otherLen ? otherLen : view->len) == 0;
}

char* viewToStr(StrView* view) {
    char* str = MALLOC((view->len + 1) * sizeof(char));
    strncpy(str, view->start, view->len);
    str[view->len] = '\0';
    return str;
}

StrView* strToView(char* source) {
    StrView* view = RALLOC(1, sizeof(StrView));
    view->start = strcpy(RALLOC(1, (view->len = strlen(source) + 1) * sizeof(char)), source);
    return view;
}
#endif //LAVA_STRVIEW_H

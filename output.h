#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "include/region.h"

typedef struct Rect Rect;

void Rect_draw(int* w, int* h);
typedef struct Widget Widget;

void Widget_draw(int* w, int* h);
int main();
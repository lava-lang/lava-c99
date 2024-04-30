#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct Rect Rect;

int Rect_getSize(int* w, int* h);
typedef struct Widget Widget;

int Widget_getSize(int* w, int* h);
void Widget_draw(int* w, int* h);
int main();
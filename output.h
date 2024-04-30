#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct Rect Rect;

int Rect_area(int* w, int* h);
typedef struct Widget Widget;

int Widget_area(int* w, int* h);
void Widget_init(int* w, int* h);
void Widget_draw(int* w, int* h);
typedef struct Label Label;

int Label_area(int* w, int* h);
void Label_init(int* w, int* h);
void Label_draw(int* w, int* h);
int main();
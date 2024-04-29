#include <stdbool.h>
#include <stdint.h>
#include <math.h>

typedef struct Vec Vec;

void Vec_add(int* x, int* y, int* z, Vec* v);
void Vec_sub(int* x, int* y, int* z, Vec* v);
int main();
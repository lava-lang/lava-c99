#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum Direction Direction;

typedef struct Vec Vec;
int Vec_add(	int xa, 	int xb) {
		return xa + xb;
}
void makeVec(int x, int y, int z);
int test(int x, int y, int z);
int main();
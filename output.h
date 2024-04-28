#include <stdio.h>

typedef struct Vec Vec;

int Vec_offset(	int x, 	int y, 	int z, 	int w) {
		return x + y + z + w;
}
int Vec_size(	int x, 	int y, 	int z) {
		return x + y + z;
}
int main();
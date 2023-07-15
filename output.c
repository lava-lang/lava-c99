#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct Vec_t Vec;
void makeVec(int x, int y, int z);
int main();

int32_t i = 187686223;
int ii = 0;
float f = 1.5;
char* s = "lava";
char c = 'x';
bool flag = true;

struct Vec_t {
	float x;
	float y;
	float z;
	int s;
};

void makeVec(int x, int y, int z) {

}

int main() {
	printf("i: %d\n", i);
	int var1 = 10;
	printf("var1: %d\n", var1);
	return 0;
}
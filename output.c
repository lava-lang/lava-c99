#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

int32_t x = 187686223;
float y = 1.5;
char* s = "lava";
char c = 'x';
bool flag = true;
int i = 0;

typedef struct Vec3_t {
	float x;
	float y;
	float z;
} Vec3;

int main() {
	printf("x: %d\n", x);
	int var1 = 10;
	printf("var1: %d\n", var1);
	return 0;
}
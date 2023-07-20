#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct Vec_t Vec;
void makeVec(int x, int y, int z);
int main();

int32_t i = 187686223;
int ii = 0;
float f = 1.5;
char* s = "l";
char c = 'x';
bool flag = true;
int nums[];

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
	return 0;
}
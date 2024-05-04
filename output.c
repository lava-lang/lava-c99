#include "output.h"


struct Vec {

};
struct Sphere {
Vec c;
Vec col;

};
struct Ray {
Sphere o;
Vec d;

};



int main() {
	initGlobalRegion(calloc(1, GLOBAL_REGION_CAPACITY));

	freeGlobalRegion();
}

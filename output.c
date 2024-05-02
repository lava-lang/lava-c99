#include "output.h"


struct Rect {
int w;
int h;

};
void Rect_draw(int* w, int* h) {
	printf("I am a Rect!\n");
}

Rect* test() {
	Rect _r = {	100, 	50};
	Rect* r = allocRegion(&GLOBAL_REGION, sizeof(Rect));
	memcpy(r, &_r, sizeof(Rect));
	return r;
}

int main() {
	initGlobalRegion(calloc(1, GLOBAL_REGION_CAPACITY));
	Rect* r = test();
	printf("%d %d\n", r->w, r->h);
	freeGlobalRegion();
}

#include "output.h"


struct Rect {
int w;
int h;

};
void Rect_draw(int* w, int* h) {
	printf("I am a Rect!\n");
}

struct Widget {
int w;
int h;

};
void Widget_draw(int* w, int* h) {
	printf("I am a Widget!\n");
}

Widget test() {
	Widget _w = {	100, 	50};
	Widget* w = allocRegion(&GLOBAL_REGION, sizeof(Widget));
	memcpy(w, &_w, sizeof(Widget));
	return w;
}

int main() {
	initGlobalRegion(calloc(1, GLOBAL_REGION_CAPACITY));

	freeGlobalRegion();
}

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

int main() {
	int* x = allocRegion(&GLOBAL_REGION, sizeof(int));
	Widget w = {	100, 	50};
	Widget_draw(	&w.w,	&w.h);
	freeGlobalRegion();
}

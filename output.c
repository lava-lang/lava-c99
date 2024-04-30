#include "output.h"


struct Rect {
int w;
int h;

};
int Rect_getSize(int* w, int* h) {
	return *w * *h;
}

struct Widget {
int w;
int h;

};
int Widget_getSize(int* w, int* h) {
	return *w * *h;
}
void Widget_draw(int* w, int* h) {
	int area = *w * *h;
	printf("Area: %d\n", area);
}

int main() {
	Widget w = {	100, 	50};
	Widget_draw(	&w.w,	&w.h);
}

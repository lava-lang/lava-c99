#include "output.h"


struct Rect {
int w;
int h;

};

struct Widget {
int w;
int h;

};
void Widget_draw(int* w, int* h) {
	int area = *w * *h;
	printf("Area: %d\n", area);
}

int main() {
	Widget w = {	100, 	50};
	Widget_draw(	&w.w,	&w.h);
}

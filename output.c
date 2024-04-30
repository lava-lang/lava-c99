#include "output.h"


struct Rect {
int w;
int h;

};
int Rect_area(int* w, int* h) {
	return *w * *h;
}

struct Widget {
int w;
int h;

};
int Widget_area(int* w, int* h) {
	return *w * *h;
}
void Widget_init(int* w, int* h) {

}
void Widget_draw(int* w, int* h) {
	int x = Widget_area(	w,	h);;
	printf("Area For Widget: %d\n", x);
}

struct Label {
int w;
int h;

};
int Label_area(int* w, int* h) {
	return *w * *h;
}
void Label_init(int* w, int* h) {

}
void Label_draw(int* w, int* h) {
	int x = Widget_area(	w,	h);;
	printf("Area For Widget: %d\n", x);
}
void Label_drawd(int* w, int* h) {
	int x = Label_area(	w,	h);;
	printf("Area For Label: %d\n", x);
}

int main() {
	Widget w = {	100, 	50};
	Widget_draw(	&w.w,	&w.h);
	Label l = {	25, 	10};
	Label_draw(	&l.w,	&l.h);
}

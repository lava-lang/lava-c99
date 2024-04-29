#include "output.h"


struct Vec {
double x;
double y;
double z;

};
void Vec_add(double* x, double* y, double* z, Vec* v) {
	*x += v->x;
	*y += v->y;
	*z += v->z;
}
void Vec_sub(double* x, double* y, double* z, Vec* v) {
	*x -= v->x;
	*y -= v->y;
	*z -= v->z;
}
void Vec_mult(double* x, double* y, double* z, double m) {
	*x *= m;
	*y *= m;
	*z *= m;
}
void Vec_div(double* x, double* y, double* z, double d) {
	*x /= d;
	*y /= d;
	*z /= d;
}
void Vec_norm(double* x, double* y, double* z) {
	double m = *x * *x + *y * *y + *z * *z;
	m = sqrt(m);;
	*x /= m;
	*y /= m;
	*z /= m;
}
double Vec_dot(double* x, double* y, double* z, Vec* v) {
	return *x * v->x + *y * v->y + *z * v->z;
}

struct Ray {
Vec o;
Vec d;

};

struct Sphere {
Vec c;
double r;

};
void Sphere_getNormal(Vec* c, double* r, Vec* out, Vec* p) {
	out->x = p->x;
	out->y = p->y;
	out->z = p->z;
	Vec_sub(	&out->x,	&out->y,	&out->z,	c);
	Vec_div(	&out->x,	&out->y,	&out->z,	*r);
}
bool Sphere_intersects(Vec* c, double* r, Ray* ray) {
	Vec p = {	0, 	0, 	0};
	p = ray->o;
	Vec_sub(	&p.x,	&p.y,	&p.z,	c);
	double b = 2 * Vec_dot(	&p.x,	&p.y,	&p.z,	&ray->d);
	double c1 = Vec_dot(	&p.x,	&p.y,	&p.z,	&p) - (*r * *r);
	double disc = b * b - 4 * c1;
	if (disc < 0.0001) {
		return false;
	}
	else {
		return true;
	}
}
double Sphere_intersection(Vec* c, double* r, Ray* ray) {
	Vec p = {	0, 	0, 	0};
	p = ray->o;
	Vec_sub(	&p.x,	&p.y,	&p.z,	c);
	double b = 2 * Vec_dot(	&p.x,	&p.y,	&p.z,	&ray->d);
	double c1 = Vec_dot(	&p.x,	&p.y,	&p.z,	&p) - (*r * *r);
	double disc = sqrt(b * b - 4 * c1);;
	double t0 = -b - disc;
	double t1 = -b + disc;
	if (t0 < t1) {
		return t0;
	}
	else {
		return t1;
	}
}

void clamp(Vec* col) {
	if (col->x > 255) {
		col->x = 255;
	}
	else 
	if (col->x < 0) {
		col->x = 0;
	}
	if (col->y > 255) {
		col->y = 255;
	}
	else 
	if (col->y < 0) {
		col->y = 0;
	}
	if (col->z > 255) {
		col->z = 255;
	}
	else 
	if (col->z < 0) {
		col->z = 0;
	}
}

int main() {
	int h = 500;
	int w = 500;
	Vec white = {	255, 	255, 	255};
	Vec bg = {	16, 	16, 	16};
	Vec green = {	34, 	139, 	34};
	Vec sv1 = {	w * 0.5, 	h * 0.5, 	50};
	Vec sv2 = {	1000, 	-1000, 	0};
	Sphere s1 = {	sv1, 	150};
	Sphere light = {	sv2, 	100};
	FILE *output = fopen("out.ppm", "w");
	fprintf(output, "P3\n%d %d 255\n", w, h);
	double t;
	Vec pixCol = bg;
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			pixCol = bg;
			Vec v1 = {			x, 			y, 			0};
			Vec v2 = {			0, 			0, 			1};
			Ray ray = {			v1, 			v2};
			if (Sphere_intersects(			&s1.c,			&s1.r,			&ray)) {
				t = Sphere_intersection(				&s1.c,				&s1.r,				&ray);
				Vec temp = {				0, 				0, 				0};
				Vec sln = {				0, 				0, 				0};
				temp = ray.d;
				Vec_mult(				&temp.x,				&temp.y,				&temp.z,				t);
				sln = ray.o;
				Vec_add(				&sln.x,				&sln.y,				&sln.z,				&temp);
				temp = light.c;
				Vec l = {				0, 				0, 				0};
				Vec_sub(				&temp.x,				&temp.y,				&temp.z,				&sln);
				l = temp;
				Vec n = {				0, 				0, 				0};
				Sphere_getNormal(				&s1.c,				&s1.r,				&n,				&sln);
				Vec_norm(				&n.x,				&n.y,				&n.z);
				Vec_norm(				&l.x,				&l.y,				&l.z);
				double dt = Vec_dot(				&l.x,				&l.y,				&l.z,				&n);
				Vec whiteTemp = {				white.x, 				white.y, 				white.z};
				Vec_mult(				&whiteTemp.x,				&whiteTemp.y,				&whiteTemp.z,				dt);
				Vec_add(				&whiteTemp.x,				&whiteTemp.y,				&whiteTemp.z,				&green);
				Vec_mult(				&whiteTemp.x,				&whiteTemp.y,				&whiteTemp.z,				0.5);
				pixCol = whiteTemp;
				clamp(				&pixCol);
			}
			fprintf(output, "%f %f %f\n", pixCol.x, pixCol.y, pixCol.z);
		}
	}
	fclose(output);
}

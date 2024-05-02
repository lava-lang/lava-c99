#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "region_api.h"
#include <math.h>

typedef struct Vec Vec;

void Vec_add(double* x, double* y, double* z, Vec* v);
void Vec_sub(double* x, double* y, double* z, Vec* v);
void Vec_mult(double* x, double* y, double* z, double m);
void Vec_div(double* x, double* y, double* z, double d);
void Vec_norm(double* x, double* y, double* z);
double Vec_dot(double* x, double* y, double* z, Vec* v);
typedef struct Ray Ray;

typedef struct Sphere Sphere;

void Sphere_getNormal(Vec* c, double* r, Vec* col, Vec* out, Vec* p);
bool Sphere_intersects(Vec* c, double* r, Vec* col, Ray* ray);
double Sphere_intersection(Vec* c, double* r, Vec* col, Ray* ray);
void clamp(Vec* col);
void drawPixel(Sphere* s, Vec* pixCol, Ray* ray, Sphere* light);
int main();
#include "ray_test.h"
#include <windows.h>
#include <psapi.h>
#include <time.h>

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
    Vec col;

};
void Sphere_getNormal(Vec* c, double* r, Vec* col, Vec* out, Vec* p) {
    out->x = p->x;
    out->y = p->y;
    out->z = p->z;
    Vec_sub(	&out->x,	&out->y,	&out->z,	c);
    Vec_div(	&out->x,	&out->y,	&out->z,	*r);
}
bool Sphere_intersects(Vec* c, double* r, Vec* col, Ray* ray) {
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
double Sphere_intersection(Vec* c, double* r, Vec* col, Ray* ray) {
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



void clamp(int* pixColX, int* pixColY, int* pixColZ) {
    if (*pixColX > 255) {
        *pixColX = 255;
    }
    else
    if (*pixColX < 0) {
        *pixColX = 0;
    }
    if (*pixColY > 255) {
        *pixColY = 255;
    }
    else
    if (*pixColY < 0) {
        *pixColY = 0;
    }
    if (*pixColZ > 255) {
        *pixColZ = 255;
    }
    else
    if (*pixColZ < 0) {
        *pixColZ = 0;
    }
}

void drawPixel(Sphere* s, int* pixColX, int* pixColY, int* pixColZ, Ray* ray, Sphere* light) {
    double t = Sphere_intersection(	&s->c,	&s->r,	&s->col,	ray);
    Vec temp = {	0, 	0, 	0};
    Vec sln = {	0, 	0, 	0};
    temp = ray->d;
    Vec_mult(	&temp.x,	&temp.y,	&temp.z,	t);
    sln = ray->o;
    Vec_add(	&sln.x,	&sln.y,	&sln.z,	&temp);
    temp = light->c;
    Vec l = {	0, 	0, 	0};
    Vec_sub(	&temp.x,	&temp.y,	&temp.z,	&sln);
    l = temp;
    Vec n = {	0, 	0, 	0};
    Sphere_getNormal(	&s->c,	&s->r,	&s->col,	&n,	&sln);
    Vec_norm(	&n.x,	&n.y,	&n.z);
    Vec_norm(	&l.x,	&l.y,	&l.z);
    double dt = Vec_dot(	&l.x,	&l.y,	&l.z,	&n);
    Vec whiteTemp = {	255, 	255, 	255};
    Vec_mult(	&whiteTemp.x,	&whiteTemp.y,	&whiteTemp.z,	dt);
    Vec_add(	&whiteTemp.x,	&whiteTemp.y,	&whiteTemp.z,	&s->col);
    Vec_mult(	&whiteTemp.x,	&whiteTemp.y,	&whiteTemp.z,	0.5);
    *pixColX = whiteTemp.x;
    *pixColY = whiteTemp.y;
    *pixColZ = whiteTemp.z;
    clamp(	pixColX, pixColY, pixColZ);
}

int main() {
    initGlobalRegion(calloc(1, GLOBAL_REGION_CAPACITY));
    clock_t startCodegen = clock();
    int h = 512;
    int w = 512;
    Vec bg = {	16, 	16, 	16};
    Vec lv = {	1000, 	-1000, 	0};
    Sphere light = {	lv, 	100};
    Vec sv1 = {	w * 0.5, 	h * 0.5, 	50};
    Vec purple = {	160, 	32, 	240};
    Sphere s1 = {	sv1, 	125, 	purple};
    Vec sv2 = {	w * 0.75, 	h * 0.75, 	50};
    Vec red = {	255, 	0, 	0};
    Sphere s2 = {	sv2, 	75, 	red};
    Vec sv3 = {	w * 0.25, 	h * 0.25, 	10};
    Vec blue = {	0, 	0, 	255};
    Sphere s3 = {	sv3, 	75, 	blue};
    FILE *output = fopen("c_out.ppm", "w");
    fprintf(output, "P3\n%d %d 255\n", w, h);
    Vec pixCol = {255, 20, 147};
    int pixColX = 0;
    int pixColY = 0;
    int pixColZ = 0;
    char colBuf[3];
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            pixColX = bg.x;
            pixColY = bg.y;
            pixColZ = bg.z;
            Vec v1 = {			x, 			y, 			0};
            Vec v2 = {			0, 			0, 			1};
            Ray ray = {			v1, 			v2};
            if (Sphere_intersects(			&s3.c,			&s3.r,			&s3.col,			&ray)) {
                drawPixel(				&s3,				&pixColX, &pixColY, &pixColZ,				&ray,				&light);
            }
            if (Sphere_intersects(			&s2.c,			&s2.r,			&s2.col,			&ray)) {
                drawPixel(				&s2,				&pixColX, &pixColY, &pixColZ,				&ray,				&light);
            }
            if (Sphere_intersects(			&s1.c,			&s1.r,			&s1.col,			&ray)) {
                drawPixel(				&s1,				&pixColX, &pixColY, &pixColZ,				&ray,				&light);
            }
            itoa(pixColX, colBuf, 10);
            fwrite(colBuf, sizeof(char), pixColX > 99 ? 3 : 2, output);
            fwrite(" ", sizeof(char), 1, output);
            itoa(pixColY, colBuf, 10);
            fwrite(colBuf, sizeof(char), pixColY > 99 ? 3 : 2, output);
            fwrite(" ", sizeof(char), 1, output);
            itoa(pixColZ, colBuf, 10);
            fwrite(colBuf, sizeof(char), pixColZ > 99 ? 3 : 2, output);
            fwrite("\n", sizeof(char), 1, output);
        }
    }
    fclose(output);

    if (0) {
        HINSTANCE hProcHandle = GetModuleHandle(NULL);  // get the current process handle
        PROCESS_MEMORY_COUNTERS_EX memory; // output will go here.
        GetProcessMemoryInfo(hProcHandle, &memory, sizeof(memory)); // call function

        printf("Memory Used: %zu bytes\n", memory.PrivateUsage);
        printf("Memory Used: %f kb\n", ((double) memory.PrivateUsage) / 1024.0f);
        printf("Memory Used: %f mb\n", (((double) memory.PrivateUsage) / 1024.0f) / 1024.0f);
    }

    double elapsed = (double)(clock() - startCodegen) / CLOCKS_PER_SEC;
    printf("Elapsed: %f secs\n", elapsed);

    freeGlobalRegion();
}
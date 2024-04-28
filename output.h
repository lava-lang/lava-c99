#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

typedef struct Vec Vec;

float dot(Vec* v1, Vec* v2);
void writePPM(char* fileName, char* img, int width, int height);
float checkRay(float px, float py, float pz, float dx, float dy, float dz, float r);
int main();
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cmath>
#include <stdio.h>
#include <windows.h>
#include <psapi.h>
#include <time.h>
using namespace std;

class Vec {
public:
    double x, y, z;

    void add(Vec* v) {
        x += v->x; y += v->y; z += v->z;
    }

    void sub(Vec* v) {
        x -= v->x; y -= v->y; z -= v->z;
    }

    void mult(double m) {
        x *= m; y *= m; z *= m;
    }

    void div(double d) {
        x /= d; y /= d; z /= d;
    }

    void norm() {
        double m = x*x + y*y + z*z;
        m = sqrt(m);
        x /= m; y /= m; z /= m;
    }

    double dot(Vec* v) {
        return x * v->x + y * v->y + z * v->z;
    }
};

class Ray {
public:
    Vec o;
    Vec d;
};

class Sphere {
public:
    Vec c;
    double r;
    Vec col;

    void getNormal(Vec* out, Vec* p) {
        out->x = p->x;
        out->y = p->y;
        out->z = p->z;
        out->sub(&c);
        out->div(r);
    }

    // if the discriminant of the intersection formula is a negative
    // or very small number, it does not intersect.
    bool intersects(Ray* ray) {
        Vec p = {0 ,0, 0};
        p = ray->o; //should mem copy
        p.sub(&c);
        double b = 2 * p.dot(&ray->d);
        double c1 = p.dot(&p) - (r * r);
        double disc = b * b - 4 * c1;
        if (disc < 0.0001) {
            return false;
        } else {
            return true;
        }
    }

    double intersection(Ray* ray) {
        Vec p = {0 ,0, 0};
        p = ray->o; //should mem copy
        p.sub(&c);
        double b = 2 * p.dot(&ray->d);
        double c1 = p.dot(&p) - (r * r);
        double disc = sqrt(b * b - 4 * c1);
        // solution point is the smaller point
        double t0 = -b-disc;
        double t1 = -b+disc;
        if (t0 < t1) {
            return t0;
        } else {
            return t1;
        }
    }
};

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
    double t = s->intersection(ray); //returns new
    Vec temp = {0, 0, 0};
    //vector sln
    Vec sln = {0, 0, 0};
    //mult
    temp = ray->d;
    temp.mult(t); //temp has result of mult
    //add
    sln = ray->o;
    sln.add(&temp); //temp gets added to sln

    //vector l
    temp = light->c; //has light.c value
    Vec l = {0, 0, 0};
    temp.sub(&sln);
    l = temp;

    //vector n
    Vec n = {0, 0, 0};
    s->getNormal(&n, &sln); //result of getNormal is stored in n, and sln is not changed

    // dt is the amount of shading produced, determined
    // by Lambertian reflection equation
    n.norm(); //n is now normalised in place
    l.norm(); //l is now normalised in place
    double dt = l.dot(&n); //dt is the in place dot (with in) returning float

    Vec whiteTemp = {255, 255, 255}; //white
    whiteTemp.mult(dt);
    whiteTemp.add(&s->col);
    whiteTemp.mult(0.5);
    *pixColX = whiteTemp.x;
    *pixColY = whiteTemp.y;
    *pixColZ = whiteTemp.z;
    clamp(pixColX, pixColY, pixColZ);
}

void process_mem_usage(double& vm_usage, double& resident_set) {
    vm_usage     = 0.0;
    resident_set = 0.0;

    // the two fields we want
    unsigned long vsize;
    long rss;
    {
        std::string ignore;
        std::ifstream ifs("/proc/self/stat", std::ios_base::in);
        ifs >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
            >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
            >> ignore >> ignore >> vsize >> rss;
    }

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    long page_size_kb = sysInfo.dwPageSize / 1024; // in case x86-64 is configured to use 2MB pages
    vm_usage = vsize / 1024.0;
    resident_set = rss * page_size_kb;
}

int main() {
    using std::cout;
    using std::endl;

    clock_t startCodegen = clock();
    int h = 512;
    int w = 512;
    Vec bg = {16, 16, 16};

    //Light
    Vec lv = {1000, -1000, 0};
    Sphere light = {lv, 100};

    //Shpere 1
    Vec sv1 = {w * 0.5, h * 0.5, 50};
    Vec purple = {160, 32, 240};
    Sphere s1 = {sv1, 125, purple};

    //Sphere 2
    Vec sv2 = {w * 0.75, h * 0.75, 50};
    Vec red = {255, 0, 0};
    Sphere s2 = {sv2, 75, red};

    //Sphere 3
    Vec sv3 = {w * 0.25, h * 0.25, 10};
    Vec blue = {0, 0, 255};
    Sphere s3 = {sv3, 75, blue};

    FILE* output = fopen("cpp_out.ppm", "w");
    //PPM header
    fprintf(output, "P3\n%d %d 255\n", w, h);

    //intersection
//    Vec pixCol = bg;
    int pixColX = 0;
    int pixColY = 0;
    int pixColZ = 0;
    char colBuf[3];
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            pixColX = bg.x;
            pixColY = bg.y;
            pixColZ = bg.z;
            // a ray is sent from the current camera's pixel, out onto the scene
            // if the ray intersects, determine the point of intersection
            // and the direction of the reflected ray
            Vec v1 = {static_cast<double>(x), static_cast<double>(y), 0};
            Vec v2 = {0, 0, 1};
            Ray ray = {v1, v2};
            if (s3.intersects(&ray)) {
                drawPixel(&s3, &pixColX, &pixColY, &pixColZ, &ray, &light);
            }
            if (s2.intersects(&ray)) {
                drawPixel(&s2, &pixColX, &pixColY, &pixColZ, &ray, &light);
            }
            if (s1.intersects(&ray)) {
                drawPixel(&s1, &pixColX, &pixColY, &pixColZ, &ray, &light);
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
    fclose (output);

    if (1) {
        PROCESS_MEMORY_COUNTERS memCounter;
        BOOL result = K32GetProcessMemoryInfo(GetCurrentProcess(), &memCounter, sizeof(memCounter));

        std::cout << "WorkingSetSize: " << memCounter.WorkingSetSize << std::endl;
        std::cout << "WorkingSetSize: " << ((double) memCounter.WorkingSetSize) / 1024.0f << std::endl;
        std::cout << "WorkingSetSize: " << ((((double) memCounter.WorkingSetSize) / 1024.0f) / 1024.0f) << std::endl;
    }

    double elapsed = (double)(clock() - startCodegen) / CLOCKS_PER_SEC;
    printf("Elapsed: %f secs\n", elapsed);
}
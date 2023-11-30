#include <stdio.h>
#include <ctype.h>
#include <stdint.h>

// dummy float function
void print_f(float x) {
    printf("Float: %ff", x);
}

// dummy int function
void print_i(int x) {
    printf("Int: %di", x);
}

int main(int argc, char *argv[]) {
    int x = 42;
    _Generic(x, float: print_f, int: print_i)(x);
}
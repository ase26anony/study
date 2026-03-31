/* test.c - Simple program to generate coverage data */
#include <stdio.h>
#include <stdlib.h>

int add(int a, int b) {
    return a + b;
}

int subtract(int a, int b) {
    return a - b;
}

int multiply(int a, int b) {
    return a * b;
}

int main(int argc, char *argv[]) {
    int x = 10;
    int y = 5;
    
    if (argc > 1) {
        x = atoi(argv[1]);
    }
    if (argc > 2) {
        y = atoi(argv[2]);
    }
    
    printf("x = %d, y = %d\n", x, y);
    printf("add: %d + %d = %d\n", x, y, add(x, y));
    printf("subtract: %d - %d = %d\n", x, y, subtract(x, y));
    printf("multiply: %d * %d = %d\n", x, y, multiply(x, y));
    
    return 0;
}

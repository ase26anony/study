/* test.c - Simple program to generate coverage data */
#include <stdio.h>
#include <stdlib.h>

int add(int a, int b) {
    return a + b;
}

int subtract(int a, int b) {
    return a - b;
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
    
    printf("Add: %d + %d = %d\n", x, y, add(x, y));
    printf("Subtract: %d - %d = %d\n", x, y, subtract(x, y));
    
    return 0;
}

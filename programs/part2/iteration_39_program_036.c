/* test.c - Simple test program for gcov-tool testing */
#include <stdio.h>
#include <stdlib.h>

void func1(int x) {
    printf("func1: %d\n", x);
    if (x > 0) {
        printf("Positive\n");
    } else {
        printf("Non-positive\n");
    }
}

void func2(int x, int y) {
    printf("func2: %d, %d\n", x, y);
    if (x > y) {
        printf("x > y\n");
    } else if (x < y) {
        printf("x < y\n");
    } else {
        printf("x == y\n");
    }
}

int main(int argc, char *argv[]) {
    int a = 0, b = 0;
    
    if (argc > 1) {
        a = atoi(argv[1]);
    }
    if (argc > 2) {
        b = atoi(argv[2]);
    }
    
    func1(a);
    func2(a, b);
    
    // Additional branching for coverage
    for (int i = 0; i < a; i++) {
        if (i % 2 == 0) {
            printf("Even: %d\n", i);
        } else {
            printf("Odd: %d\n", i);
        }
    }
    
    return 0;
}

/* test.c - Simple program to generate coverage data */
#include <stdio.h>
#include <stdlib.h>

void func1(int x) {
    if (x > 0) {
        printf("Positive: %d\n", x);
    } else {
        printf("Non-positive: %d\n", x);
    }
}

void func2(int a, int b) {
    if (a > b) {
        printf("a > b: %d > %d\n", a, b);
    } else if (a < b) {
        printf("a < b: %d < %d\n", a, b);
    } else {
        printf("a == b: %d == %d\n", a, b);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <mode>\n", argv[0]);
        printf("Mode 1: Test func1 with positive\n");
        printf("Mode 2: Test func1 with negative\n");
        printf("Mode 3: Test func2 with a > b\n");
        printf("Mode 4: Test func2 with a < b\n");
        printf("Mode 5: Test func2 with a == b\n");
        return 1;
    }
    
    int mode = atoi(argv[1]);
    
    switch (mode) {
        case 1:
            func1(10);
            break;
        case 2:
            func1(-5);
            break;
        case 3:
            func2(10, 5);
            break;
        case 4:
            func2(5, 10);
            break;
        case 5:
            func2(7, 7);
            break;
        default:
            printf("Invalid mode: %d\n", mode);
            break;
    }
    
    return 0;
}

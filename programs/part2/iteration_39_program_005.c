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

void func2(int x, int y) {
    if (x > y) {
        printf("x > y: %d > %d\n", x, y);
    } else if (x < y) {
        printf("x < y: %d < %d\n", x, y);
    } else {
        printf("x == y: %d == %d\n", x, y);
    }
}

int main(int argc, char *argv[]) {
    int input1 = 0;
    int input2 = 0;
    
    if (argc > 1) {
        input1 = atoi(argv[1]);
    }
    if (argc > 2) {
        input2 = atoi(argv[2]);
    }
    
    func1(input1);
    func2(input1, input2);
    
    return 0;
}

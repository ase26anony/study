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

void func2(int y) {
    for (int i = 0; i < y; i++) {
        printf("Iteration %d\n", i);
    }
}

int main(int argc, char *argv[]) {
    int value = 5;
    
    if (argc > 1) {
        value = atoi(argv[1]);
    }
    
    func1(value);
    func2(value);
    
    return 0;
}

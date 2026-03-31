/* test.c - Simple program to generate coverage data */
#include <stdio.h>
#include <stdlib.h>

void function1(int x) {
    if (x > 0) {
        printf("Positive: %d\n", x);
    } else {
        printf("Non-positive: %d\n", x);
    }
}

void function2(int y) {
    for (int i = 0; i < y; i++) {
        printf("Iteration %d\n", i);
    }
}

int main(int argc, char *argv[]) {
    int value = 5;
    
    if (argc > 1) {
        value = atoi(argv[1]);
    }
    
    function1(value);
    function2(value);
    
    return 0;
}

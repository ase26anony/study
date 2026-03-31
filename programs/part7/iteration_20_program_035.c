/* test.c - Simple program to generate coverage data */
#include <stdio.h>

int function1(int x) {
    if (x > 0) {
        return x * 2;
    } else {
        return x - 1;
    }
}

int function2(int y) {
    for (int i = 0; i < y; i++) {
        printf("Iteration %d\n", i);
    }
    return y * 3;
}

int main() {
    int result1 = function1(5);
    int result2 = function2(3);
    printf("Results: %d, %d\n", result1, result2);
    return 0;
}

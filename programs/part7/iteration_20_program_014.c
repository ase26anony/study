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
    int a = 5;
    int b = -3;
    
    int result1 = function1(a);
    int result2 = function1(b);
    int result3 = function2(3);
    
    printf("Results: %d, %d, %d\n", result1, result2, result3);
    
    return 0;
}

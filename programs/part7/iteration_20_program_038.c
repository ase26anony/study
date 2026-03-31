/* test.c - Simple program for generating coverage data */
#include <stdio.h>

int function1(int x) {
    if (x > 0) {
        return x * 2;
    } else if (x < 0) {
        return x * -1;
    } else {
        return 0;
    }
}

int function2(int a, int b) {
    for (int i = 0; i < a; i++) {
        b += i;
    }
    return b;
}

int main() {
    int result1 = function1(5);
    int result2 = function1(-3);
    int result3 = function1(0);
    
    int sum = function2(3, 10);
    
    printf("Results: %d, %d, %d, %d\n", result1, result2, result3, sum);
    return 0;
}

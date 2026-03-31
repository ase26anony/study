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
    int result = 0;
    for (int i = 0; i < y; i++) {
        result += i;
    }
    return result;
}

int main() {
    int a = function1(5);
    int b = function1(-3);
    int c = function2(4);
    
    printf("Results: %d, %d, %d\n", a, b, c);
    return 0;
}

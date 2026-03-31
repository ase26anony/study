/* test.c - Simple test program for generating coverage data */
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

void function2(int n) {
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            printf("Even: %d\n", i);
        } else {
            printf("Odd: %d\n", i);
        }
    }
}

int main() {
    int result = function1(5);
    printf("function1(5) = %d\n", result);
    
    result = function1(-3);
    printf("function1(-3) = %d\n", result);
    
    result = function1(0);
    printf("function1(0) = %d\n", result);
    
    function2(4);
    
    return 0;
}

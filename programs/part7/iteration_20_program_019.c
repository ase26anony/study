/* test.c - Simple program to generate coverage data */
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

void function2() {
    for (int i = 0; i < 3; i++) {
        printf("Iteration %d\n", i);
    }
}

int main() {
    int result = function1(5);
    printf("Result: %d\n", result);
    
    function2();
    
    result = function1(-3);
    printf("Result: %d\n", result);
    
    result = function1(0);
    printf("Result: %d\n", result);
    
    return 0;
}

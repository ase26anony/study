/* test.c - Simple test program for gcov-dump */
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

void function2(void) {
    for (int i = 0; i < 3; i++) {
        printf("Loop iteration %d\n", i);
    }
}

int main(void) {
    int result = function1(5);
    printf("Result: %d\n", result);
    
    function2();
    
    return 0;
}

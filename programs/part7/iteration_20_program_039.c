/* test.c - Simple test program for gcov-dump */
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
    
    if (result1 > result2) {
        printf("Result1 is greater\n");
    } else {
        printf("Result2 is greater or equal\n");
    }
    
    return 0;
}

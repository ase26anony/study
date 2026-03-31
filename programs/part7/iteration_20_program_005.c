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

int function2(int a, int b) {
    int result = 0;
    for (int i = 0; i < a; i++) {
        result += b;
    }
    return result;
}

int main() {
    int val1 = function1(5);
    int val2 = function1(-3);
    int val3 = function2(4, 2);
    
    printf("Results: %d, %d, %d\n", val1, val2, val3);
    return 0;
}

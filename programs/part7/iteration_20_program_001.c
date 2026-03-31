/* test.c - Simple test program for gcov-dump */
#include <stdio.h>

int func1(int x) {
    if (x > 0) {
        return x * 2;
    } else {
        return x - 1;
    }
}

int func2(int y) {
    for (int i = 0; i < y; i++) {
        printf("Iteration %d\n", i);
    }
    return y * 3;
}

int main() {
    int a = 5;
    int b = -3;
    
    int result1 = func1(a);
    int result2 = func1(b);
    int result3 = func2(2);
    
    printf("Results: %d, %d, %d\n", result1, result2, result3);
    
    return 0;
}

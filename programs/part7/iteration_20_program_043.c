/* test.c - Simple test program for gcov-dump */
#include <stdio.h>

int add(int a, int b) {
    return a + b;
}

int subtract(int a, int b) {
    return a - b;
}

int main() {
    int x = 5;
    int y = 3;
    
    if (x > 0) {
        printf("x is positive\n");
        int sum = add(x, y);
        printf("Sum: %d\n", sum);
    } else {
        printf("x is non-positive\n");
    }
    
    for (int i = 0; i < 3; i++) {
        printf("Loop iteration %d\n", i);
    }
    
    return 0;
}

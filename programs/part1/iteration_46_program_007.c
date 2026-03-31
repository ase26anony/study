/* test_coverage.c - Simple program to generate .gcda files */
#include <stdio.h>

int add(int a, int b) {
    return a + b;
}

int multiply(int a, int b) {
    return a * b;
}

int main() {
    int x = 5;
    int y = 3;
    
    printf("Addition: %d + %d = %d\n", x, y, add(x, y));
    printf("Multiplication: %d * %d = %d\n", x, y, multiply(x, y));
    
    // Some conditional code for coverage
    if (x > y) {
        printf("x is greater than y\n");
    } else {
        printf("x is not greater than y\n");
    }
    
    // Loop for coverage
    for (int i = 0; i < 3; i++) {
        printf("Loop iteration: %d\n", i);
    }
    
    return 0;
}

/* test_coverage.c - Simple program to generate GCOV data */
#include <stdio.h>

void function_a() {
    printf("Function A executed\n");
}

void function_b() {
    printf("Function B executed\n");
}

int main() {
    printf("Starting coverage test program\n");
    
    function_a();
    function_b();
    
    int x = 5;
    if (x > 3) {
        printf("x is greater than 3\n");
    } else {
        printf("x is not greater than 3\n");
    }
    
    for (int i = 0; i < 2; i++) {
        printf("Loop iteration %d\n", i);
    }
    
    printf("Program completed\n");
    return 0;
}

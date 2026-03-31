/* Test 1: Integer condition modified in the then block */
#include <stdio.h>
#include <stdlib.h>

int test_modify_condition(int seed) {
    volatile int x = seed;  /* Prevent constant folding */
    int y = 0;
    
    /* Simple if-else pattern that's a candidate for if-conversion */
    if (x > 0) {
        y = 10;
        x = 5;  /* This modifies the condition variable! */
    } else {
        y = 20;
    }
    
    /* Use both variables to prevent dead code elimination */
    return y + (x & 1);
}

/* Variant with different operation */
int test_modify_condition2(int seed) {
    int a = seed;
    int b = 0;
    int c = 0;
    
    if (a != 0) {
        b = a * 2;
        a = 0;  /* Modifies condition variable */
        c = 1;
    } else {
        b = -a;
        c = 2;
    }
    
    return b + c;
}

int main(int argc, char **argv) {
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    int result = 0;
    
    result += test_modify_condition(seed);
    result += test_modify_condition2(seed);
    
    printf("Result: %d\n", result);
    return result;
}

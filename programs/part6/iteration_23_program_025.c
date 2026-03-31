/* Test 1: Integer condition modified in the then block */
#include <stdio.h>
#include <stdlib.h>

int test_modify_condition(int seed) {
    volatile int x = seed;  /* Prevent constant folding */
    int y = 0;
    
    /* Simple if-else pattern that if-conversion would like to convert */
    if (x > 0) {
        y = 10;
        x = 5;  /* MODIFIES THE CONDITION VARIABLE - should trigger modified_in_p */
    } else {
        y = 20;
    }
    
    /* Use result to prevent optimization */
    return y + x;
}

int test_modify_condition2(int seed) {
    int a = seed;
    int b = 0;
    
    /* Another pattern with different modification */
    if (a != 0) {
        b = a * 2;
        a = 0;  /* MODIFIES CONDITION - should be detected */
    } else {
        b = -1;
    }
    
    return b;
}

int main(int argc, char **argv) {
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    int result = 0;
    
    result += test_modify_condition(seed);
    result += test_modify_condition2(seed);
    
    printf("Result: %d\n", result);
    return result;
}

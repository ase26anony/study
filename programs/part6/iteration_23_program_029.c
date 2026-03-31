/* Test 1: Integer condition modified in the then block */
#include <stdio.h>
#include <stdlib.h>

int test_modify_condition(int seed) {
    volatile int x = seed;  /* Prevent constant folding */
    int y = 0;
    
    /* Condition variable x gets modified inside the then block */
    if (x > 100) {
        y = 42;
        x = 50;  /* This modifies the condition variable! */
    } else {
        y = 24;
    }
    
    /* Use both x and y to prevent dead code elimination */
    return y + (x % 10);
}

/* Variant with same destination in both arms */
int test_same_dest_modify(int seed) {
    volatile int a = seed;
    int result = 0;
    
    /* Classic if-conversion candidate with modification */
    if (a != 0) {
        result = a * 2;
        a = 0;  /* Modifies condition variable */
    } else {
        result = -1;
    }
    
    return result + a;  /* Use a to ensure it's not optimized away */
}

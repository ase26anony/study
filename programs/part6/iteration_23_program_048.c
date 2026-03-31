/* test1.c - Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Direct modification of condition variable */
int test1_modify_condition(int x, int y) {
    int result;
    int cond = x > 0;  /* Condition variable */
    
    if (cond) {
        result = y * 2;
        cond = 0;  /* MODIFIES condition variable - should trigger modified_in_p */
    } else {
        result = y / 2;
    }
    
    return result + cond;  /* Use cond to prevent dead store elimination */
}

/* Function 2: Multiple modifications in then block */
int test1_complex_modification(int a, int b) {
    int output;
    int test = (a != b);  /* Condition expression */
    
    if (test) {
        output = a + b;
        test = (a == b);  /* First modification */
        output += test;   /* Use modified value */
        test = 0;         /* Second modification */
    } else {
        output = a - b;
    }
    
    return output * (test + 1);  /* Prevent optimization */
}

/* Function 3: Condition variable used in multiple places */
int test1_volatile_cond(volatile int* cond_ptr) {
    int val = 100;
    int condition = *cond_ptr > 50;  /* Volatile read for condition */
    
    if (condition) {
        val = 200;
        condition = *cond_ptr < 25;  /* Modify condition based on volatile */
        val += condition ? 50 : 25;
    } else {
        val = 300;
    }
    
    return val;
}

/* Main test driver */
int main(int argc, char** argv) {
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    int result = 0;
    
    /* Test with varying inputs to ensure different paths */
    result += test1_modify_condition(seed, seed * 2);
    result += test1_complex_modification(seed, seed / 2);
    
    volatile int volatile_cond = seed;
    result += test1_volatile_cond(&volatile_cond);
    
    printf("Test1 result: %d\n", result);
    return result;
}

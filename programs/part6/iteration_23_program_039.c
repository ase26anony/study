/* test1.c - Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Direct modification of condition variable */
int test1_modify_in_then(int x, int y) {
    int result;
    /* Condition variable x gets modified in then block */
    if (x > 0) {
        result = y * 2;
        x = 0;  /* This modifies the condition variable! */
    } else {
        result = y / 2;
    }
    return result + x;  /* Use x to prevent optimization */
}

/* Function 2: Multiple modifications in then block */
int test1_complex_modify(int a, int b) {
    int res;
    volatile int cond = a;  /* volatile to prevent optimization */
    
    if (cond > 10) {
        res = b + 100;
        cond = cond - 5;    /* First modification */
        cond = cond * 2;    /* Second modification */
    } else {
        res = b - 100;
    }
    
    return res + (cond % 10);  /* Use cond to keep it alive */
}

/* Function 3: Condition variable used in computation then modified */
int test1_arithmetic_modify(int base, int offset) {
    int value;
    int threshold = base + offset;
    
    if (threshold < 100) {
        value = base * offset;
        threshold = threshold + 50;  /* Modify condition variable */
    } else {
        value = base / offset;
    }
    
    return value + (threshold % 20);
}

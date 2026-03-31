/* test1.c - Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Simple integer condition with modification in then block */
int test1_modify_condition_in_then(int x, int y) {
    int result;
    
    /* Condition variable x gets modified in then block */
    if (x > 0) {
        result = y * 2;
        x = 5;  /* Modifies the condition variable */
    } else {
        result = y / 2;
    }
    
    /* Use x to prevent dead store elimination */
    return result + x;
}

/* Function 2: Multiple modifications in then block */
int test1_multiple_modifications(int a, int b) {
    int res;
    
    /* a is used in condition and modified in then block */
    if (a != b) {
        res = b * 3;
        a = b;      /* First modification */
        a += 1;     /* Second modification */
    } else {
        res = b * 2;
    }
    
    return res + a;
}

/* Function 3: Condition variable used in arithmetic then modified */
int test1_arithmetic_then_modify(int x, int y) {
    int val;
    
    if (x + y > 100) {
        val = x * y;
        x = 0;  /* Modifies one part of the condition */
    } else {
        val = x + y;
    }
    
    return val - x;
}

int main(int argc, char *argv[]) {
    volatile int seed = 0;
    
    /* Use argv to prevent constant folding */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    int x = seed + 10;
    int y = seed + 20;
    
    int result = 0;
    result += test1_modify_condition_in_then(x, y);
    result += test1_multiple_modifications(x, y);
    result += test1_arithmetic_then_modify(x, y);
    
    printf("Test1 result: %d\n", result);
    return result;
}

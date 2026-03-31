/* Test program for GCC if-conversion pass coverage */
/* Targets lines 577-583 in ifcvt.cc: validation that "then" block doesn't modify condition variable */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent constant propagation */
volatile int global_cond = 1;

/* Function attribute to prevent inlining and preserve CFG */
__attribute__((noinline)) 
int test_if_conversion(int start_a, int start_b) {
    int a = start_a;
    int b = start_b;
    
    /* Volatile condition variable - prevents constant folding */
    volatile int cond = global_cond;
    
    /* Volatile loop counter to prevent unrolling */
    volatile int N = 100;
    
    for (int i = 0; i < N; i++) {
        /* Condition expression using cond (test_expr in ifcvt.cc) */
        /* This creates a candidate for if-conversion */
        if (cond > 0) {
            /* THEN BLOCK: Operations that DO NOT modify cond */
            /* These instructions should pass the validation check */
            a = b + 1;      /* Simple arithmetic on different variable */
            b = a * 2;      /* Another arithmetic operation */
            a = a ^ b;      /* Bitwise operation */
            b = b & 0xFF;   /* Mask operation */
        } else {
            /* ELSE BLOCK: Also doesn't modify cond */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify cond in a loop-variant way, but NOT in the then/else blocks */
        /* This ensures the condition changes across iterations */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Additional operations to create more RTL instructions */
        a = a + i;
        b = b - i;
    }
    
    /* Return value derived from computations to prevent dead code elimination */
    return a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
int test_if_conversion2(int x, int y) {
    volatile int flag = global_cond;
    int result = 0;
    
    for (volatile int i = 0; i < 50; i++) {
        /* Different condition expression */
        if (flag != 0) {
            /* Safe then block - no modification of flag */
            x = y << 2;
            y = x >> 1;
            result += x | y;
        } else {
            x = y + i;
            y = x - i;
            result += x & y;
        }
        
        /* Modify condition variable outside the blocks */
        flag = (flag + i) % 7;
    }
    
    return result;
}

/* Main function to drive the tests */
int main() {
    /* Initialize with non-deterministic values */
    int a = rand() % 100;
    int b = rand() % 100;
    
    /* Call test functions multiple times */
    int result1 = test_if_conversion(a, b);
    int result2 = test_if_conversion2(a, b);
    
    /* Use results to prevent optimization */
    printf("Results: %d, %d\n", result1, result2);
    
    return 0;
}

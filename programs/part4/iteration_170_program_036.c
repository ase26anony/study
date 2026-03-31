/* ifcvt_coverage.c
 * Designed to trigger GCC's if-conversion validation logic
 * for checking that instructions in the "then" block don't modify
 * the condition expression.
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent constant propagation */
volatile int global_cond = 1;

/* Function marked noinline to prevent early optimization */
__attribute__((noinline)) 
int test_if_conversion(int init_a, int init_b) {
    int a = init_a;
    int b = init_b;
    
    /* Volatile condition variable - ensures real branch */
    volatile int cond = global_cond;
    
    /* Volatile loop limit to prevent unrolling */
    volatile int N = 100;
    
    for (int i = 0; i < N; i++) {
        /* Condition expression using cond (test_expr) */
        if (cond > 0) {
            /* THEN BLOCK: Operations that DO NOT modify cond */
            /* These instructions should pass the validation check */
            a = b + 1;      /* Simple arithmetic on other variables */
            b = a * 2;      /* Another arithmetic operation */
            a = a ^ b;      /* Bitwise operation */
            b = b & 0xFF;   /* More bitwise operations */
        } else {
            /* ELSE BLOCK: Also doesn't modify cond */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify cond in a loop-variant way, but NOT in the then/else blocks */
        /* This ensures the condition changes but the validation still passes */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Additional operations to create more RTL instructions */
        a = a + i;
        b = b - i;
    }
    
    /* Return value to prevent dead code elimination */
    return a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
int test_if_conversion2(unsigned int mask) {
    unsigned int a = 0x12345678;
    unsigned int b = 0x87654321;
    volatile unsigned int cond = mask;
    
    volatile int iterations = 50;
    
    for (int i = 0; i < iterations; i++) {
        /* Different condition expression */
        if ((cond & 0xF) == 0) {
            /* THEN BLOCK: Multiple non-modifying instructions */
            a = b << 1;     /* Shift operation */
            b = a >> 2;     /* Another shift */
            a = a | b;      /* Bitwise OR */
            b = b + 0x1000; /* Arithmetic with constant */
        } else {
            /* ELSE BLOCK */
            a = b >> 1;
            b = a << 2;
        }
        
        /* Rotate cond to change condition */
        cond = (cond << 1) | (cond >> 31);
        
        /* More operations to fill the block */
        a = a ^ i;
        b = b + i;
    }
    
    return (int)(a ^ b);
}

/* Main function to drive the tests */
int main() {
    /* Initialize with random values to avoid constant folding */
    int result1 = test_if_conversion(rand() % 100, rand() % 100);
    int result2 = test_if_conversion2(rand());
    
    /* Use results to prevent optimization */
    printf("Results: %d, %d\n", result1, result2);
    
    /* Also test with edge cases */
    global_cond = 0;
    result1 = test_if_conversion(1, 2);
    
    global_cond = -1;
    result2 = test_if_conversion2(0xFFFFFFFF);
    
    printf("Edge results: %d, %d\n", result1, result2);
    
    return 0;
}

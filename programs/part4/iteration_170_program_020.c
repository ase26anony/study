/* ifcvt_coverage.c
 * Designed to trigger GCC's if-conversion validation logic
 * Specifically targets lines 577-583 in ifcvt.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent constant propagation and folding */
static volatile int external_cond = 1;
static volatile int loop_limit = 100;

/* Function marked noinline to prevent early optimization */
__attribute__((noinline))
static int process_conditional(int start_a, int start_b) {
    int a = start_a;
    int b = start_b;
    volatile int cond = external_cond;  /* Force real conditional */
    
    /* Loop with volatile limit to prevent unrolling */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* This is the test_expr - uses cond but doesn't modify it in then block */
        if (cond > 0) {
            /* THEN BLOCK: These instructions must NOT modify cond */
            /* Simple arithmetic operations on other variables */
            a = b + 1;      /* First non-label, non-debug instruction */
            b = a * 2;      /* Second non-label, non-debug instruction */
            a = a ^ b;      /* Third instruction - bitwise operation */
            b = b & 0xFF;   /* Fourth instruction - mask operation */
            
            /* Add more safe operations to ensure block is non-trivial */
            a = a + (b >> 3);
            b = b - (a & 0xF);
        } else {
            /* ELSE BLOCK: Also doesn't modify cond */
            a = b - 1;
            b = a / 2;
            a = a | b;
            b = b % 256;
        }
        
        /* Modify cond AFTER the conditional block */
        /* This ensures cond is not modified in the then block */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Prevent loop optimization */
        asm volatile("" : "+r" (a), "+r" (b));
    }
    
    return a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
static int test_comparison(int x, int y) {
    volatile int threshold = 50;
    int result = 0;
    
    for (volatile int i = 0; i < 50; i++) {
        /* Different condition expression */
        if (x < threshold) {
            /* Safe then block - doesn't modify x, y, or threshold */
            result = result + y;
            y = y * 2;
            result = result ^ y;
            y = y & 0xFFFF;
        } else {
            result = result - y;
            y = y / 2;
        }
        
        /* Modify condition variables outside the then block */
        x = (x * 3 + 7) & 0xFF;
        threshold = (threshold + i) & 0xFF;
    }
    
    return result;
}

int main(void) {
    /* Initialize with non-constant values */
    int a = rand() % 100;
    int b = rand() % 100;
    
    /* First test case */
    int result1 = process_conditional(a, b);
    
    /* Second test case */
    int x = rand() % 100;
    int y = rand() % 100;
    int result2 = test_comparison(x, y);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    return (result1 + result2) != 0 ? 0 : 1;
}

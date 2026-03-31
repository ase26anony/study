/* ifcvt_coverage.c
 * Designed to trigger GCC's if-conversion pass validation logic
 * Specifically targets lines 577-583 in ifcvt.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent constant propagation and folding */
volatile int external_cond = 1;

/* Prevent function inlining to preserve CFG */
__attribute__((noinline)) 
static int process_conditional(int start_a, int start_b) {
    volatile int cond = external_cond;  /* Force memory read */
    int a = start_a;
    int b = start_b;
    int result = 0;
    
    /* Volatile loop counter to prevent unrolling */
    volatile int iterations = 100;
    
    for (int i = 0; i < iterations; i++) {
        /* This is the test_expr - uses cond but doesn't modify it in then block */
        if (cond > 0) {
            /* THEN BLOCK: Operations that do NOT modify cond */
            /* These should be non-label, non-debug instructions */
            a = b + 1;      /* First arithmetic operation */
            b = a * 2;      /* Second arithmetic operation */
            a = a ^ b;      /* Bitwise operation */
            b = b & 0xFF;   /* Another bitwise operation */
            
            /* More safe operations to ensure block is non-trivial */
            result += a;
            result -= b;
        } else {
            /* ELSE BLOCK: Also doesn't modify cond */
            a = b - 1;
            b = a / 2;
            result += b;
        }
        
        /* Modify cond AFTER the conditional block */
        /* This ensures cond is modified, but not within the then block */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Prevent loop optimization */
        asm volatile("" : "+r" (i) : : "memory");
    }
    
    return result + a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
static int test_comparison(int x, int y) {
    volatile int threshold = 50;
    int acc = 0;
    
    for (volatile int i = 0; i < 50; i++) {
        /* Different condition expression */
        if (x < threshold) {
            /* Safe then block - modifies x and y but not threshold */
            x = y + i;
            y = x * i;
            acc += x - y;
        } else {
            x = y - i;
            acc += x;
        }
        
        /* Modify threshold outside conditional */
        threshold = (threshold + i) % 100;
    }
    
    return acc;
}

int main(void) {
    /* Initialize with non-deterministic values */
    int a = rand() % 100;
    int b = rand() % 100;
    
    /* Call the function with if-conversion candidate */
    int result1 = process_conditional(a, b);
    
    /* Another test case */
    int result2 = test_comparison(a, b);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    return (result1 + result2) != 0 ? 0 : 1;
}

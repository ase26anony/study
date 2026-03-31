/* ifcvt_coverage.c
 * Designed to trigger GCC's if-conversion pass validation logic
 * Specifically targets lines 577-583 in ifcvt.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent constant propagation and folding */
volatile int external_cond = 1;

/* Non-inline function to preserve control flow */
__attribute__((noinline)) 
int ifcvt_candidate(int start_a, int start_b) {
    int a = start_a;
    int b = start_b;
    
    /* Volatile variable prevents loop unrolling */
    volatile int iterations = 100;
    
    /* Use volatile condition to force real branch */
    volatile int cond = external_cond;
    
    for (int i = 0; i < iterations; i++) {
        /* This is the test_expr - condition variable 'cond' is read but not modified in then block */
        if (cond > 0) {
            /* THEN BLOCK: Instructions that do NOT modify 'cond'
             * These will be validated by the uncovered logic
             * Each line generates non-label, non-debug instructions
             */
            a = b + 1;      /* First arithmetic operation */
            b = a * 2;      /* Second arithmetic operation */
            a = a ^ b;      /* Bitwise operation */
            b = b & 0xFF;   /* Mask operation */
        } else {
            /* ELSE BLOCK: Also doesn't modify 'cond' */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify condition variable OUTSIDE the conditional blocks
         * This ensures 'cond' changes across iterations but isn't modified
         * in the then/else blocks being validated
         */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Prevent loop optimization */
        asm volatile("" : : "r"(a), "r"(b));
    }
    
    /* Return value derived from computations to prevent dead code elimination */
    return a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
int ifcvt_candidate2(int x, int y) {
    volatile int flag = external_cond;
    int result = 0;
    
    for (volatile int i = 0; i < 50; i++) {
        /* Test with different comparison operator */
        if (flag != 0) {
            /* Multiple safe operations in then block */
            x = y << 1;
            y = x | 0x1;
            result += x - y;
        } else {
            y = x >> 1;
            result -= y;
        }
        
        /* Change condition variable outside the blocks */
        flag = flag ^ (i + 1);
    }
    
    return result;
}

int main(void) {
    /* Initialize with random values to prevent constant folding */
    int seed = external_cond;
    srand(seed);
    
    int a = rand() % 100;
    int b = rand() % 100;
    
    /* Call the if-conversion candidate functions */
    int result1 = ifcvt_candidate(a, b);
    int result2 = ifcvt_candidate2(a, b);
    
    /* Use results to prevent optimization */
    printf("Results: %d, %d\n", result1, result2);
    
    return (result1 + result2) > 0 ? 0 : 1;
}

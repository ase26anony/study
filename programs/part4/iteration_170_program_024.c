/* ifcvt_coverage.c
 * Designed to trigger GCC's if-conversion pass validation logic
 * for checking that instructions in the "then" block don't modify
 * the condition expression.
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent constant propagation */
volatile int global_cond = 1;

/* Function marked noinline to prevent early optimization */
__attribute__((noinline))
int ifcvt_candidate(int start_a, int start_b) {
    int a = start_a;
    int b = start_b;
    
    /* Volatile variable for loop limit to prevent unrolling */
    volatile int N = 100;
    
    /* Local volatile copy of global condition */
    volatile int cond = global_cond;
    
    for (int i = 0; i < N; i++) {
        /* This is the test_expr - uses cond but doesn't modify it */
        if (cond > 0) {
            /* THEN BLOCK: Operations that DO NOT modify cond */
            /* These instructions should pass the validation check */
            a = b + 1;      /* First non-label, non-debug instruction */
            b = a * 2;      /* Second instruction */
            a = a ^ b;      /* Third instruction - bitwise operation */
            b = b & 0xFF;   /* Fourth instruction */
            /* All these modify a and b, but NOT cond */
        } else {
            /* ELSE BLOCK: Also doesn't modify cond */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify cond for next iteration (loop-variant but safe) */
        /* This happens OUTSIDE the if-else blocks */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Prevent loop from being optimized away */
        if (i % 10 == 0) {
            global_cond = cond;
        }
    }
    
    /* Return value derived from a and b to prevent dead code elimination */
    return a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
int ifcvt_candidate2(int x, int y) {
    volatile int test_var = x;
    int result = y;
    
    /* Multiple condition variables to increase coverage */
    volatile int cond1 = test_var & 1;
    volatile int cond2 = (test_var >> 1) & 1;
    
    /* Loop with volatile limit */
    volatile int iterations = 50;
    
    for (int i = 0; i < iterations; i++) {
        /* Compound condition using cond1 and cond2 */
        if (cond1 && (cond2 || (i % 2 == 0))) {
            /* THEN BLOCK: Multiple instructions that don't modify cond1/cond2 */
            result = result + x;
            x = x * 3;
            result = result ^ x;
            y = y | 0x0F;
            /* Still not modifying cond1 or cond2 */
        } else {
            /* ELSE BLOCK */
            result = result - y;
            x = x >> 1;
        }
        
        /* Modify condition variables outside the if block */
        cond1 = (cond1 + i) & 1;
        cond2 = (cond2 * 3) & 1;
        
        /* Use result to prevent optimization */
        if (i % 7 == 0) {
            global_cond = result;
        }
    }
    
    return result;
}

int main(void) {
    /* Initialize with non-constant values */
    int a = rand() % 100;
    int b = rand() % 100;
    
    /* Call the if-conversion candidate functions */
    int result1 = ifcvt_candidate(a, b);
    int result2 = ifcvt_candidate2(a, b);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    return (result1 + result2) > 0 ? 0 : 1;
}

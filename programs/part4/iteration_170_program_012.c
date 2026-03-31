/* ifcvt_coverage.c
 * Designed to trigger GCC's if-conversion pass validation logic
 * for the "then" block safety check in ifcvt.cc lines 577-583
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
    
    /* Volatile iteration limit to prevent loop unrolling */
    volatile int N = 100;
    
    /* Loop to increase chances of if-conversion analysis */
    for (int i = 0; i < N; i++) {
        /* Read condition from volatile global - prevents constant folding */
        int cond = global_cond;
        
        /* The test expression - condition variable is 'cond' */
        if (cond > 0) {
            /* THEN BLOCK: Operations that DO NOT modify 'cond' */
            /* These instructions should pass the safety check */
            a = b + 1;      /* First non-label, non-debug instruction */
            b = a * 2;      /* Second non-label, non-debug instruction */
            a = a ^ b;      /* Third instruction - bitwise operation */
            b = b & 0xFF;   /* Fourth instruction - masking operation */
        } else {
            /* ELSE BLOCK: Also doesn't modify 'cond' */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify condition variable OUTSIDE the if block */
        /* This ensures 'cond' changes but isn't modified in the then block */
        global_cond = (global_cond * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Return value derived from computations to prevent dead code elimination */
    return a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
int ifcvt_candidate2(int x, int y) {
    volatile int threshold = 500;
    int result = 0;
    
    for (int i = 0; i < 50; i++) {
        /* Complex enough condition to avoid simplification */
        int test_var = (x * y + i) & 0xFF;
        
        /* Conditional with safe then block */
        if (test_var < threshold) {
            /* Multiple instructions that don't modify test_var */
            x = y << 2;
            y = x | 0x0F;
            result += x - y;
            x = x ^ result;
        } else {
            y = x >> 1;
            x = y + result;
        }
        
        /* Modify variables used in condition */
        x = (x + 1) % 256;
        y = (y * 3) % 256;
    }
    
    return result;
}

int main() {
    /* Initialize with random values to create varying conditions */
    int a = rand() % 1000;
    int b = rand() % 1000;
    
    /* Call the if-conversion candidate functions */
    int result1 = ifcvt_candidate(a, b);
    int result2 = ifcvt_candidate2(a, b);
    
    /* Use results to prevent optimization */
    printf("Results: %d, %d\n", result1, result2);
    
    return (result1 + result2) > 0 ? 0 : 1;
}

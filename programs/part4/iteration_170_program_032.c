/* ifcvt_coverage.c
 * Designed to trigger GCC's if-conversion validation logic
 * Specifically targets lines 577-583 in ifcvt.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent constant propagation */
volatile int global_cond = 1;

/* Prevent inlining to preserve control flow */
__attribute__((noinline))
int ifcvt_candidate(int start_a, int start_b) {
    int a = start_a;
    int b = start_b;
    
    /* Volatile iteration count to prevent loop unrolling */
    volatile int iterations = 100;
    
    /* Loop to make the block interesting for if-conversion */
    for (int i = 0; i < iterations; i++) {
        /* Condition variable - read from volatile to prevent constant folding */
        int cond = global_cond;
        
        /* The critical if-structure:
         * - Condition uses 'cond' (test_expr)
         * - Then block does NOT modify 'cond'
         * - Contains non-label, non-debug instructions
         */
        if (cond > 0) {
            /* THEN BLOCK: Safe operations that don't modify 'cond' */
            /* These will become the instructions between BB_HEAD and then_last_head */
            a = b + 1;      /* First non-label, non-debug instruction */
            b = a * 2;      /* Second instruction */
            a = a ^ b;      /* Third instruction - bitwise operation */
            b = b & 0xFF;   /* Fourth instruction */
        } else {
            /* ELSE BLOCK: Also safe, doesn't modify 'cond' */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify condition variable OUTSIDE the if-block
         * This ensures it's not modified between BB_HEAD and then_last_head
         * but changes across iterations to prevent dead code elimination */
        global_cond = (global_cond * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Return value derived from computations to prevent elimination */
    return a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
int ifcvt_candidate2(int x, int y) {
    volatile int external = rand() % 100;
    int result = 0;
    
    for (volatile int i = 0; i < 50; i++) {
        int test_var = external + i;
        
        /* Different condition expression */
        if (test_var & 1) {  /* test_expr is (test_var & 1) */
            /* Safe then block - multiple instructions that don't modify test_var */
            x = y << 1;
            y = x | 0x1;
            x = x + y;
            result += x;
        } else {
            y = x >> 1;
            x = y - 1;
            result += y;
        }
        
        /* Modify external to change condition */
        external = (external * 1664525 + 1013904223) & 0x7fffffff;
    }
    
    return result;
}

/* Main function to drive execution */
int main() {
    /* Seed random for variability */
    srand(42);
    
    /* Call first candidate */
    int res1 = ifcvt_candidate(1, 2);
    printf("Result 1: %d\n", res1);
    
    /* Call second candidate */
    int res2 = ifcvt_candidate2(10, 20);
    printf("Result 2: %d\n", res2);
    
    /* Use results to prevent dead code elimination */
    return (res1 + res2) > 0 ? 0 : 1;
}

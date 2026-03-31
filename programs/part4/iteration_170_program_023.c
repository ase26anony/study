/* ifcvt_coverage.c
 * Designed to trigger uncovered lines 577-583 in ifcvt.cc
 * Compile with: gcc -O2 -fdump-rtl-ifcvt -fno-tree-loop-if-convert -fno-tree-loop-if-convert-stores ifcvt_coverage.c -o ifcvt_coverage
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent constant propagation */
volatile int global_cond = 1;

/* Function marked noinline to prevent inlining and CFG simplification */
__attribute__((noinline)) 
int ifcvt_candidate(int start_a, int start_b) {
    int a = start_a;
    int b = start_b;
    
    /* Volatile iteration limit to prevent loop unrolling */
    volatile int N = 100;
    
    /* Loop to increase chances of if-conversion analysis */
    for (int i = 0; i < N; i++) {
        /* Read condition from volatile global - this creates the test_expr */
        int cond = global_cond;
        
        /* Conditional branch with test_expr (cond > 0) */
        if (cond > 0) {
            /* THEN BLOCK: Operations that DO NOT modify 'cond' 
             * This is crucial - these lines should NOT trigger modified_in_p(test_expr, insn)
             * Contains non-label, non-debug instructions as required
             */
            a = b + 1;      /* First arithmetic operation */
            b = a * 2;      /* Second arithmetic operation */
            a = a ^ b;      /* Bitwise operation */
            b = b & 0xFF;   /* Another bitwise operation */
        } else {
            /* ELSE BLOCK: Also doesn't modify 'cond' */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify cond for next iteration (but doesn't affect current test_expr) */
        /* This update is outside the conditional blocks */
        global_cond = (global_cond * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Return value derived from a and b to prevent dead code elimination */
    return a + b;
}

/* Another test case with different pattern */
__attribute__((noinline))
int ifcvt_candidate2(int x, int y) {
    volatile int trigger = rand() % 100;
    int result = 0;
    
    for (int i = 0; i < 50; i++) {
        /* test_expr uses 'trigger' */
        if (trigger < 50) {
            /* THEN block with multiple non-modifying operations */
            x = y << 2;     /* Shift operation */
            y = x | 0x0F;   /* Bitwise OR */
            x = x + y;      /* Addition */
            result += x;    /* Accumulate result */
        } else {
            y = x >> 1;
            x = y - 1;
            result += y;
        }
        
        /* Modify trigger for next iteration */
        trigger = (trigger * 1664525 + 1013904223) & 0x7fffffff;
    }
    
    return result;
}

int main() {
    /* Seed random for variability */
    srand(42);
    
    /* Call first candidate function */
    int res1 = ifcvt_candidate(10, 20);
    
    /* Call second candidate function */
    int res2 = ifcvt_candidate2(5, 15);
    
    /* Print results to prevent optimization */
    printf("Result 1: %d\n", res1);
    printf("Result 2: %d\n", res2);
    
    /* Also test with different inputs */
    printf("Test 2: %d\n", ifcvt_candidate(100, 200));
    printf("Test 3: %d\n", ifcvt_candidate2(0, 1));
    
    return 0;
}

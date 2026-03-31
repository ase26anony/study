/* ifcvt_coverage.c
 * Designed to trigger uncovered lines 577-583 in ifcvt.cc
 * Compile with: gcc -O2 -fdump-rtl-ifcvt -S ifcvt_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent constant propagation */
volatile int global_cond = 1;

/* Function marked noinline to prevent early optimization */
__attribute__((noinline)) 
static int process_conditional(int start_a, int start_b) {
    int a = start_a;
    int b = start_b;
    
    /* Volatile condition variable - ensures real branch */
    volatile int cond = global_cond;
    
    /* Loop with volatile limit to prevent unrolling */
    volatile int iterations = 100;
    
    for (int i = 0; i < iterations; i++) {
        /* Condition expression using cond (test_expr) */
        if (cond > 0) {
            /* THEN BLOCK: Operations that do NOT modify cond */
            /* These instructions should pass the validation check */
            a = b + 1;      /* First non-label, non-debug instruction */
            b = a * 2;      /* Second instruction */
            a = a ^ b;      /* Third instruction - bitwise operation */
            b = b & 0xFF;   /* Fourth instruction */
            
            /* More safe operations to ensure block has multiple insns */
            a = a + (b << 2);
            b = b - (a >> 1);
        } else {
            /* ELSE BLOCK: Also doesn't modify cond */
            a = b - 1;
            b = a / 2;
        }
        
        /* Modify cond for next iteration (but NOT in the then block!) */
        /* This ensures the condition changes across iterations */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Prevent loop from being optimized away */
        global_cond = cond;
    }
    
    /* Return value based on computations to prevent DCE */
    return a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
static int test_comparison(int x, int y) {
    volatile int threshold = 50;
    int result = 0;
    
    /* Different condition expression */
    if (x < threshold) {
        /* Safe then block - doesn't modify x or threshold */
        result = y * 3;
        result = result + (y << 1);
        result = result ^ 0xABCD;
        y = y + result;
    } else {
        result = y / 2;
    }
    
    return result;
}

int main(void) {
    int total = 0;
    
    /* Seed random for variability */
    srand(42);
    
    /* Test case 1: Main conditional in loop */
    total += process_conditional(rand() % 100, rand() % 100);
    
    /* Test case 2: Different condition pattern */
    total += test_comparison(rand() % 100, rand() % 100);
    
    /* Test case 3: Multiple condition variables */
    {
        volatile int cond1 = rand() % 10;
        volatile int cond2 = rand() % 10;
        int a = 0, b = 0;
        
        for (volatile int i = 0; i < 50; i++) {
            if (cond1 == cond2) {
                /* Safe then block */
                a = b + cond1;    /* Uses but doesn't modify cond1 */
                b = a * cond2;    /* Uses but doesn't modify cond2 */
                a = a | b;
                b = b ^ a;
            } else {
                a = b - cond1;
                b = a + cond2;
            }
            
            /* Update conditions outside then block */
            cond1 = (cond1 + 3) % 7;
            cond2 = (cond2 * 5) % 11;
        }
        total += a + b;
    }
    
    printf("Result: %d\n", total);
    return total;
}

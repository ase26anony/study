/* Compile with: gcc -O2 -fno-schedule-insns -fno-schedule-insns2 -fno-peephole2 -o test test.c */
/* For debugging: gcc -O1 -dP -fdump-rtl-early_remat -o test test.c */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable helper to prevent optimization */
static int __attribute__((noinline)) helper(int x) {
    volatile int dummy = 0;
    return x + dummy;
}

/* Volatile variable to create side effects */
static volatile int volatile_sink;

/* Test function designed to create high register pressure */
static int test_function(int iterations) {
    /* Declare many variables to create pseudo registers */
    register int v0 asm ("r12") = 1;
    int v1 = 2, v2 = 3, v3 = 4, v4 = 5;
    int v5 = 6, v6 = 7, v7 = 8, v8 = 9, v9 = 10;
    int v10 = 11, v11 = 12, v12 = 13, v13 = 14, v14 = 15;
    int v15 = 16, v16 = 17, v17 = 18, v18 = 19, v19 = 20;
    int v20 = 21, v21 = 22, v22 = 23, v23 = 24, v24 = 25;
    
    /* Use volatile to prevent dead code elimination */
    volatile_sink = v0;
    
    /* Complex computation with overlapping live ranges */
    for (int i = 0; i < iterations; i++) {
        /* Create def-use chains across operations */
        v1 = v0 + v1 * 3;
        v2 = v1 - v2 + helper(i);
        v3 = v2 * v3 - v0;
        v4 = v3 + v4 / 2;
        v5 = v4 ^ v5;
        v6 = v5 + v6 * 7;
        v7 = v6 - v7 + v1;
        v8 = v7 * v8;
        v9 = v8 + v9 - v2;
        v10 = v9 ^ v10;
        
        /* More operations to increase pressure */
        v11 = v10 + v11 * 11;
        v12 = v11 - v12 + v3;
        v13 = v12 * v13;
        v14 = v13 + v14 / 3;
        v15 = v14 ^ v15;
        v16 = v15 + v16 * 13;
        v17 = v16 - v17 + v4;
        v18 = v17 * v18;
        v19 = v18 + v19 - v5;
        v20 = v19 ^ v20;
        
        /* Conditional to create basic block boundaries */
        if (i & 1) {
            v21 = v20 + v21 * 17;
            v22 = v21 - v22 + v6;
        } else {
            v21 = v20 - v21;
            v22 = v21 * v22;
        }
        
        v23 = v22 + v23 * 19;
        v24 = v23 - v24 + v7;
        
        /* Use asm to clobber registers and increase pressure */
        asm volatile ("# Dummy asm" : : "r"(v0), "r"(v8), "r"(v16) : "memory");
        
        /* Force register reuse with different values */
        v0 = v24 ^ 0x1234;
    }
    
    /* Mix all results to ensure they're live at the end */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                 v20 + v21 + v22 + v23 + v24;
    
    return result;
}

int main() {
    int total = 0;
    
    /* Call with different parameters to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_function(i % 10 + 5);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

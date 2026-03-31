/* Test program for GCC early rematerialization pass
 * Targets uncovered lines in early-remat.cc:930-937
 */

#include <stdio.h>
#include <stdlib.h>

/* Non-inlineable helper to create side effects */
static int __attribute__((noinline)) side_effect(int x) {
    static volatile int counter = 0;
    counter += x;
    return counter & 1;
}

/* Volatile memory access to prevent optimization */
volatile int global_seed = 42;

/* Main test function with high register pressure */
static int __attribute__((noinline)) test_rematerialization(int iterations) {
    /* Declare many variables to create pseudo registers */
    register int r0 asm ("r12") = iterations;
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Initialize with volatile to prevent constant propagation */
    v1 = global_seed + 1;
    v2 = global_seed * 2;
    v3 = global_seed - 5;
    
    /* Complex computation with overlapping live ranges */
    for (int i = 0; i < iterations; i++) {
        /* Create long dependency chains */
        v4 = v1 + v2 + side_effect(i);
        v5 = v2 * v3 - v4;
        v6 = v3 + v4 * v5;
        v7 = v4 - v5 / (v6 + 1);
        v8 = v5 ^ v6 | v7;
        v9 = v6 + v7 * v8;
        v10 = v7 - v8 + v9;
        
        v11 = v8 * v9 + v10;
        v12 = v9 - v10 * v11;
        v13 = v10 + v11 - v12;
        v14 = v11 * v12 / (v13 + 1);
        v15 = v12 ^ v13 | v14;
        v16 = v13 + v14 * v15;
        v17 = v14 - v15 + v16;
        v18 = v15 * v16 + v17;
        v19 = v16 - v17 * v18;
        v20 = v17 + v18 - v19;
        
        /* Conditional to create basic block boundaries */
        if (v20 & 1) {
            v21 = v18 * v19 + v20;
            v22 = v19 - v20 * v21;
            v23 = v20 + v21 - v22;
        } else {
            v21 = v18 + v19 * v20;
            v22 = v19 * v20 - v21;
            v23 = v20 - v21 + v22;
        }
        
        /* More computations with register variable */
        v24 = v21 * v22 + v23 + r0;
        v25 = v22 - v23 * v24;
        v26 = v23 + v24 - v25;
        v27 = v24 * v25 / (v26 + 1);
        v28 = v25 ^ v26 | v27;
        v29 = v26 + v27 * v28;
        v30 = v27 - v28 + v29;
        
        /* Rotate values to maintain liveness across iterations */
        v1 = v28 + side_effect(v30);
        v2 = v29 * 3;
        v3 = v30 - 7;
        
        /* Use asm to clobber registers and increase pressure */
        asm volatile ("# Dummy asm" : : "r" (v1), "r" (v2), "r" (v3) : "memory");
    }
    
    /* Final computation using all variables */
    int result = (v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                  v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                  v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30);
    
    return result & 0xFF; /* Prevent overflow */
}

int main() {
    int total = 0;
    
    /* Call with different parameters to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_rematerialization(10 + (i % 5));
        global_seed += i; /* Change volatile to affect computation */
    }
    
    printf("Result: %d\n", total);
    return 0;
}

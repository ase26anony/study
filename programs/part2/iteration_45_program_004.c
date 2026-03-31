/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass logic
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Non-inlineable helper function to prevent optimization */
static int __attribute__((noinline)) compute_offset(int x) {
    volatile int dummy = 0;
    return x + (dummy ? 0 : 1);
}

/* Function to create register pressure and complex data flow */
static int __attribute__((noinline)) create_register_pressure(int seed) {
    /* Declare many variables to create register pressure */
    register int v0 asm ("r12") = seed + 1;
    register int v1 asm ("r13") = seed + 2;
    int v2 = seed + 3;
    int v3 = seed + 4;
    int v4 = seed + 5;
    int v5 = seed + 6;
    int v6 = seed + 7;
    int v7 = seed + 8;
    int v8 = seed + 9;
    int v9 = seed + 10;
    int v10 = seed + 11;
    int v11 = seed + 12;
    int v12 = seed + 13;
    int v13 = seed + 14;
    int v14 = seed + 15;
    int v15 = seed + 16;
    int v16 = seed + 17;
    int v17 = seed + 18;
    int v18 = seed + 19;
    int v19 = seed + 20;
    
    /* Volatile variable to create side effects */
    volatile int barrier = 0;
    
    /* Complex computation with dependent operations across variables */
    /* This creates long live ranges and def-use chains */
    for (int i = 0; i < 100; i++) {
        /* Use barrier to prevent reordering */
        barrier = i;
        
        /* Chain of dependent computations */
        v0 = v1 + v2 + barrier;
        v1 = v0 * v3 - barrier;
        v2 = v1 / (v4 + 1) + barrier;
        v3 = v2 ^ v5 ^ barrier;
        v4 = v3 | v6 | barrier;
        v5 = v4 & v7 & ~barrier;
        v6 = v5 + v8 + compute_offset(barrier);
        v7 = v6 - v9 - barrier;
        v8 = v7 * v10 * (barrier + 1);
        v9 = v8 % (v11 + 2) + barrier;
        v10 = v9 << (v12 & 3) << barrier;
        v11 = v10 >> (v13 % 4) >> barrier;
        v12 = v11 ^ v14 ^ compute_offset(barrier);
        v13 = v12 | v15 | barrier;
        v14 = v13 & v16 & ~barrier;
        v15 = v14 + v17 + barrier;
        v16 = v15 - v18 - compute_offset(barrier);
        v17 = v16 * v19 * (barrier + 1);
        v18 = v17 % (v0 + 3) + barrier;
        v19 = v18 << (v1 & 2) << barrier;
        
        /* Conditional to create different basic blocks */
        if (i % 3 == 0) {
            v0 = v19 + v2;
            v1 = v0 * v3;
        } else if (i % 3 == 1) {
            v2 = v1 - v4;
            v3 = v2 / (v5 + 1);
        } else {
            v4 = v3 | v6;
            v5 = v4 & v7;
        }
        
        /* Use asm to clobber registers and increase pressure */
        asm volatile ("# Dummy asm" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3));
    }
    
    /* Final computation using all variables */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
    
    /* Another asm to ensure variables are used */
    asm volatile ("# Result: %0" : : "r"(result));
    
    return result;
}

/* Additional function to create more complex call graph */
static int __attribute__((noinline)) nested_computation(int x) {
    int a = x * 2;
    int b = x + 3;
    int c = x - 4;
    int d = x / 5;
    int e = x % 6;
    
    /* Create cross-block dependencies */
    for (int i = 0; i < 50; i++) {
        volatile int sync = i;
        a = b + c + sync;
        b = c - d - sync;
        c = d * e * (sync + 1);
        d = e / (a + 2) + sync;
        e = a % (b + 3) + sync;
        
        if (i % 2 == 0) {
            a = create_register_pressure(e);
        }
    }
    
    return a + b + c + d + e;
}

int main(void) {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 10; i++) {
        int result1 = create_register_pressure(i * 100);
        int result2 = nested_computation(i * 50);
        total += result1 + result2;
        
        /* Print to prevent dead code elimination */
        printf("Iteration %d: %d + %d = %d\n", 
               i, result1, result2, result1 + result2);
    }
    
    printf("Total: %d\n", total);
    return total != 0 ? 0 : 1;
}

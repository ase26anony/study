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

/* Function to create high register pressure and rematerialization candidates */
static int __attribute__((noinline)) test_remat_pressure(int seed) {
    /* Declare many variables to create pseudo registers */
    register int r0 asm ("r12") = seed + 1;
    int v1 = seed * 2;
    int v2 = seed / 3;
    int v3 = seed ^ 0x55AA;
    int v4 = seed << 2;
    int v5 = seed >> 1;
    int v6 = seed | 0xFF00;
    int v7 = seed & 0x00FF;
    int v8 = seed + 100;
    int v9 = seed - 50;
    int v10 = seed * 3;
    int v11 = seed / 2;
    int v12 = seed ^ 0x1234;
    int v13 = seed << 1;
    int v14 = seed >> 2;
    int v15 = seed | 0xAAAA;
    int v16 = seed & 0x5555;
    int v17 = seed + 200;
    int v18 = seed - 100;
    int v19 = seed * 5;
    int v20 = seed / 4;
    
    /* Volatile variable to prevent optimization */
    volatile int barrier = 0;
    
    /* Complex computation with overlapping live ranges */
    for (int i = 0; i < 100; i++) {
        /* Create def-use chains across operations */
        r0 = r0 + v1 + side_effect(i);
        v1 = v2 * v3 - r0;
        v2 = v3 ^ v4 + v1;
        v3 = v4 | v5 * v2;
        v4 = v5 & v6 - v3;
        v5 = v6 + v7 ^ v4;
        v6 = v7 - v8 | v5;
        v7 = v8 * v9 & v6;
        v8 = v9 / v10 + v7;
        v9 = v10 ^ v11 - v8;
        v10 = v11 << v12 | v9;
        v11 = v12 >> v13 & v10;
        v12 = v13 + v14 ^ v11;
        v13 = v14 - v15 | v12;
        v14 = v15 * v16 & v13;
        v15 = v16 / v17 + v14;
        v16 = v17 ^ v18 - v15;
        v17 = v18 << v19 | v16;
        v18 = v19 >> v20 & v17;
        v19 = v20 + r0 ^ v18;
        v20 = r0 - v1 | v19;
        
        /* Use asm to clobber registers and increase pressure */
        asm volatile ("" : : "r"(r0), "r"(v1), "r"(v2), "r"(v3), 
                      "r"(v4), "r"(v5), "r"(v6), "r"(v7) : 
                      "memory");
        
        /* Conditional assignment creating branching live ranges */
        if (i & 1) {
            r0 = v1 + v2;
            barrier = i;  /* Volatile access */
        } else {
            r0 = v3 - v4;
            barrier = i * 2;
        }
        
        /* Use results in subsequent iteration to maintain liveness */
        v1 = v1 + (r0 & 0xFF);
        v2 = v2 ^ (r0 >> 8);
    }
    
    /* Mix all results to ensure they're used */
    int result = r0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    
    return result & 0xFFFF;  /* Prevent overflow issues */
}

/* Main function with varying parameters */
int main(void) {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 10; i++) {
        int result = test_remat_pressure(i * 100 + 12345);
        total += result;
        printf("Iteration %d: result = %d, total = %d\n", i, result, total);
    }
    
    /* Use result to prevent dead code elimination */
    if (total > 1000) {
        printf("Final total: %d\n", total);
    } else {
        printf("Unexpected low total: %d\n", total);
    }
    
    return total > 0 ? 0 : 1;
}

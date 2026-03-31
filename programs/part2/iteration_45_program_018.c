/* Test program for GCC early rematerialization pass
 * Targets uncovered lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Non-inlineable helper to create side effects */
static int __attribute__((noinline)) side_effect(int x) {
    static volatile int counter = 0;
    counter++;
    return x + (counter & 1);
}

/* Main test function with high register pressure */
static int __attribute__((noinline)) test_remat(int seed) {
    /* Declare many variables to create register pressure */
    register int v0 asm ("r12") = seed;
    int v1 = seed + 1;
    int v2 = seed * 2;
    int v3 = seed / 3;
    int v4 = seed ^ 0x55;
    int v5 = seed | 0xAA;
    int v6 = seed & 0xF0;
    int v7 = seed << 2;
    int v8 = seed >> 1;
    int v9 = ~seed;
    int v10 = seed + 100;
    int v11 = seed - 50;
    int v12 = seed * 3;
    int v13 = seed / 2;
    int v14 = seed ^ 0xFF;
    int v15 = seed | 0xCC;
    int v16 = seed & 0x0F;
    int v17 = seed << 1;
    int v18 = seed >> 2;
    int v19 = seed + 200;
    int v20 = seed - 100;
    int v21 = seed * 5;
    int v22 = seed / 4;
    int v23 = seed ^ 0xAA;
    int v24 = seed | 0x55;
    int v25 = seed & 0xF0;
    int v26 = seed << 3;
    int v27 = seed >> 3;
    int v28 = seed + 300;
    int v29 = seed - 150;
    
    /* Volatile variable to prevent optimization */
    volatile int barrier = 0;
    
    /* Complex computation with dependent operations
     * Creates long live ranges and def-use chains */
    for (int i = 0; i < 100; i++) {
        /* Mix register and stack variables */
        v0 = v1 + v2;
        v1 = v0 * v3;
        v2 = v1 - v4;
        v3 = v2 ^ v5;
        v4 = v3 | v6;
        v5 = v4 & v7;
        v6 = v5 << 1;
        v7 = v6 >> 1;
        v8 = v7 + v9;
        v9 = v8 * v10;
        v10 = v9 - v11;
        v11 = v10 ^ v12;
        v12 = v11 | v13;
        v13 = v12 & v14;
        v14 = v13 << 2;
        v15 = v14 >> 2;
        v16 = v15 + v17;
        v17 = v16 * v18;
        v18 = v17 - v19;
        v19 = v18 ^ v20;
        v20 = v19 | v21;
        v21 = v20 & v22;
        v22 = v21 << 1;
        v23 = v22 >> 1;
        v24 = v23 + v25;
        v25 = v24 * v26;
        v26 = v25 - v27;
        v27 = v26 ^ v28;
        v28 = v27 | v29;
        v29 = v28 & v0;
        
        /* Introduce side effect that can't be moved */
        barrier = side_effect(i);
        
        /* Use results in conditional to maintain liveness */
        if (barrier & 1) {
            v0 += v15;
            v1 += v16;
            v2 += v17;
        } else {
            v0 -= v18;
            v1 -= v19;
            v2 -= v20;
        }
        
        /* More arithmetic to create overlapping live ranges */
        v3 = v4 * v5 + v6;
        v4 = v7 / (v8 + 1) + v9;
        v5 = v10 ^ v11 | v12;
        v6 = v13 & v14 << v15;
        v7 = v16 + v17 - v18;
        v8 = v19 * v20 / (v21 + 1);
        v9 = v22 | v23 ^ v24;
        v10 = v25 & v26 >> v27;
        
        /* Inline asm with clobbers to increase register pressure */
        asm volatile (
            "addl %1, %0\n\t"
            "subl %2, %0\n\t"
            : "+r" (v11)
            : "r" (v12), "r" (v13)
            : "cc"
        );
        
        /* Another asm to create pseudo register references */
        asm volatile (
            "movl %1, %%eax\n\t"
            "imull %2, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (v12)
            : "r" (v13), "r" (v14)
            : "%eax", "cc"
        );
    }
    
    /* Final computation using all variables */
    int result = v0 + v1 - v2 * v3 / (v4 + 1) + 
                 v5 | v6 & v7 ^ v8 + v9 - v10 * 
                 v11 / (v12 + 1) + v13 | v14 & v15 ^ 
                 v16 + v17 - v18 * v19 / (v20 + 1) + 
                 v21 | v22 & v23 ^ v24 + v25 - v26 * 
                 v27 / (v28 + 1) + v29;
    
    return result;
}

int main() {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 10; i++) {
        total += test_remat(i * 12345);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

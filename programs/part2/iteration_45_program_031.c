/* Test program for GCC early rematerialization pass
 * Targets uncovered lines in early-remat.cc:930-937
 */

#include <stdio.h>
#include <stdlib.h>

/* Non-inlineable helper to prevent optimization */
static int __attribute__((noinline)) 
compute_offset(int x) {
    volatile int dummy = 0;  /* Side effect */
    return x ^ 0x55AA55AA;
}

/* Main test function creating register pressure */
static int __attribute__((noinline))
test_remat_pressure(int seed) {
    /* Declare many variables to create pseudo registers */
    register int r0 asm ("r12") = seed + 1;
    register int r1 asm ("r13") = seed + 2;
    int v0 = seed * 3;
    int v1 = seed / 2;
    int v2 = seed ^ 0x1234;
    int v3 = seed | 0xABCD;
    int v4 = seed & 0xF0F0;
    int v5 = seed << 3;
    int v6 = seed >> 2;
    int v7 = ~seed;
    int v8 = seed + 100;
    int v9 = seed - 50;
    int v10 = seed * seed;
    int v11 = seed % 17;
    int v12 = seed + compute_offset(seed);
    int v13 = v12 * 2;
    int v14 = v13 / 3;
    int v15 = v14 ^ v13;
    int v16 = v15 | v14;
    int v17 = v16 & v15;
    int v18 = v17 << 1;
    int v19 = v18 >> 2;
    
    /* Complex computation with overlapping live ranges */
    for (int i = 0; i < 100; i++) {
        /* Chain of dependent operations */
        r0 = v0 + v1 + r0;
        v1 = v2 * v3 - r0;
        r1 = v4 ^ v5 | r1;
        v2 = v6 + v7 * r1;
        v3 = v8 - v9 / (r0 + 1);
        v4 = v10 & v11 ^ v2;
        v5 = v12 | v13 & v3;
        v6 = v14 + v15 - v4;
        v7 = v16 * v17 / (v5 + 1);
        v8 = v18 ^ v19 | v6;
        v9 = r0 + r1 * v7;
        v10 = v0 - v1 + v8;
        v11 = v2 * v3 ^ v9;
        v12 = v4 & v5 | v10;
        v13 = v6 + v7 - v11;
        v14 = v8 * v9 / (v12 + 1);
        v15 = v10 ^ v11 & v13;
        v16 = v12 | v13 ^ v14;
        v17 = v14 + v15 * v15;
        v18 = v16 - v17 / (v15 + 1);
        v19 = v18 ^ v17 | v16;
        
        /* Force side effect to keep computations alive */
        if (i % 10 == 0) {
            volatile int barrier = compute_offset(i);
            v0 += barrier;
        }
        
        /* Conditional assignment creating cross-basic-block live ranges */
        if (r0 > r1) {
            v0 = v1 + v2;
            v1 = v3 * v4;
        } else {
            v0 = v5 - v6;
            v1 = v7 / (v8 + 1);
        }
        
        /* Use asm to clobber registers and increase pressure */
        asm volatile ("# Dummy asm" : : "r" (r0), "r" (r1) : "memory");
    }
    
    /* Final computation using all variables */
    int result = r0 + r1 + v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
    
    /* Mix in another non-inlineable call */
    result ^= compute_offset(result);
    
    return result;
}

int main() {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 10; i++) {
        int result = test_remat_pressure(i * 100);
        total += result;
        printf("Iteration %d: result = %d\n", i, result);
    }
    
    printf("Total: %d\n", total);
    return 0;
}

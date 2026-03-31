/* Compile with: gcc -O2 -fno-schedule-insns -fno-schedule-insns2 -fno-peephole2 -o test test.c */
/* For debugging: gcc -O1 -dP -fdump-rtl-early_remat -o test test.c */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable helper to create side effects */
static int __attribute__((noinline)) side_effect(int x) {
    static volatile int counter = 0;
    counter += x;
    return counter & 1;
}

/* Main test function designed to create high register pressure */
static int __attribute__((noinline)) test_remat(int iterations) {
    /* Declare many variables to create register pressure */
    register int v0 asm ("r12") = 1;
    int v1 = 2, v2 = 3, v3 = 4, v4 = 5, v5 = 6, v6 = 7, v7 = 8;
    int v8 = 9, v9 = 10, v10 = 11, v11 = 12, v12 = 13, v13 = 14;
    int v14 = 15, v15 = 16, v16 = 17, v17 = 18, v18 = 19, v19 = 20;
    int v20 = 21, v21 = 22, v22 = 23, v23 = 24, v24 = 25, v25 = 26;
    int v26 = 27, v27 = 28, v28 = 29, v29 = 30;
    
    volatile int memory_barrier = 0;
    
    /* Complex computation with overlapping live ranges */
    for (int i = 0; i < iterations; i++) {
        /* Create def-use chains across operations */
        v1 = v0 + v1 + side_effect(i);
        v2 = v1 * v2 - v0;
        v3 = v2 + v3 ^ v1;
        v4 = v3 - v4 | v2;
        v5 = v4 * v5 & v3;
        v6 = v5 + v6 ^ v4;
        v7 = v6 - v7 | v5;
        v8 = v7 * v8 & v6;
        v9 = v8 + v9 ^ v7;
        v10 = v9 - v10 | v8;
        
        /* More computations creating register pressure */
        v11 = v10 * v11 & v9 + memory_barrier;
        v12 = v11 + v12 ^ v10;
        v13 = v12 - v13 | v11;
        v14 = v13 * v14 & v12;
        v15 = v14 + v15 ^ v13;
        v16 = v15 - v16 | v14;
        v17 = v16 * v17 & v15;
        v18 = v17 + v18 ^ v16;
        v19 = v18 - v19 | v17;
        
        /* Additional layer to increase complexity */
        v20 = v19 * v20 & v18;
        v21 = v20 + v21 ^ v19;
        v22 = v21 - v22 | v20;
        v23 = v22 * v23 & v21;
        v24 = v23 + v24 ^ v22;
        v25 = v24 - v25 | v23;
        v26 = v25 * v26 & v24;
        v27 = v26 + v27 ^ v25;
        v28 = v27 - v28 | v26;
        v29 = v28 * v29 & v27;
        
        /* Use asm to clobber registers and force spills */
        asm volatile ("# Force register clobber" 
                     : "+r" (v0), "+r" (v1), "+r" (v2)
                     : 
                     : "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10");
        
        /* Conditional assignment to create different paths */
        if (i & 1) {
            v0 = v29 + v0;
            v1 = v28 + v1;
            v2 = v27 + v2;
        } else {
            v0 = v29 - v0;
            v1 = v28 - v1;
            v2 = v27 - v2;
        }
        
        /* Memory barrier to prevent reordering */
        memory_barrier = i;
    }
    
    /* Final computation using all variables */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                 v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
    
    return result & 0xFF; /* Return small value to avoid overflow issues */
}

/* Alternative test with different pattern */
static int __attribute__((noinline)) test_remat2(int seed) {
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    int i = seed + 8, j = seed + 9, k = seed + 10, l = seed + 11;
    int m = seed + 12, n = seed + 13, o = seed + 14, p = seed + 15;
    
    /* Create identical computation in multiple places */
    #define DUPLICATE_COMPUTE(x, y) \
        do { \
            int t1 = x * y + side_effect(x); \
            int t2 = t1 ^ (x << 3); \
            int t3 = t2 | (y >> 2); \
            x = t3 - y + (t1 & 0xF); \
        } while(0)
    
    /* Multiple duplicate computations encourage rematerialization */
    DUPLICATE_COMPUTE(a, b);
    DUPLICATE_COMPUTE(c, d);
    DUPLICATE_COMPUTE(e, f);
    DUPLICATE_COMPUTE(g, h);
    DUPLICATE_COMPUTE(i, j);
    DUPLICATE_COMPUTE(k, l);
    DUPLICATE_COMPUTE(m, n);
    DUPLICATE_COMPUTE(o, p);
    
    /* Use results in overlapping expressions */
    int r1 = a + c + e + g;
    int r2 = b + d + f + h;
    int r3 = i + k + m + o;
    int r4 = j + l + n + p;
    
    /* More duplicate patterns */
    DUPLICATE_COMPUTE(r1, r2);
    DUPLICATE_COMPUTE(r3, r4);
    
    return r1 + r2 + r3 + r4;
}

int main() {
    int total = 0;
    
    /* Call with different parameters to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_remat(i % 10 + 5);
        total += test_remat2(i);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

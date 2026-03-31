/* Test program for GCC early rematerialization pass
 * Targets uncovered lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable helper to create side effects */
static int __attribute__((noinline)) side_effect(int x) {
    static volatile int counter = 0;
    counter += x;
    return counter & 1;
}

/* Main test function with high register pressure */
static int __attribute__((noinline)) test_remat(int iterations) {
    /* Declare many variables to create register pressure */
    register int v0 asm ("r12") = 1;
    int v1 = 2, v2 = 3, v3 = 4, v4 = 5, v5 = 6;
    int v6 = 7, v7 = 8, v8 = 9, v9 = 10, v10 = 11;
    int v11 = 12, v12 = 13, v13 = 14, v14 = 15, v15 = 16;
    int v16 = 17, v17 = 18, v18 = 19, v19 = 20, v20 = 21;
    int v21 = 22, v22 = 23, v23 = 24, v24 = 25, v25 = 26;
    
    volatile int memory_barrier = 0;
    
    /* Complex computation with overlapping live ranges */
    for (int i = 0; i < iterations; i++) {
        /* Create long dependency chains */
        v0 = v1 + v2 + side_effect(i);
        v1 = v0 * v3 - v4;
        v2 = v1 / (v5 + 1) + v6;
        v3 = v2 | v7 ^ v8;
        v4 = v3 & v9 | v10;
        v5 = v4 + v11 - v12;
        v6 = v5 * v13 >> 2;
        v7 = v6 & v14 | v15;
        v8 = v7 ^ v16 + v17;
        v9 = v8 * v18 - v19;
        v10 = v9 / (v20 + 1) | v21;
        
        /* More computations creating pseudo registers */
        v11 = v10 + v22 * v23;
        v12 = v11 - v24 ^ v25;
        v13 = v12 & v0 | v1;
        v14 = v13 * v2 - v3;
        v15 = v14 / (v4 + 1) + v5;
        v16 = v15 | v6 ^ v7;
        v17 = v16 & v8 | v9;
        v18 = v17 + v10 - v11;
        v19 = v18 * v12 >> 1;
        v20 = v19 & v13 | v14;
        v21 = v20 ^ v15 + v16;
        v22 = v21 * v17 - v18;
        v23 = v22 / (v19 + 1) | v20;
        v24 = v23 + v21 * v22;
        v25 = v24 - v23 ^ v24;
        
        /* Memory access to prevent optimization */
        memory_barrier = i;
        
        /* Conditional to create different basic blocks */
        if (i & 1) {
            /* Different computation path */
            v0 = v25 + v0;
            v1 = v24 - v1;
            v2 = v23 * v2;
        } else {
            /* Alternative path */
            v0 = v0 - v25;
            v1 = v1 + v24;
            v2 = v2 / (v23 + 1);
        }
        
        /* Use asm to clobber registers and increase pressure */
        asm volatile ("# Force register clobber" 
                     : "+r" (v0), "+r" (v1), "+r" (v2)
                     : 
                     : "r0", "r1", "r2", "r3", "r4", "r5");
    }
    
    /* Combine results to ensure all values are used */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                 v20 + v21 + v22 + v23 + v24 + v25;
    
    return result & 0xFF; /* Return something to prevent dead code elimination */
}

/* Second test function with different pattern */
static int __attribute__((noinline)) test_remat2(int seed) {
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    int i = seed + 8, j = seed + 9, k = seed + 10, l = seed + 11;
    int m = seed + 12, n = seed + 13, o = seed + 14, p = seed + 15;
    
    /* Unrolled computation with many temporary values */
    for (int iter = 0; iter < 100; iter++) {
        /* Each statement creates a new pseudo register */
        int t1 = a * b + side_effect(iter);
        int t2 = c * d - t1;
        int t3 = e * f | t2;
        int t4 = g * h ^ t3;
        int t5 = i * j + t4;
        int t6 = k * l - t5;
        int t7 = m * n | t6;
        int t8 = o * p ^ t7;
        
        /* Cross-dependencies */
        a = t8 + a;
        b = t7 - b;
        c = t6 | c;
        d = t5 ^ d;
        e = t4 + e;
        f = t3 - f;
        g = t2 | g;
        h = t1 ^ h;
        i = t8 * i;
        j = t7 / (j + 1);
        k = t6 + k;
        l = t5 - l;
        m = t4 | m;
        n = t3 ^ n;
        o = t2 + o;
        p = t1 - p;
    }
    
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
}

int main() {
    int total = 0;
    
    /* Call with different parameters to prevent constant propagation */
    for (int i = 0; i < 10; i++) {
        total += test_remat(50 + i);
        total += test_remat2(i * 10);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

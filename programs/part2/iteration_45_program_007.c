/* Test program for GCC early rematerialization pass
 * Targets uncovered lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Non-inlineable helper to create side effects */
static int __attribute__((noinline)) side_effect_func(int x) {
    static volatile int counter = 0;
    counter++;
    return x ^ counter;
}

/* Main test function creating high register pressure */
static int __attribute__((noinline)) test_remat_pressure(int seed) {
    /* Declare many variables to create pseudo registers */
    register int r0 asm ("r12") = seed;
    int v1 = seed + 1, v2 = seed + 2, v3 = seed + 3, v4 = seed + 4;
    int v5 = seed + 5, v6 = seed + 6, v7 = seed + 7, v8 = seed + 8;
    int v9 = seed + 9, v10 = seed + 10, v11 = seed + 11, v12 = seed + 12;
    int v13 = seed + 13, v14 = seed + 14, v15 = seed + 15, v16 = seed + 16;
    int v17 = seed + 17, v18 = seed + 18, v19 = seed + 19, v20 = seed + 20;
    
    /* Volatile variable to prevent optimization */
    volatile int barrier = 0;
    
    /* Complex computation with overlapping live ranges */
    for (int i = 0; i < 100; i++) {
        /* Create def-use chains across basic blocks */
        if (i & 1) {
            r0 = v1 + v2;
            v3 = v4 * v5;
            v6 = side_effect_func(v7) | v8;
        } else {
            r0 = v9 - v10;
            v3 = v11 / (v12 + 1);
            v6 = v13 & v14;
        }
        
        /* More arithmetic creating register pressure */
        v15 = v16 * r0 + v17;
        v18 = v19 - v20 + v15;
        v1 = v2 ^ v3;
        v4 = v5 | v6;
        v7 = v8 + v9;
        v10 = v11 - v12;
        v13 = v14 * v15;
        v16 = v17 ^ v18;
        v19 = v20 + v1;
        v2 = v3 - v4;
        
        /* Use asm to clobber registers and increase pressure */
        asm volatile ("# Dummy asm" : : "r"(r0), "r"(v1), "r"(v2), "r"(v3), 
                     "r"(v4), "r"(v5), "r"(v6) : "memory");
        
        /* Barrier to prevent reordering */
        barrier = i;
    }
    
    /* Final computation using all variables */
    int result = r0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    
    /* Another asm to force pseudo register usage */
    asm volatile ("# Result: %0" : : "r"(result));
    
    return result;
}

/* Second test function with different pattern */
static int __attribute__((noinline)) test_remat_alternative(int seed) {
    int a = seed, b = seed * 2, c = seed * 3, d = seed * 4;
    int e = seed * 5, f = seed * 6, g = seed * 7, h = seed * 8;
    int i = seed * 9, j = seed * 10, k = seed * 11, l = seed * 12;
    int m = seed * 13, n = seed * 14, o = seed * 15, p = seed * 16;
    
    volatile int sync = 0;
    
    /* Unrolled loop with dependent operations */
    for (int iter = 0; iter < 50; iter++) {
        /* Multiple computation paths */
        int t1 = a + b;
        int t2 = c - d;
        int t3 = e * f;
        int t4 = g / (h + 1);
        int t5 = i | j;
        int t6 = k & l;
        int t7 = m ^ n;
        int t8 = o + p;
        
        /* Cross-dependent computations */
        a = t1 + t2;
        b = t3 - t4;
        c = t5 * t6;
        d = t7 ^ t8;
        e = a + b;
        f = c - d;
        g = e * f;
        h = g / (a + 1);
        
        /* Side effect to maintain computation */
        sync = side_effect_func(iter);
        
        /* Register-intensive sequence */
        i = j + k + l + m + n + o + p;
        j = i - a - b - c - d;
        k = side_effect_func(j) * 2;
        l = k | m;
        m = n ^ o;
        n = p + i;
        o = j - k;
        p = l * m;
    }
    
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
}

int main(void) {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 10; i++) {
        total += test_remat_pressure(i);
        total += test_remat_alternative(i * 7);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

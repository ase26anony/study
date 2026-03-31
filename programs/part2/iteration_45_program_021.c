/* Test program for GCC early rematerialization pass
 * Targets uncovered lines in early-remat.cc:930-937
 */

#include <stdio.h>
#include <stdlib.h>

/* Non-inlineable helper to create side effects */
static int __attribute__((noinline)) side_effect_func(int x) {
    static volatile int counter = 0;
    counter++;
    return x + (counter & 1);
}

/* Main test function with high register pressure */
static int __attribute__((noinline)) test_remat_pressure(int seed) {
    /* Declare many variables to create pseudo registers */
    register int r0 asm ("r12") = seed;
    int v1 = seed * 2;
    int v2 = seed / 3;
    int v3 = seed + 7;
    int v4 = seed - 5;
    int v5 = seed ^ 0x55;
    int v6 = seed | 0xAA;
    int v7 = seed & 0xF0;
    int v8 = seed << 2;
    int v9 = seed >> 1;
    int v10 = seed % 13;
    int v11 = seed * seed;
    int v12 = seed + seed;
    int v13 = seed - seed;
    int v14 = ~seed;
    int v15 = seed * 3;
    int v16 = seed * 4;
    int v17 = seed * 5;
    int v18 = seed * 6;
    int v19 = seed * 7;
    int v20 = seed * 8;
    
    /* Volatile variable to prevent optimization */
    volatile int barrier = 0;
    
    /* Complex computation with overlapping live ranges */
    for (int i = 0; i < 100; i++) {
        /* Create def-use chains across operations */
        r0 = r0 + v1 + barrier;
        v1 = v1 * v2 + side_effect_func(i);
        v2 = v2 - v3 + r0;
        v3 = v3 ^ v4 + v1;
        v4 = v4 | v5 + v2;
        v5 = v5 & v6 + v3;
        v6 = v6 << (i & 3) + v4;
        v7 = v7 >> (i & 1) + v5;
        v8 = v8 + v9 * v6;
        v9 = v9 - v10 / (v7 ? v7 : 1);
        v10 = v10 % (v8 + 1) + v8;
        v11 = v11 * v12 + v9;
        v12 = v12 - v13 + v10;
        v13 = v13 ^ v14 + v11;
        v14 = v14 | v15 + v12;
        v15 = v15 & v16 + v13;
        v16 = v16 << 1 + v14;
        v17 = v17 >> 1 + v15;
        v18 = v18 + v19 * v16;
        v19 = v19 - v20 / (v17 ? v17 : 1);
        v20 = v20 % (v18 + 1) + v18;
        
        /* Inline asm with clobber to increase register pressure */
        asm volatile ("# Force register clobber" : : : 
                     "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                     "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
        
        /* Conditional assignment creating cross-basic-block live ranges */
        if (i & 1) {
            r0 = v1 + v2;
            v3 = v4 + v5;
        } else {
            r0 = v6 + v7;
            v3 = v8 + v9;
        }
        
        /* Use results in another conditional */
        if (i & 2) {
            v10 = r0 * v3;
        } else {
            v10 = r0 / (v3 ? v3 : 1);
        }
        
        /* Mix with volatile to prevent dead code elimination */
        barrier = i;
    }
    
    /* Final computation using all variables */
    int result = r0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    
    return result ^ barrier;
}

/* Secondary function with similar pattern to create more opportunities */
static int __attribute__((noinline)) test_remat_pressure2(int seed) {
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    int i = seed + 8, j = seed + 9, k = seed + 10, l = seed + 11;
    int m = seed + 12, n = seed + 13, o = seed + 14, p = seed + 15;
    
    volatile int sync = 0;
    
    for (int iter = 0; iter < 50; iter++) {
        /* Duplicate computations that could be rematerialized */
        a = b + c + side_effect_func(iter);
        d = e * f + a;
        g = h - i + d;
        j = k ^ l + g;
        m = n | o + j;
        p = a & b + m;
        
        /* Same value computed multiple ways */
        int tmp1 = a + b + c;
        int tmp2 = d + e + f;
        int tmp3 = tmp1 * tmp2;
        int tmp4 = tmp1 / (tmp2 ? tmp2 : 1);
        
        /* Use in conditional with overlapping live ranges */
        if (iter % 3 == 0) {
            a = tmp3 + tmp4;
            b = tmp3 - tmp4;
        } else if (iter % 3 == 1) {
            c = tmp3 * tmp4;
            d = tmp3 ^ tmp4;
        } else {
            e = tmp3 | tmp4;
            f = tmp3 & tmp4;
        }
        
        sync = iter;
        
        /* More asm to clobber registers */
        asm volatile ("# More clobbering" : : : 
                     "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23");
    }
    
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p + sync;
}

int main(void) {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 10; i++) {
        total += test_remat_pressure(i);
        total += test_remat_pressure2(i * 7 + 3);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

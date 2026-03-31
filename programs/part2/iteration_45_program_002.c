/* Compile with: gcc -O2 -fno-schedule-insns -fno-schedule-insns2 -fno-peephole2 -o test test.c */
/* For debugging: gcc -O1 -dP -fdump-rtl-early_remat -o test test.c */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable helper to create side effects */
static int __attribute__((noinline)) side_effect(int x) {
    volatile int dummy = 0;
    dummy = x;
    return dummy + 1;
}

/* Function to create high register pressure and rematerialization candidates */
static int __attribute__((noinline)) create_remat_pressure(int seed) {
    /* Declare many variables to create pseudo registers */
    register int v0 asm ("r12") = seed + 1;
    int v1 = seed + 2;
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
    
    volatile int barrier = 0;
    
    /* Complex computation with overlapping live ranges */
    for (int i = 0; i < 100; i++) {
        /* Create def-use chains across operations */
        v0 = v1 + v2 + side_effect(i);
        v1 = v0 * v3 - v4;
        v2 = v1 / (v5 + 1) + v6;
        v3 = v2 ^ v7 | v8;
        v4 = v3 << (v9 & 3);
        v5 = v4 >> (v10 % 4);
        v6 = v5 + v11 * v12;
        v7 = v6 - v13 + v14;
        v8 = v7 * v15 / (v16 + 1);
        v9 = v8 | v17 & v18;
        v10 = v9 ^ v19;
        
        /* Force register pressure with many intermediate values */
        int t0 = v0 + v1;
        int t1 = v2 + v3;
        int t2 = v4 + v5;
        int t3 = v6 + v7;
        int t4 = v8 + v9;
        int t5 = v10 + v11;
        int t6 = v12 + v13;
        int t7 = v14 + v15;
        int t8 = v16 + v17;
        int t9 = v18 + v19;
        
        /* Use results in conditional to maintain liveness */
        if (i % 10 == 0) {
            barrier = t0 + t1 + t2;
            v11 = barrier + t3;
            v12 = barrier - t4;
        } else {
            barrier = t5 + t6 + t7;
            v13 = barrier + t8;
            v14 = barrier - t9;
        }
        
        /* More arithmetic to create rematerialization opportunities */
        v15 = v0 * 2 + v1 * 3;
        v16 = v2 * 4 - v3 * 5;
        v17 = v4 * 6 + v5 * 7;
        v18 = v6 * 8 - v7 * 9;
        v19 = v8 * 10 + v9 * 11;
        
        /* Inline asm to clobber registers and increase pressure */
        asm volatile ("# Dummy asm" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3));
    }
    
    /* Final computation using all variables */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
    
    return result + barrier;
}

/* Alternative version with different computation pattern */
static int __attribute__((noinline)) create_remat_pressure2(int seed) {
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    int i = seed + 8, j = seed + 9, k = seed + 10, l = seed + 11;
    int m = seed + 12, n = seed + 13, o = seed + 14, p = seed + 15;
    int q = seed + 16, r = seed + 17, s = seed + 18, t = seed + 19;
    
    volatile int mem_barrier = 0;
    
    /* Unrolled loop with dependent computations */
    for (int iter = 0; iter < 50; iter++) {
        /* First chain of dependent operations */
        a = b + c + side_effect(iter);
        b = c * d - a;
        c = d / (e + 1) + b;
        d = e ^ f | c;
        e = f << (g & 3) + d;
        f = g >> (h % 4) * e;
        g = h + i * f;
        h = i - j + g;
        
        /* Second independent chain */
        i = j * k / (l + 1);
        j = k | m & i;
        k = l ^ n ^ j;
        l = m + o - k;
        m = n * p / (q + 1);
        n = o | r & m;
        o = p ^ s ^ n;
        p = q + t - o;
        
        /* Cross-chain dependencies */
        q = a + i + mem_barrier;
        r = b + j - mem_barrier;
        s = c + k * mem_barrier;
        t = d + l / (mem_barrier + 1);
        
        mem_barrier = iter;
    }
    
    return a + b + c + d + e + f + g + h + i + j + 
           k + l + m + n + o + p + q + r + s + t;
}

int main() {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 10; i++) {
        total += create_remat_pressure(i);
        total += create_remat_pressure2(i * 7);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

/* Compile with: gcc -O2 -fno-schedule-insns -fno-schedule-insns2 -fno-peephole2 -o test test.c */
/* For debugging: gcc -O1 -dP -fdump-rtl-early_remat -o test test.c */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable helper to create side effects */
static int __attribute__((noinline)) side_effect(int x) {
    static volatile int counter = 0;
    counter++;
    return x + (counter & 1);
}

/* Test function designed to create high register pressure */
static int __attribute__((noinline)) trigger_remat(int seed) {
    /* Declare many variables to create pseudo registers */
    register int v0 asm ("r12") = seed + 1;
    int v1 = seed * 2;
    int v2 = seed / 3;
    int v3 = seed ^ 0x55AA;
    int v4 = seed << 2;
    int v5 = seed >> 1;
    int v6 = seed + 0x1234;
    int v7 = seed - 0x5678;
    int v8 = seed | 0xF0F0;
    int v9 = seed & 0x0F0F;
    int v10 = seed * 3;
    int v11 = seed + v0;
    int v12 = v1 * v2;
    int v13 = v3 ^ v4;
    int v14 = v5 + v6;
    int v15 = v7 - v8;
    int v16 = v9 | v10;
    int v17 = v11 & v12;
    int v18 = v13 ^ v14;
    int v19 = v15 + v16;
    int v20 = v17 - v18;
    int v21 = v19 | v20;
    int v22 = v21 * 2;
    int v23 = v22 / 3;
    int v24 = v23 ^ 0xAA55;
    int v25 = v24 << 1;
    int v26 = v25 >> 2;
    
    /* Volatile variable to prevent optimization */
    volatile int barrier = 0;
    
    /* Complex loop with dependent computations */
    for (int i = 0; i < 100; i++) {
        /* Use asm to clobber registers and increase pressure */
        asm volatile ("# Dummy asm" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3));
        
        /* Chain of dependent operations */
        v0 = v1 + v2 + side_effect(i);
        v1 = v2 * v3 - side_effect(v0);
        v2 = v3 ^ v4 + side_effect(v1);
        v3 = v4 | v5 - side_effect(v2);
        v4 = v5 & v6 + side_effect(v3);
        v5 = v6 + v7 - side_effect(v4);
        v6 = v7 * v8 + side_effect(v5);
        v7 = v8 ^ v9 - side_effect(v6);
        v8 = v9 | v10 + side_effect(v7);
        v9 = v10 & v11 - side_effect(v8);
        
        /* More computations to extend live ranges */
        v10 = v11 + v12 + side_effect(v9);
        v11 = v12 * v13 - side_effect(v10);
        v12 = v13 ^ v14 + side_effect(v11);
        v13 = v14 | v15 - side_effect(v12);
        v14 = v15 & v16 + side_effect(v13);
        v15 = v16 + v17 - side_effect(v14);
        v16 = v17 * v18 + side_effect(v15);
        v17 = v18 ^ v19 - side_effect(v16);
        v18 = v19 | v20 + side_effect(v17);
        v19 = v20 & v21 - side_effect(v18);
        
        /* Conditional to create different basic blocks */
        if (i & 1) {
            v20 = v21 + v22 + side_effect(v19);
            v21 = v22 * v23 - side_effect(v20);
        } else {
            v20 = v21 - v22 + side_effect(v19);
            v21 = v22 / (v23 ? v23 : 1) - side_effect(v20);
        }
        
        /* Use barrier to prevent reordering */
        barrier = i;
        v22 = v23 + barrier;
        v23 = v24 * (barrier + 1);
        
        /* More register pressure */
        v24 = v25 ^ v26;
        v25 = v26 + v0;
        v26 = v0 * v1;
    }
    
    /* Final computation using all variables */
    int result = v0 + v1 - v2 + v3 - v4 + v5 - v6 + v7 - v8 + v9
                 + v10 - v11 + v12 - v13 + v14 - v15 + v16 - v17 + v18 - v19
                 + v20 - v21 + v22 - v23 + v24 - v25 + v26;
    
    /* Another asm to ensure pseudo registers are exposed */
    asm volatile ("# Final asm" 
                  : "+r"(v0), "+r"(v1), "+r"(v2), "+r"(v3), "+r"(v4), "+r"(v5)
                  : "r"(v6), "r"(v7), "r"(v8), "r"(v9), "r"(v10), "r"(v11));
    
    return result + side_effect(barrier);
}

/* Alternative test function with different pattern */
static int __attribute__((noinline)) trigger_remat2(int seed) {
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    int i = seed + 8, j = seed + 9, k = seed + 10, l = seed + 11;
    int m = seed + 12, n = seed + 13, o = seed + 14, p = seed + 15;
    
    volatile int sync = 0;
    
    /* Unrolled loop with duplicate computations */
    for (int iter = 0; iter < 50; iter++) {
        /* Same computation in multiple places - candidate for remat */
        int t1 = a * b + c * d;
        int t2 = e * f + g * h;
        int t3 = i * j + k * l;
        int t4 = m * n + o * p;
        
        /* Use results in different ways */
        a = t1 + side_effect(iter);
        b = t2 - side_effect(a);
        c = t3 ^ side_effect(b);
        d = t4 | side_effect(c);
        
        /* Duplicate the t1 computation */
        e = a * b + c * d + side_effect(d);
        
        /* More duplicates */
        f = e * f + g * h;
        g = i * j + k * l;
        h = m * n + o * p;
        
        sync = iter;
        
        /* Cross-dependent computations */
        i = (a + b) * (c + d) + side_effect(sync);
        j = (e + f) * (g + h) - side_effect(i);
        k = (i + j) * (a + e) ^ side_effect(j);
        l = (b + f) * (c + g) | side_effect(k);
        
        m = t1 + t2 + t3 + t4;
        n = m * 2 - side_effect(l);
        o = n / 3 + side_effect(m);
        p = o ^ 0x1234 - side_effect(n);
    }
    
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p + sync;
}

int main() {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 10; i++) {
        total += trigger_remat(i * 100);
        total += trigger_remat2(i * 200);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

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

/* Function to create high register pressure */
static int __attribute__((noinline)) create_register_pressure(int seed) {
    /* Declare many variables to create pseudo registers */
    register int v0 asm ("r12") = seed;
    int v1 = seed + 1;
    int v2 = seed + 2;
    int v3 = seed + 3;
    int v4 = seed + 4;
    int v5 = seed + 5;
    int v6 = seed + 6;
    int v7 = seed + 7;
    int v8 = seed + 8;
    int v9 = seed + 9;
    int v10 = seed + 10;
    int v11 = seed + 11;
    int v12 = seed + 12;
    int v13 = seed + 13;
    int v14 = seed + 14;
    int v15 = seed + 15;
    int v16 = seed + 16;
    int v17 = seed + 17;
    int v18 = seed + 18;
    int v19 = seed + 19;
    
    volatile int barrier = 0;
    
    /* Complex computation with dependent operations */
    for (int i = 0; i < 100; i++) {
        /* Create long dependency chains */
        v0 = v1 + v2 + side_effect(i);
        v1 = v0 * v3 - v4;
        v2 = v1 / (v5 + 1) + v6;
        v3 = v2 ^ v7 | v8;
        v4 = v3 + v9 - v10;
        v5 = v4 * v11 % (v12 + 1);
        v6 = v5 & v13 | v14;
        v7 = v6 + v15 - v16;
        v8 = v7 * v17 / (v18 + 1);
        v9 = v8 ^ v19 + v0;
        
        /* Use asm to create register pressure and clobbers */
        asm volatile ("" : "+r" (v0), "+r" (v1), "+r" (v2) 
                      : : "r0", "r1", "r2", "r3", "r4", "r5");
        
        /* Conditional assignments to create different basic blocks */
        if (i & 1) {
            v10 = v9 + v1;
            v11 = v10 * v2;
            v12 = v11 - v3;
        } else {
            v10 = v9 - v1;
            v11 = v10 / (v2 + 1);
            v12 = v11 | v3;
        }
        
        /* More computations with overlapping live ranges */
        v13 = v12 + v4;
        v14 = v13 * v5;
        v15 = v14 - v6;
        v16 = v15 ^ v7;
        v17 = v16 & v8;
        v18 = v17 + v9;
        v19 = v18 * v10;
        
        /* Memory barrier to prevent reordering */
        barrier = i;
    }
    
    /* Final computation using all variables */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
    
    return result & 0xFF; /* Return something non-trivial */
}

/* Alternative version with different computation pattern */
static int __attribute__((noinline)) create_register_pressure2(int seed) {
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    int i = seed + 8, j = seed + 9, k = seed + 10, l = seed + 11;
    int m = seed + 12, n = seed + 13, o = seed + 14, p = seed + 15;
    int q = seed + 16, r = seed + 17, s = seed + 18, t = seed + 19;
    
    volatile int mem_barrier = 0;
    
    /* Unrolled loop with dependent computations */
    for (int iter = 0; iter < 50; iter++) {
        /* First computation block */
        a = b + c + side_effect(iter);
        b = a * d - e;
        c = b / (f + 1) + g;
        d = c ^ h | i;
        
        /* Force spill/reload candidates */
        mem_barrier = iter;
        
        /* Second computation block - same values recomputed differently */
        e = d + j - k;
        f = e * l % (m + 1);
        g = f & n | o;
        h = g + p - q;
        
        /* Third block - creates opportunities for rematerialization */
        i = h * r / (s + 1);
        j = i ^ t + a;
        k = j + b;
        l = k * c;
        
        /* Use all variables to keep them live */
        m = l - d;
        n = m ^ e;
        o = n & f;
        p = o + g;
        q = p * h;
        r = q - i;
        s = r ^ j;
        t = s & k;
    }
    
    return (a + b + c + d + e + f + g + h + i + j + 
            k + l + m + n + o + p + q + r + s + t) & 0xFF;
}

int main() {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 1000; i++) {
        total += create_register_pressure(i);
        total += create_register_pressure2(i * 3);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

/* Test program for GCC early rematerialization pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlineable helper to create side effects */
static int __attribute__((noinline)) side_effect(int x) {
    static volatile int counter = 0;
    counter++;
    return x + (counter & 1);
}

/* Main test function with high register pressure */
static int __attribute__((noinline)) test_rematerialization(int seed) {
    /* Declare many variables to create register pressure */
    register int r0 asm ("r12") = seed;
    register int r1 asm ("r13") = seed * 2;
    int v0 = seed + 1;
    int v1 = seed * 3;
    int v2 = seed + 4;
    int v3 = seed * 5;
    int v4 = seed + 6;
    int v5 = seed * 7;
    int v6 = seed + 8;
    int v7 = seed * 9;
    int v8 = seed + 10;
    int v9 = seed * 11;
    int v10 = seed + 12;
    int v11 = seed * 13;
    int v12 = seed + 14;
    int v13 = seed * 15;
    int v14 = seed + 16;
    int v15 = seed * 17;
    int v16 = seed + 18;
    int v17 = seed * 19;
    int v18 = seed + 20;
    int v19 = seed * 21;
    
    /* Volatile variable to prevent optimization */
    volatile int barrier = 0;
    
    /* Complex computation with overlapping live ranges */
    for (int i = 0; i < 100; i++) {
        /* Create def-use chains across operations */
        r0 = v0 + v1 + side_effect(i);
        v2 = r0 * v3 - v4;
        r1 = v5 + v6 + side_effect(r0);
        v7 = r1 * v8 - v9;
        v10 = v2 + v7 + side_effect(r1);
        
        v11 = v10 * v12 - v13;
        v14 = v11 + v15 + side_effect(v10);
        v16 = v14 * v17 - v18;
        v19 = v16 + v0 + side_effect(v14);
        
        /* Cross-dependent computations */
        v0 = v19 + v1 + barrier;
        v1 = v0 * v2 - v3;
        v3 = v1 + v4 + side_effect(v0);
        v4 = v3 * v5 - v6;
        v5 = v4 + v7 + side_effect(v3);
        v6 = v5 * v8 - v9;
        
        /* More overlapping live ranges */
        v8 = v6 + v10 + barrier;
        v9 = v8 * v11 - v12;
        v12 = v9 + v13 + side_effect(v8);
        v13 = v12 * v14 - v15;
        v15 = v13 + v16 + side_effect(v12);
        v17 = v15 * v18 - v19;
        
        /* Force register pressure with asm clobbers */
        asm volatile ("# Force clobber" : : : "memory", "r0", "r1", "r2", "r3", "r4", "r5");
        
        /* Conditional assignments to create basic block boundaries */
        if (i & 1) {
            v18 = v17 + v0 + side_effect(v15);
            v19 = v18 * v1 - v2;
        } else {
            v18 = v17 + v3 + side_effect(v15);
            v19 = v18 * v4 - v5;
        }
        
        barrier = i; /* Volatile write */
    }
    
    /* Final computation using all variables */
    int result = r0 + r1 + v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
    
    return result + barrier;
}

/* Secondary test with different pattern */
static int __attribute__((noinline)) test_remat_variant(int seed) {
    int a = seed, b = seed * 2, c = seed * 3, d = seed * 4;
    int e = seed * 5, f = seed * 6, g = seed * 7, h = seed * 8;
    int i = seed * 9, j = seed * 10, k = seed * 11, l = seed * 12;
    int m = seed * 13, n = seed * 14, o = seed * 15, p = seed * 16;
    
    volatile int sync = 0;
    
    for (int iter = 0; iter < 50; iter++) {
        /* Duplicate computations that are rematerialization candidates */
        int t1 = a + b + side_effect(iter);
        int t2 = c + d + side_effect(t1);
        int t3 = e + f + side_effect(t2);
        int t4 = g + h + side_effect(t3);
        
        /* Use results in multiple places */
        a = t1 * t2 - t3;
        b = t2 * t3 - t4;
        c = t3 * t4 - t1;
        d = t4 * t1 - t2;
        
        /* Same computation pattern repeated */
        int u1 = i + j + side_effect(iter);
        int u2 = k + l + side_effect(u1);
        int u3 = m + n + side_effect(u2);
        int u4 = o + p + side_effect(u3);
        
        i = u1 * u2 - u3;
        j = u2 * u3 - u4;
        k = u3 * u4 - u1;
        l = u4 * u1 - u2;
        
        /* Mix the two sets */
        m = a + i + sync;
        n = b + j + side_effect(a);
        o = c + k + sync;
        p = d + l + side_effect(c);
        
        sync = iter;
        
        /* Inline asm to create register constraints */
        asm volatile ("# Complex constraint" 
                     : "=r" (a), "=r" (b), "=r" (c), "=r" (d)
                     : "0" (a), "1" (b), "2" (c), "3" (d)
                     : "r5", "r6", "r7", "r8", "r9", "r10");
    }
    
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p + sync;
}

int main(void) {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 10; i++) {
        total += test_rematerialization(i);
        total += test_remat_variant(i * 7 + 3);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

/* Test program for GCC early rematerialization pass
 * Targets uncovered lines in early-remat.cc:930-937
 */

#include <stdio.h>
#include <stdlib.h>

/* Non-inlineable helper to create side effects */
static int __attribute__((noinline)) side_effect(int x) {
    static volatile int counter = 0;
    counter++;
    return x + (counter & 1);
}

/* Main test function creating register pressure */
static int __attribute__((noinline)) test_rematerialization(int seed) {
    /* Declare many variables to create register pressure */
    register int v0 asm ("r12") = seed + 1;
    register int v1 asm ("r13") = seed + 2;
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
    
    /* Volatile variable to prevent optimization */
    volatile int barrier = 0;
    
    /* Complex computation with overlapping live ranges */
    for (int i = 0; i < 100; i++) {
        /* Create def-use chains across operations */
        v0 = v1 + v2;
        v1 = v0 * v3;
        v2 = v1 - v4;
        v3 = v2 + v5;
        v4 = v3 * v6;
        v5 = v4 - v7;
        v6 = v5 + v8;
        v7 = v6 * v9;
        v8 = v7 - v10;
        v9 = v8 + v11;
        v10 = v9 * v12;
        v11 = v10 - v13;
        v12 = v11 + v14;
        v13 = v12 * v15;
        v14 = v13 - v16;
        v15 = v14 + v17;
        v16 = v15 * v18;
        v17 = v16 - v19;
        v18 = v17 + v0;  /* Circular dependency */
        v19 = v18 * v1;
        
        /* Mix with side effects to create non-movable instructions */
        barrier = side_effect(i);
        v0 += barrier;
        v1 -= barrier;
        
        /* Conditional to create basic block boundaries */
        if (i & 1) {
            v2 = v3 + v4;
            v5 = v6 * v7;
        } else {
            v2 = v8 - v9;
            v5 = v10 / (v11 + 1);
        }
        
        /* Use asm to clobber registers and increase pressure */
        asm volatile ("# Dummy asm" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3));
        
        /* More computations to extend live ranges */
        v3 = v12 + v13;
        v4 = v14 * v15;
        v6 = v16 - v17;
        v7 = v18 + v19;
        v8 = v0 * v1;
        v9 = v2 - v3;
        v10 = v4 + v5;
        v11 = v6 * v7;
        v12 = v8 - v9;
        v13 = v10 + v11;
        v14 = v12 * v13;
        v15 = v14 - v0;
        v16 = v15 + v1;
        v17 = v16 * v2;
        v18 = v17 - v3;
        v19 = v18 + v4;
    }
    
    /* Final computation using all variables */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
    
    return result & 0xFF;  /* Return small value to avoid overflow */
}

/* Secondary test with different pattern */
static int __attribute__((noinline)) test_remat_variant(int seed) {
    int a = seed * 2;
    int b = seed + 5;
    int c = seed - 3;
    int d = seed * 3;
    int e = seed + 7;
    int f = seed - 2;
    
    volatile int sync = 0;
    
    for (int i = 0; i < 50; i++) {
        /* Duplicate computation patterns - candidates for remat */
        int t1 = a * b + c;
        int t2 = d * e - f;
        int t3 = t1 + t2;
        int t4 = t1 - t2;
        
        /* Same computation again in different context */
        int u1 = a * b + c;  /* Duplicate of t1 computation */
        int u2 = d * e - f;  /* Duplicate of t2 computation */
        
        sync = side_effect(i);
        a = t3 + sync;
        b = t4 - sync;
        c = u1 * u2;
        d = a + b;
        e = c - d;
        f = e * 2;
        
        /* Force pseudo register usage with asm */
        asm volatile ("# Force reg usage %0, %1, %2" 
                     : "+r"(a), "+r"(b), "+r"(c));
    }
    
    return (a + b + c + d + e + f) & 0xFF;
}

int main() {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 10; i++) {
        total += test_rematerialization(i * 17);
        total += test_remat_variant(i * 23 + 1);
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}

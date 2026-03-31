/* Compile with: gcc -O2 -fno-schedule-insns -fno-schedule-insns2 -fno-peephole2 -o test_remat test_remat.c */
/* For debugging: gcc -O1 -dP -fdump-rtl-early_remat -o test_remat_debug test_remat.c */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable helper to create side effects */
static int __attribute__((noinline)) side_effect(int x) {
    static volatile int counter = 0;
    counter++;
    return x + (counter & 1);
}

/* Function to create high register pressure and trigger rematerialization */
static int __attribute__((noinline)) test_remat_pressure(int iterations) {
    /* Declare many variables to create pseudo registers */
    register int v0 asm ("r12") = 1;
    int v1 = 2, v2 = 3, v3 = 4, v4 = 5, v5 = 6, v6 = 7, v7 = 8;
    int v8 = 9, v9 = 10, v10 = 11, v11 = 12, v12 = 13, v13 = 14, v14 = 15;
    int v15 = 16, v16 = 17, v17 = 18, v18 = 19, v19 = 20;
    int v20 = 21, v21 = 22, v22 = 23, v23 = 24, v24 = 25;
    
    /* Volatile variable to prevent optimization */
    volatile int barrier = 0;
    
    /* Complex computation with overlapping live ranges */
    for (int i = 0; i < iterations; i++) {
        /* Create long dependency chains with side effects */
        v0 = side_effect(v0 + v1);
        v1 = v0 * v2 - v3;
        v2 = v1 + v4 * v5;
        v3 = side_effect(v2 - v6);
        v4 = v3 * v7 + v8;
        v5 = v4 - v9 * v10;
        v6 = side_effect(v5 + v11);
        v7 = v6 * v12 - v13;
        v8 = v7 + v14 * v15;
        v9 = side_effect(v8 - v16);
        v10 = v9 * v17 + v18;
        v11 = v10 - v19 * v20;
        v12 = side_effect(v11 + v21);
        v13 = v12 * v22 - v23;
        v14 = v13 + v24 * v0;
        
        /* Use asm to create register pressure and clobber registers */
        asm volatile ("# Force register pressure" : 
                     : "r" (v0), "r" (v1), "r" (v2), "r" (v3), 
                       "r" (v4), "r" (v5), "r" (v6), "r" (v7));
        
        /* Mix stack and register variables */
        v15 = v14 + barrier;
        v16 = v15 * v1 - v2;
        v17 = side_effect(v16 + v3);
        v18 = v17 * v4 + v5;
        v19 = v18 - v6 * v7;
        v20 = side_effect(v19 + v8);
        v21 = v20 * v9 - v10;
        v22 = v21 + v11 * v12;
        v23 = side_effect(v22 - v13);
        v24 = v23 * v14 + v15;
        
        /* Create conditional assignments for cross-basic-block live ranges */
        if (i & 1) {
            v0 = v24 + v1;
            v1 = side_effect(v0 * 2);
        } else {
            v0 = v24 - v1;
            v1 = side_effect(v0 / 2);
        }
        
        /* Use results in next iteration to maintain liveness */
        barrier = i;
    }
    
    /* Combine all results to prevent dead code elimination */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                 v20 + v21 + v22 + v23 + v24;
    
    return result ^ barrier;
}

/* Alternative version with different optimization characteristics */
static int __attribute__((noinline)) test_remat_alt(int seed) {
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    int i = seed + 8, j = seed + 9, k = seed + 10, l = seed + 11;
    int m = seed + 12, n = seed + 13, o = seed + 14, p = seed + 15;
    
    /* Create identical computations in different paths */
    for (int x = 0; x < 100; x++) {
        /* First computation path */
        int t1 = a * b + c;
        int t2 = side_effect(t1 - d);
        int t3 = t2 * e + f;
        int t4 = side_effect(t3 - g);
        
        /* Second computation path with same values */
        int u1 = a * b + c;
        int u2 = side_effect(u1 - d);
        int u3 = u2 * e + f;
        int u4 = side_effect(u3 - g);
        
        /* Use both results differently */
        if (x & 1) {
            h = t4 + u4;
        } else {
            h = t4 - u4;
        }
        
        /* Rotate values to extend live ranges */
        a = b; b = c; c = d; d = e;
        e = f; f = g; g = h; h = i;
        i = j; j = k; k = l; l = m;
        m = n; n = o; o = p; p = a + x;
    }
    
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
}

int main() {
    int total = 0;
    
    /* Call with different parameters to prevent constant propagation */
    for (int i = 0; i < 10; i++) {
        total += test_remat_pressure(50 + i);
        total += test_remat_alt(i * 7);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

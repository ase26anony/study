/* Compile with: gcc -O2 -fno-schedule-insns -fno-schedule-insns2 -fno-peephole2 -o test test.c */
/* For debugging: gcc -O1 -dP -fdump-rtl-early_remat -o test test.c */

#include <stdio.h>
#include <stdlib.h>

/* Non-inlineable helper to create side effects */
static int __attribute__((noinline)) side_effect(int x) {
    static volatile int counter = 0;
    counter++;
    return x + (counter & 1);
}

/* Function to create high register pressure and rematerialization candidates */
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
    
    /* Volatile variable to prevent optimizations */
    volatile int barrier = 0;
    
    /* Complex computation with overlapping live ranges */
    for (int i = 0; i < 100; i++) {
        /* Create def-use chains across operations */
        v0 = v1 + v2 + side_effect(i);
        v1 = v0 * v3 - v4;
        v2 = v1 / (v5 + 1) + v6;
        v3 = v2 | v7 ^ v8;
        v4 = v3 & v9 + v10;
        v5 = v4 - v11 * v12;
        v6 = v5 + v13 - v14;
        v7 = v6 * v15 + v16;
        v8 = v7 & v17 | v18;
        v9 = v8 ^ v19 + v0;
        v10 = v9 - v1 * v2;
        v11 = v10 + v3 - v4;
        v12 = v11 * v5 + v6;
        v13 = v12 & v7 | v8;
        v14 = v13 ^ v9 + v10;
        v15 = v14 - v11 * v12;
        v16 = v15 + v13 - v14;
        v17 = v16 * v15 + v16;
        v18 = v17 & v17 | v18;
        v19 = v18 ^ v19 + v0;
        
        /* Use barrier to create side effects */
        barrier = i;
        
        /* Conditional to create basic block boundaries */
        if (i & 1) {
            v0 = v1 + v2;
            v3 = v4 * v5;
        } else {
            v0 = v6 - v7;
            v3 = v8 / (v9 + 1);
        }
        
        /* More computations to extend live ranges */
        v1 = v0 + v3;
        v2 = v1 * v10;
        v4 = v2 - v11;
        v5 = v4 | v12;
        v6 = v5 & v13;
        v7 = v6 ^ v14;
        v8 = v7 + v15;
        v9 = v8 * v16;
        v10 = v9 - v17;
        v11 = v10 | v18;
        v12 = v11 & v19;
        
        /* Inline asm to clobber registers and increase pressure */
        asm volatile ("# Dummy asm" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3));
    }
    
    /* Final computation using all variables */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
    
    return result + barrier;
}

/* Another layer to prevent optimization */
static int __attribute__((noinline)) compute_with_variants(int base) {
    int sum = 0;
    for (int j = 0; j < 10; j++) {
        /* Create multiple similar computations */
        int r1 = create_register_pressure(base + j * 7);
        int r2 = create_register_pressure(base + j * 13);
        int r3 = create_register_pressure(base + j * 19);
        
        /* Force use of results */
        sum += r1 * r2 - r3;
        
        /* Additional asm to affect register allocation */
        asm volatile ("# Another dummy" : "+r"(sum) : : "memory");
    }
    return sum;
}

int main() {
    int total = 0;
    
    /* Call with different parameters to prevent constant propagation */
    for (int iter = 0; iter < 50; iter++) {
        total += compute_with_variants(iter * 100);
        
        /* Print periodically to ensure computation isn't eliminated */
        if (iter % 10 == 9) {
            printf("Iteration %d: total = %d\n", iter, total);
        }
    }
    
    printf("Final result: %d\n", total);
    return 0;
}

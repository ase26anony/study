/* Compile with: gcc -O2 -fno-schedule-insns -fno-schedule-insns2 -fno-peephole2 -o test test.c */
/* For debugging: gcc -O1 -dP -fdump-rtl-early_remat -o test test.c */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable helper to create side effects */
static int __attribute__((noinline)) side_effect(int x) {
    volatile int sink = x;
    return sink + 1;
}

/* Function to create high register pressure and rematerialization candidates */
static int __attribute__((noinline)) create_remat_pressure(int seed) {
    /* Declare many variables to create pseudo registers */
    register int v0 asm ("r12") = seed;
    int v1 = seed + 1;
    int v2 = seed * 2;
    int v3 = seed / 3;
    int v4 = seed ^ 0x55;
    int v5 = seed | 0xAA;
    int v6 = seed & 0xF0;
    int v7 = seed << 2;
    int v8 = seed >> 1;
    int v9 = ~seed;
    int v10 = seed + 11;
    int v11 = seed * 13;
    int v12 = seed / 17;
    int v13 = seed ^ 0x33;
    int v14 = seed | 0xCC;
    int v15 = seed & 0x0F;
    int v16 = seed << 3;
    int v17 = seed >> 2;
    int v18 = ~seed + 1;
    int v19 = seed + 19;
    int v20 = seed * 23;
    int v21 = seed / 29;
    int v22 = seed ^ 0x99;
    int v23 = seed | 0x66;
    int v24 = seed & 0xF5;
    int v25 = seed << 1;
    int v26 = seed >> 3;
    int v27 = ~seed - 1;
    int v28 = seed + 31;
    int v29 = seed * 37;
    
    /* Volatile variable to prevent optimization */
    volatile int barrier = 0;
    
    /* Complex computation with overlapping live ranges */
    for (int i = 0; i < 100; i++) {
        /* Create def-use chains across operations */
        v0 = v1 + v2 + barrier;
        v1 = v0 * v3 - v4;
        v2 = v1 / (v5 + 1) + v6;
        v3 = v2 ^ v7 | v8;
        v4 = v3 & v9 + v10;
        v5 = v4 << (v11 & 3);
        v6 = v5 >> (v12 % 4);
        v7 = v6 + v13 - v14;
        v8 = v7 * v15 / (v16 + 1);
        v9 = v8 ^ v17 & v18;
        v10 = v9 | v19 + v20;
        v11 = v10 & v21 * v22;
        v12 = v11 << (v23 % 8);
        v13 = v12 >> (v24 & 7);
        v14 = v13 + v25 - v26;
        v15 = v14 * v27 / (v28 + 1);
        v16 = v15 ^ v29 & v0;
        v17 = v16 | v1 + v2;
        v18 = v17 & v3 * v4;
        v19 = v18 << (v5 & 7);
        v20 = v19 >> (v6 % 8);
        v21 = v20 + v7 - v8;
        v22 = v21 * v9 / (v10 + 1);
        v23 = v22 ^ v11 & v12;
        v24 = v23 | v13 + v14;
        v25 = v24 & v15 * v16;
        v26 = v25 << (v17 & 7);
        v27 = v26 >> (v18 % 8);
        v28 = v27 + v19 - v20;
        v29 = v28 * v21 / (v22 + 1);
        
        /* Side effect to create non-movable instructions */
        barrier = side_effect(i);
        
        /* Conditional to create different basic blocks */
        if (i & 1) {
            v0 = v1 + v2;
            v3 = v4 * v5;
        } else {
            v0 = v6 - v7;
            v3 = v8 / (v9 + 1);
        }
        
        /* Use results in another conditional to maintain liveness */
        if (i & 2) {
            v10 = v0 * v3;
        } else {
            v10 = v0 + v3;
        }
        
        /* More arithmetic to increase register pressure */
        v11 = v10 + v1 + v2 + v3 + v4 + v5;
        v12 = v11 * v6 * v7 * v8 * v9;
        v13 = v12 - v14 - v15 - v16 - v17;
        v14 = v13 & v18 & v19 & v20 & v21;
        v15 = v14 | v22 | v23 | v24 | v25;
        v16 = v15 ^ v26 ^ v27 ^ v28 ^ v29;
    }
    
    /* Final computation using all variables */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                 v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
    
    return result;
}

int main() {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 10; i++) {
        total += create_remat_pressure(i * 100);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

/* Compile with: gcc -O2 -fno-schedule-insns -fno-schedule-insns2 -fno-peephole2 -o test test.c */
/* For debugging: gcc -O1 -dP -fdump-rtl-early_remat -o test test.c */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable helper to create side effects */
static int __attribute__((noinline)) side_effect(int x) {
    static volatile int counter = 0;
    counter += x;
    return counter & 1;
}

/* Test function designed to create high register pressure */
static int __attribute__((noinline)) test_remat(int seed) {
    /* Declare many variables to create pseudo registers */
    register int v0 asm ("r12") = seed + 1;
    int v1 = seed * 2;
    int v2 = seed / 3;
    int v3 = seed ^ 0x55;
    int v4 = seed | 0xAA;
    int v5 = seed & 0xF0;
    int v6 = seed << 2;
    int v7 = seed >> 1;
    int v8 = seed + 100;
    int v9 = seed - 50;
    int v10 = seed * 3;
    int v11 = seed / 2;
    int v12 = seed ^ 0xFF;
    int v13 = seed | 0x33;
    int v14 = seed & 0x0F;
    int v15 = seed << 1;
    int v16 = seed >> 2;
    int v17 = seed + 200;
    int v18 = seed - 100;
    int v19 = seed * 5;
    int v20 = seed / 4;
    int v21 = seed ^ 0xAA;
    int v22 = seed | 0xCC;
    int v23 = seed & 0x3F;
    int v24 = seed << 3;
    int v25 = seed >> 3;
    int v26 = seed + 300;
    int v27 = seed - 150;
    int v28 = seed * 7;
    int v29 = seed / 5;
    
    /* Volatile variable to prevent optimization */
    volatile int barrier = 0;
    
    /* Complex computation with overlapping live ranges */
    for (int i = 0; i < 100; i++) {
        /* Create dependent chains that span multiple iterations */
        v0 = v1 + v2 + side_effect(i);
        v1 = v0 * v3 - v4;
        v2 = v1 / (v5 + 1) + v6;
        v3 = v2 ^ v7 | v8;
        v4 = v3 & v9 + v10;
        v5 = v4 << (v11 & 3);
        v6 = v5 >> (v12 & 3);
        v7 = v6 + v13 - v14;
        v8 = v7 * v15 / (v16 + 1);
        v9 = v8 ^ v17 | v18;
        v10 = v9 & v19 + v20;
        
        /* Another independent chain */
        v11 = v21 + v22 + side_effect(i + 1);
        v12 = v11 * v23 - v24;
        v13 = v12 / (v25 + 1) + v26;
        v14 = v13 ^ v27 | v28;
        v15 = v14 & v29 + v0;
        v16 = v15 << (v1 & 3);
        v17 = v16 >> (v2 & 3);
        v18 = v17 + v3 - v4;
        v19 = v18 * v5 / (v6 + 1);
        v20 = v19 ^ v7 | v8;
        
        /* Cross-chain dependencies */
        v21 = v9 + v10 + v20;
        v22 = v21 * v11 - v12;
        v23 = v22 / (v13 + 1) + v14;
        v24 = v23 ^ v15 | v16;
        v25 = v24 & v17 + v18;
        v26 = v25 << (v19 & 3);
        v27 = v26 >> (v20 & 3);
        v28 = v27 + v0 - v1;
        v29 = v28 * v2 / (v3 + 1);
        
        /* Force memory barrier to create side effects */
        barrier = i;
        
        /* Inline asm to clobber registers and increase pressure */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            : 
            : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4),
              "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9)
            : "memory"
        );
    }
    
    /* Final computation using all variables to ensure they're live */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                 v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
    
    return result ^ barrier;
}

/* Main function to call test repeatedly */
int main() {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 10; i++) {
        total += test_remat(i * 12345);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

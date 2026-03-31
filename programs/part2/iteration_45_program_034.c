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

/* Main test function designed to create high register pressure */
static int __attribute__((noinline)) test_remat(int seed) {
    /* Declare many variables to create pseudo registers */
    register int v0 asm ("r12") = seed;
    int v1 = seed + 1;
    int v2 = seed * 2;
    int v3 = seed / 3;
    int v4 = seed ^ 0x55;
    int v5 = seed | 0xAA;
    int v6 = seed & 0xFF;
    int v7 = seed << 2;
    int v8 = seed >> 1;
    int v9 = seed + 100;
    int v10 = seed - 50;
    int v11 = seed * 3;
    int v12 = seed / 2;
    int v13 = seed ^ 0xCC;
    int v14 = seed | 0x33;
    int v15 = seed & 0xF0;
    int v16 = seed << 1;
    int v17 = seed >> 2;
    int v18 = seed + 200;
    int v19 = seed - 100;
    int v20 = seed * 5;
    int v21 = seed / 4;
    int v22 = seed ^ 0xAA;
    int v23 = seed | 0x55;
    int v24 = seed & 0x0F;
    int v25 = seed << 3;
    int v26 = seed >> 3;
    int v27 = seed + 300;
    int v28 = seed - 150;
    int v29 = seed * 7;
    
    /* Create complex data flow with side effects to prevent optimization */
    volatile int barrier = 0;
    
    /* Unrolled loop with dependent operations to extend live ranges */
    for (int i = 0; i < 10; i++) {
        /* Mix register and automatic variables in complex expressions */
        v0 = v1 + v2 + side_effect(i);
        v1 = v0 * v3 - v4;
        v2 = v1 / (v5 + 1) + v6;
        v3 = v2 ^ v7 | v8;
        v4 = v3 & v9 + v10;
        v5 = v4 << (v11 % 8) >> 1;
        v6 = v5 + v12 - v13;
        v7 = v6 * v14 / (v15 + 1);
        v8 = v7 | v16 & v17;
        v9 = v8 ^ v18 + v19;
        v10 = v9 * v20 - v21;
        v11 = v10 + v22 / (v23 + 1);
        v12 = v11 | v24 & v25;
        v13 = v12 << 1 >> (v26 % 4);
        v14 = v13 + v27 - v28;
        v15 = v14 * v29 / (seed + 1);
        
        /* Use asm to clobber registers and increase pressure */
        asm volatile ("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3), 
                      "r"(v4), "r"(v5), "r"(v6), "r"(v7) : 
                      "memory");
        
        /* Conditional assignments to create branching data flow */
        if (i & 1) {
            v16 = v15 + v0;
            v17 = v16 * v1;
        } else {
            v16 = v15 - v0;
            v17 = v16 / (v1 + 1);
        }
        
        v18 = v17 + v2 * v3;
        v19 = v18 - v4 / (v5 + 1);
        v20 = v19 | v6 & v7;
        v21 = v20 ^ v8 + v9;
        v22 = v21 * v10 - v11;
        v23 = v22 + v12 / (v13 + 1);
        v24 = v23 | v14 & v15;
        v25 = v24 << (v16 % 8);
        v26 = v25 >> (v17 % 4);
        v27 = v26 + v18 - v19;
        v28 = v27 * v20 / (v21 + 1);
        v29 = v28 | v22 & v23;
        
        /* Memory barrier to prevent reordering */
        barrier = i;
    }
    
    /* Final computation using all variables to ensure they're live */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                 v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
    
    return result ^ barrier;
}

int main() {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_remat(i);
        total += test_remat(i * 3 + 1);
        total += test_remat(i * 7 - 2);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

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

/* Main test function designed to create high register pressure */
static int __attribute__((noinline)) test_remat(int iterations) {
    /* Declare many integer variables to create pseudo registers */
    register int v0 asm ("r12") = 1;
    int v1 = 2, v2 = 3, v3 = 4, v4 = 5, v5 = 6, v6 = 7, v7 = 8;
    int v8 = 9, v9 = 10, v10 = 11, v11 = 12, v12 = 13, v13 = 14;
    int v14 = 15, v15 = 16, v16 = 17, v17 = 18, v18 = 19, v19 = 20;
    int v20 = 21, v21 = 22, v22 = 23, v23 = 24, v24 = 25, v25 = 26;
    int v26 = 27, v27 = 28, v28 = 29, v29 = 30;
    
    /* Volatile variable to prevent optimization */
    volatile int barrier = 0;
    
    /* Complex computation with overlapping live ranges */
    for (int i = 0; i < iterations; i++) {
        /* Create long dependency chains */
        v1 = v0 + v1 + side_effect(i);
        v2 = v1 * v2 - v0;
        v3 = v2 + v3 + (v1 >> 1);
        v4 = v3 * v4 - v2;
        v5 = v4 + v5 + (v3 & 0xFF);
        v6 = v5 * v6 - v4;
        v7 = v6 + v7 + (v5 ^ 0x55);
        v8 = v7 * v8 - v6;
        v9 = v8 + v9 + (v7 | 0xAA);
        v10 = v9 * v10 - v8;
        
        /* Mix in register variable */
        v0 = v10 + v0 + barrier;
        
        /* Second dependency chain */
        v11 = v10 + v11 + (v9 & 0xF);
        v12 = v11 * v12 - v10;
        v13 = v12 + v13 + (v11 << 2);
        v14 = v13 * v14 - v12;
        v15 = v14 + v15 + (v13 >> 1);
        v16 = v15 * v16 - v14;
        v17 = v16 + v17 + (v15 ^ v14);
        v18 = v17 * v18 - v16;
        v19 = v18 + v19 + (v17 | v16);
        v20 = v19 * v20 - v18;
        
        /* Conditional assignment creating different basic blocks */
        if (i & 1) {
            v21 = v20 + v21 + v0;
            v22 = v21 * v22 - v20;
        } else {
            v21 = v20 - v21 + v0;
            v22 = v21 / (v22 ? v22 : 1) + v20;
        }
        
        /* More computations with register clobbering asm */
        asm volatile ("" : "+r" (v0) : : "memory");
        
        v23 = v22 + v23 + v21;
        v24 = v23 * v24 - v22;
        v25 = v24 + v25 + (v23 % 256);
        v26 = v25 * v26 - v24;
        v27 = v26 + v27 + (v25 ^ 0xCC);
        v28 = v27 * v28 - v26;
        v29 = v28 + v29 + (v27 | 0x33);
        
        /* Use results in next iteration to maintain liveness */
        v0 = v29 + v0;
        barrier = i;  /* Volatile write */
    }
    
    /* Final computation using all variables */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                 v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
    
    return result & 0xFF;  /* Return meaningful result */
}

int main() {
    int total = 0;
    
    /* Call with different parameters to prevent constant propagation */
    for (int i = 1; i <= 10; i++) {
        total += test_remat(i * 10);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

/* Compile with: gcc -O2 -fno-schedule-insns -fno-schedule-insns2 -fno-peephole2 -o test test.c */
/* For debugging: gcc -O1 -dP -fdump-rtl-early_remat -o test test.c */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable helper to create side effects */
static int __attribute__((noinline)) side_effect(int x) {
    volatile int dummy = x;
    return dummy + 1;
}

/* Function to create high register pressure and trigger rematerialization */
static int __attribute__((noinline)) trigger_remat(int seed) {
    /* Declare many variables to create register pressure */
    register int v0 asm ("r12") = seed;
    int v1 = seed + 1;
    int v2 = seed * 2;
    int v3 = seed / 3;
    int v4 = seed ^ 0x55;
    int v5 = seed | 0xAA;
    int v6 = seed & 0xF0;
    int v7 = seed << 2;
    int v8 = seed >> 1;
    int v9 = seed + 100;
    int v10 = seed - 50;
    int v11 = seed * 3;
    int v12 = seed / 2;
    int v13 = seed ^ 0xFF;
    int v14 = seed | 0xCC;
    int v15 = seed & 0x0F;
    int v16 = seed << 1;
    int v17 = seed >> 2;
    int v18 = seed + 200;
    int v19 = seed - 100;
    
    /* Volatile variable to prevent optimization */
    volatile int barrier = 0;
    
    /* Complex computation with overlapping live ranges */
    for (int i = 0; i < 100; i++) {
        /* Create def-use chains across operations */
        v0 = v1 + v2 + barrier;
        v1 = v0 * v3 - side_effect(i);
        v2 = v1 / (v4 + 1) + v5;
        v3 = v2 ^ v6 | v7;
        v4 = v3 & v8 + v9;
        v5 = v4 << (v10 & 3);
        v6 = v5 >> (v11 % 4);
        v7 = v6 + v12 * v13;
        v8 = v7 - v14 / (v15 + 1);
        v9 = v8 | v16 & v17;
        v10 = v9 ^ v18 + v19;
        
        /* Cross-dependent computations to extend live ranges */
        v11 = v10 + v0 - v1;
        v12 = v11 * v2 / (v3 + 1);
        v13 = v12 | v4 & v5;
        v14 = v13 ^ v6 + v7;
        v15 = v14 << (v8 % 4);
        v16 = v15 >> (v9 & 3);
        v17 = v16 + v10 * v11;
        v18 = v17 - v12 / (v13 + 1);
        v19 = v18 | v14 & v15;
        
        /* Use asm to create register constraints */
        asm volatile ("# Force register usage" 
                     : "+r" (v0), "+r" (v1), "+r" (v2)
                     : "r" (v3), "r" (v4), "r" (v5)
                     : "memory");
        
        /* Conditional assignment to create different basic blocks */
        if (i & 1) {
            v0 = side_effect(v0);
            v1 = v1 * 2 + barrier;
        } else {
            v0 = v0 / 2 - barrier;
            v1 = side_effect(v1);
        }
        
        /* More arithmetic to increase computation density */
        v2 = v0 + v1 * 3 - v2 / 4;
        v3 = v2 | v3 & v4 ^ v5;
        v4 = v3 + v6 - v7 * v8;
        v5 = v4 / (v9 + 1) + v10;
        v6 = v5 ^ v11 | v12 & v13;
        v7 = v6 + v14 - v15 * v16;
        v8 = v7 / (v17 + 1) + v18;
        v9 = v8 ^ v19 | v0 & v1;
        
        /* Force memory barrier periodically */
        if (i % 10 == 0) {
            barrier = side_effect(i);
        }
    }
    
    /* Final computation using all variables */
    int result = v0 + v1 - v2 + v3 - v4 + v5 - v6 + v7 - v8 + v9
                 + v10 - v11 + v12 - v13 + v14 - v15 + v16 - v17 + v18 - v19;
    
    return result + barrier;
}

/* Main function to call test repeatedly */
int main() {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 10; i++) {
        int result = trigger_remat(i * 100);
        total += result;
        printf("Iteration %d: result = %d, total = %d\n", i, result, total);
    }
    
    return total != 0 ? 0 : 1;
}

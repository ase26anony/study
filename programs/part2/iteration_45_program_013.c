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

/* Function to create high register pressure and rematerialization candidates */
static int __attribute__((noinline)) create_register_pressure(int seed) {
    /* Declare many variables to create pseudo registers */
    register int r0 asm ("r12") = seed;
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
    int v20 = seed + 20;
    
    /* Volatile variable to prevent optimization */
    volatile int barrier = 0;
    
    /* Complex computation with overlapping live ranges */
    for (int i = 0; i < 100; i++) {
        /* Create def-use chains across operations */
        r0 = r0 + v1 + barrier;
        v1 = v2 * v3 - r0;
        v2 = v3 + v4 * side_effect(v1);
        v3 = v4 ^ v5 | v2;
        v4 = v5 + v6 - v3;
        v5 = v6 * v7 / (v4 + 1);
        v6 = v7 - v8 + v5;
        v7 = v8 | v9 & v6;
        v8 = v9 + v10 * v7;
        v9 = v10 - v11 + v8;
        v10 = v11 * v12 ^ v9;
        v11 = v12 + v13 - v10;
        v12 = v13 * v14 / (v11 + 1);
        v13 = v14 - v15 + v12;
        v14 = v15 | v16 & v13;
        v15 = v16 + v17 * v14;
        v16 = v17 - v18 + v15;
        v17 = v18 * v19 ^ v16;
        v18 = v19 + v20 - v17;
        v19 = v20 * r0 / (v18 + 1);
        v20 = r0 - v1 + v19;
        
        /* Conditional to create different basic blocks */
        if (i & 1) {
            /* Use asm to clobber registers and increase pressure */
            asm volatile ("" : : "r"(r0), "r"(v1), "r"(v2), "r"(v3), 
                         "r"(v4), "r"(v5), "r"(v6), "r"(v7));
            barrier = side_effect(i);
        } else {
            /* Different computation path to create remat candidates */
            r0 = r0 ^ v20;
            v1 = v1 | v19;
            v2 = v2 & v18;
            asm volatile ("" : : "r"(v8), "r"(v9), "r"(v10), "r"(v11));
        }
        
        /* Force values to be used in next iteration */
        barrier = barrier + 1;
    }
    
    /* Mix all results to ensure they're live at the end */
    int result = r0 + v1 - v2 + v3 - v4 + v5 - v6 + v7 - v8 + v9 
                 - v10 + v11 - v12 + v13 - v14 + v15 - v16 + v17 
                 - v18 + v19 - v20 + barrier;
    
    return result;
}

int main() {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 10; i++) {
        total += create_register_pressure(i * 100);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

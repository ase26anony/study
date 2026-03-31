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

/* Function to create high register pressure */
static int __attribute__((noinline)) create_register_pressure(int seed) {
    /* Declare many variables to create pseudo registers */
    register int v0 asm ("r12") = seed + 1;
    int v1 = seed + 2;
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
    int v18 = seed + 18;
    int v19 = seed + 19;
    int v20 = seed + 20;
    
    /* Volatile variable to prevent optimization */
    volatile int barrier = 0;
    
    /* Complex computation with overlapping live ranges */
    for (int i = 0; i < 100; i++) {
        /* Create def-use chains across operations */
        v1 = v0 + v1 + side_effect(i);
        v2 = v1 * v2 - side_effect(v1);
        v3 = v2 + v3 + (barrier ? 1 : 0);
        v4 = v3 * v4 - i;
        v5 = v4 + v5 + v0;
        v6 = v5 * v6 - v1;
        v7 = v6 + v7 + v2;
        v8 = v7 * v8 - v3;
        v9 = v8 + v9 + v4;
        v10 = v9 * v10 - v5;
        v11 = v10 + v11 + v6;
        v12 = v11 * v12 - v7;
        v13 = v12 + v13 + v8;
        v14 = v13 * v14 - v9;
        v15 = v14 + v15 + v10;
        v16 = v15 * v16 - v11;
        v18 = v16 + v18 + v12;
        v19 = v18 * v19 - v13;
        v20 = v19 + v20 + v14;
        v0 = v20 * v0 - v15;  /* Complete the cycle */
        
        /* Inline asm with clobbers to increase register pressure */
        asm volatile ("# Force clobber" : : : "memory", "r0", "r1", "r2", "r3");
        
        /* Conditional assignment to create different execution paths */
        if (i & 1) {
            v1 = v0 + v20;
            v3 = v2 + v19;
            v5 = v4 + v18;
        } else {
            v1 = v20 - v0;
            v3 = v19 - v2;
            v5 = v18 - v4;
        }
        
        /* Use results in subsequent iteration to maintain liveness */
        barrier = side_effect(v1 + v3 + v5);
    }
    
    /* Mix all results to ensure all variables are live at the end */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v18 + v19 + v20;
    
    return result & 0xFF;  /* Return something to prevent dead code elimination */
}

int main() {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 10; i++) {
        total += create_register_pressure(i * 17);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

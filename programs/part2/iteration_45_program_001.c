/* Compile with: gcc -O2 -fno-schedule-insns -fno-schedule-insns2 -fno-peephole2 -o test test.c */
/* For debugging: gcc -O1 -dP -fdump-rtl-early_remat -o test test.c */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable helper to create side effects */
static int __attribute__((noinline)) side_effect(int x) {
    static volatile int counter = 0;
    counter++;
    return x + (counter & 1);
}

/* Test function designed to create high register pressure */
static int test_rematerialization(int seed) {
    /* Declare many variables to create pseudo registers */
    register int v0 asm ("r12") = seed + 1;
    int v1 = seed * 2;
    int v2 = seed / 3;
    int v3 = seed - 5;
    int v4 = seed + 7;
    int v5 = seed * 11;
    int v6 = seed / 13;
    int v7 = seed - 17;
    int v8 = seed + 19;
    int v9 = seed * 23;
    int v10 = seed / 29;
    int v11 = seed - 31;
    int v12 = seed + 37;
    int v13 = seed * 41;
    int v14 = seed / 43;
    int v15 = seed - 47;
    int v16 = seed + 53;
    int v17 = seed * 59;
    int v18 = seed / 61;
    int v19 = seed - 67;
    int v20 = seed + 71;
    int v21 = seed * 73;
    int v22 = seed / 79;
    int v23 = seed - 83;
    int v24 = seed + 89;
    int v25 = seed * 97;
    int v26 = seed / 101;
    int v27 = seed - 103;
    int v28 = seed + 107;
    int v29 = seed * 109;
    
    /* Volatile variable to prevent optimization */
    volatile int barrier = 0;
    
    /* Complex computation with overlapping live ranges */
    for (int i = 0; i < 100; i++) {
        /* Create long dependency chains */
        v0 = v1 + v2 + side_effect(i);
        v1 = v0 * v3 - v4;
        v2 = v1 / (v5 + 1) + v6;
        v3 = v2 - v7 * v8;
        v4 = v3 + v9 / (v10 + 1);
        v5 = v4 * v11 - v12;
        v6 = v5 + v13 / (v14 + 1);
        v7 = v6 - v15 * v16;
        v8 = v7 + v17 / (v18 + 1);
        v9 = v8 * v19 - v20;
        v10 = v9 + v21 / (v22 + 1);
        v11 = v10 - v23 * v24;
        v12 = v11 + v25 / (v26 + 1);
        v13 = v12 * v27 - v28;
        v14 = v13 + v29 / (seed + 1);
        
        /* Use asm to clobber registers and increase pressure */
        asm volatile ("# Dummy asm" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3));
        
        /* Conditional assignments that create phi nodes */
        if (i & 1) {
            v15 = v14 + v0;
            v16 = v15 * v1;
        } else {
            v15 = v14 - v0;
            v16 = v15 / (v1 + 1);
        }
        
        v17 = v16 + v2;
        v18 = v17 * v3;
        v19 = v18 - v4;
        v20 = v19 + v5;
        
        /* Another asm with different clobbers */
        asm volatile ("# Another dummy" : : "r"(v15), "r"(v16), "r"(v17));
        
        if (i & 2) {
            v21 = v20 * v6;
            v22 = v21 - v7;
        } else {
            v21 = v20 / (v6 + 1);
            v22 = v21 + v7;
        }
        
        v23 = v22 * v8;
        v24 = v23 - v9;
        v25 = v24 + v10;
        v26 = v25 * v11;
        v27 = v26 - v12;
        v28 = v27 + v13;
        v29 = v28 * v14;
        
        /* Memory barrier to prevent reordering */
        barrier = i;
    }
    
    /* Mix all results to ensure they're live at the end */
    int result = v0 + v1 - v2 + v3 - v4 + v5 - v6 + v7 - v8 + v9 
                 - v10 + v11 - v12 + v13 - v14 + v15 - v16 + v17 
                 - v18 + v19 - v20 + v21 - v22 + v23 - v24 + v25 
                 - v26 + v27 - v28 + v29;
    
    return result + barrier;
}

int main() {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 10; i++) {
        total += test_rematerialization(i * 100);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

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

/* Force register usage with explicit asm clobbers */
#define USE_REGISTER(v) asm volatile("" : "+r" (v))

/* Main test function designed to create high register pressure */
static int __attribute__((noinline)) test_remat(int seed) {
    /* Declare many variables to create pseudo registers */
    register int r0 asm ("r12") = seed;
    int v1 = seed + 1;
    int v2 = seed * 2;
    int v3 = seed - 5;
    int v4 = seed ^ 0x1234;
    int v5 = seed * 3;
    int v6 = seed + 7;
    int v7 = seed / 2;
    int v8 = seed << 1;
    int v9 = seed >> 1;
    int v10 = seed | 0xABCD;
    int v11 = seed & 0xF0F0;
    int v12 = ~seed;
    int v13 = seed + 11;
    int v14 = seed * 5;
    int v15 = seed - 13;
    int v16 = seed ^ 0x5678;
    int v17 = seed * 7;
    int v18 = seed + 17;
    int v19 = seed / 3;
    int v20 = seed << 2;
    
    /* Volatile variable to prevent optimizations */
    volatile int barrier = 0;
    
    /* Complex computation with overlapping live ranges */
    for (int i = 0; i < 100; i++) {
        /* Create long dependency chains */
        r0 = r0 + v1 + side_effect(i);
        v1 = v1 * v2 - v3;
        v2 = v2 ^ v4 | v5;
        v3 = v3 + v6 * v7;
        v4 = v4 - v8 / (v9 + 1);
        v5 = v5 & v10 | v11;
        v6 = v6 + v12 - v13;
        v7 = v7 * v14 + v15;
        v8 = v8 ^ v16 & v17;
        v9 = v9 + v18 * v19;
        v10 = v10 - v20 / (r0 + 1);
        
        /* Use all variables in conditional to extend live ranges */
        if (i & 1) {
            v11 = v11 + r0;
            v12 = v12 - v1;
            v13 = v13 * v2;
            v14 = v14 ^ v3;
        } else {
            v15 = v15 + v4;
            v16 = v16 - v5;
            v17 = v17 * v6;
            v18 = v18 ^ v7;
        }
        
        /* Force register usage with asm */
        USE_REGISTER(r0);
        USE_REGISTER(v1);
        USE_REGISTER(v2);
        
        /* Memory barrier to prevent reordering */
        barrier = i;
        
        /* More computations with swapped variables */
        v19 = v8 + v9 * side_effect(v10);
        v20 = v11 - v12 / (v13 + 1);
        r0 = r0 ^ v14 & v15;
        v1 = v1 + v16 * v17;
        v2 = v2 - v18 / (v19 + 1);
        v3 = v3 ^ v20 & r0;
        
        /* Conditional with complex expression */
        if (side_effect(v1) > 0) {
            v4 = v4 + v2 * 3;
            v5 = v5 - v3 / 2;
        }
    }
    
    /* Final computation using all variables */
    int result = r0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    
    /* Add side effect to prevent dead code elimination */
    result += side_effect(result);
    
    return result;
}

int main() {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 10; i++) {
        total += test_remat(i * 12345);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

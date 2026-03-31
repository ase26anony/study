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

/* Main test function designed to create high register pressure */
static int test_remat(int seed) {
    /* Declare many variables to create pseudo registers */
    register int v0 asm ("r12") = seed + 1;
    int v1 = seed * 2;
    int v2 = seed / 3;
    int v3 = seed ^ 0x55AA;
    int v4 = seed << 2;
    int v5 = seed >> 1;
    int v6 = seed + 0x1234;
    int v7 = seed - 5678;
    int v8 = seed * seed;
    int v9 = ~seed;
    int v10 = seed | 0xFF00;
    int v11 = seed & 0x00FF;
    int v12 = seed % 17;
    int v13 = seed + v0;
    int v14 = v1 * v2;
    int v15 = v3 ^ v4;
    int v16 = v5 << v6;
    int v17 = v7 >> 1;
    int v18 = v8 + v9;
    int v19 = v10 - v11;
    int v20 = v12 * v13;
    int v21 = v14 ^ v15;
    int v22 = v16 | v17;
    int v23 = v18 & v19;
    int v24 = v20 + v21;
    int v25 = v22 - v23;
    int v26 = v24 * v25;
    int v27 = v26 ^ seed;
    int v28 = v27 << 3;
    int v29 = v28 >> 2;
    
    /* Volatile variable to prevent optimization */
    volatile int barrier = 0;
    
    /* Complex loop with dependent computations */
    for (int i = 0; i < 100; i++) {
        /* Create long dependency chains */
        v0 = side_effect(v0) + v1;
        v1 = v1 * v2 + i;
        v2 = v2 ^ v3;
        v3 = (v3 << 1) | (v4 >> 1);
        v4 = v4 + v5 - i;
        v5 = v5 * v6;
        v6 = v6 ^ v7;
        v7 = v7 + v8;
        v8 = v8 - v9;
        v9 = v9 * v10;
        v10 = v10 ^ v11;
        v11 = v11 + v12;
        v12 = v12 - v13;
        v13 = v13 * v14;
        v14 = v14 ^ v15;
        v15 = v15 + v16;
        v16 = v16 - v17;
        v17 = v17 * v18;
        v18 = v18 ^ v19;
        v19 = v19 + v20;
        v20 = v20 - v21;
        v21 = v21 * v22;
        v22 = v22 ^ v23;
        v23 = v23 + v24;
        v24 = v24 - v25;
        v25 = v25 * v26;
        v26 = v26 ^ v27;
        v27 = v27 + v28;
        v28 = v28 - v29;
        v29 = v29 * v0;
        
        /* Use barrier to create side effects */
        barrier = i;
        
        /* Conditional to create different basic blocks */
        if (i & 1) {
            v0 = v0 + barrier;
            v1 = v1 - barrier;
        } else {
            v0 = v0 - barrier;
            v1 = v1 + barrier;
        }
        
        /* More arithmetic to extend live ranges */
        v2 = v2 + (v3 * 2);
        v3 = v3 - (v4 / 2);
        v4 = v4 ^ (v5 << 1);
        v5 = v5 | (v6 >> 1);
        v6 = v6 & v7;
        v7 = v7 + v8 * 3;
        v8 = v8 - v9 / 3;
        v9 = v9 ^ v10;
        v10 = v10 + v11;
        v11 = v11 - v12;
        v12 = v12 * v13;
        v13 = v13 ^ v14;
        v14 = v14 + v15;
        v15 = v15 - v16;
        v16 = v16 * v17;
        v17 = v17 ^ v18;
        v18 = v18 + v19;
        v19 = v19 - v20;
        v20 = v20 * v21;
        v21 = v21 ^ v22;
        v22 = v22 + v23;
        v23 = v23 - v24;
        v24 = v24 * v25;
        v25 = v25 ^ v26;
        v26 = v26 + v27;
        v27 = v27 - v28;
        v28 = v28 * v29;
        v29 = v29 ^ v0;
    }
    
    /* Final computation using all variables */
    int result = v0 + v1 - v2 + v3 - v4 + v5 - v6 + v7 - v8 + v9
                 - v10 + v11 - v12 + v13 - v14 + v15 - v16 + v17
                 - v18 + v19 - v20 + v21 - v22 + v23 - v24 + v25
                 - v26 + v27 - v28 + v29;
    
    /* Use inline asm to clobber registers and increase pressure */
    asm volatile ("# Force register clobber" 
                  : 
                  : "r" (v0), "r" (v1), "r" (v2), "r" (v3), "r" (v4),
                    "r" (v5), "r" (v6), "r" (v7), "r" (v8), "r" (v9),
                    "r" (v10), "r" (v11), "r" (v12), "r" (v13), "r" (v14),
                    "r" (v15), "r" (v16), "r" (v17), "r" (v18), "r" (v19),
                    "r" (v20), "r" (v21), "r" (v22), "r" (v23), "r" (v24),
                    "r" (v25), "r" (v26), "r" (v27), "r" (v28), "r" (v29)
                  : "memory");
    
    return result + barrier;
}

int main() {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 10; i++) {
        total += test_remat(i * 100);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

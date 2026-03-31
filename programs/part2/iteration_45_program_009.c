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
    int v19 = v10 & v11;
    int v20 = v12 | v13;
    int v21 = v14 ^ v15;
    int v22 = v16 + v17;
    int v23 = v18 * v19;
    int v24 = v20 / (v21 + 1);
    int v25 = v22 % (v23 + 1);
    int v26 = v24 << 3;
    int v27 = v25 >> 2;
    int v28 = v26 | v27;
    int v29 = v28 ^ seed;
    
    /* Volatile variable to prevent optimization */
    volatile int barrier = 0;
    
    /* Complex loop with dependent operations to extend live ranges */
    for (int i = 0; i < 100; i++) {
        /* Mix in side effects to create non-pure computations */
        int s = side_effect(i);
        
        /* Long chain of dependent operations using many variables */
        v0 = v1 + v2 + s;
        v1 = v3 * v4 - v0;
        v2 = v5 ^ v6 | v1;
        v3 = v7 + v8 - v2;
        v4 = v9 & v10 ^ v3;
        v5 = v11 << 2 | v4;
        v6 = v12 >> 1 + v5;
        v7 = v13 * v14 - v6;
        v8 = v15 + v16 ^ v7;
        v9 = v17 & v18 | v8;
        v10 = v19 << 3 - v9;
        v11 = v20 >> 2 + v10;
        v12 = v21 * v22 - v11;
        v13 = v23 ^ v24 | v12;
        v14 = v25 + v26 - v13;
        v15 = v27 & v28 ^ v14;
        v16 = v29 << 1 | v15;
        
        /* Use barrier to prevent reordering */
        if (barrier) {
            v17 = v0 + v1;
            v18 = v2 * v3;
        }
        
        /* More computations to increase pressure */
        v19 = v4 + v5 + i;
        v20 = v6 * v7 - i;
        v21 = v8 ^ v9 | i;
        v22 = v10 + v11 - i;
        v23 = v12 & v13 ^ i;
        v24 = v14 << (i & 3);
        v25 = v15 >> (i & 3);
        v26 = v16 | v17;
        v27 = v18 ^ v19;
        v28 = v20 + v21;
        v29 = v22 * v23;
        
        /* Conditional that creates different basic blocks */
        if (i & 1) {
            v0 = v24 + v25;
            v1 = v26 * v27;
        } else {
            v2 = v28 ^ v29;
            v3 = v0 & v1;
        }
        
        /* Use asm to clobber registers and increase pressure */
        asm volatile ("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3), 
                      "r"(v4), "r"(v5), "r"(v6), "r"(v7));
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
    for (int i = 0; i < 10; i++) {
        total += test_remat(i * 12345);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

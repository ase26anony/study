/* Compile with: gcc -O2 -fno-schedule-insns -fno-schedule-insns2 -fno-peephole2 -o test test.c */
/* For debugging: gcc -O1 -dP -fdump-rtl-early_remat -o test test.c */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable helper to prevent optimization */
static int __attribute__((noinline)) helper(int x) {
    static volatile int counter = 0;
    counter++;
    return x + (counter & 1);
}

/* Force register pressure test function */
static int __attribute__((noinline)) test_remat_pressure(int seed) {
    /* Declare many variables to create register pressure */
    register int v0 asm ("r12") = seed + 1;
    int v1 = seed * 2;
    int v2 = seed / 3;
    int v3 = seed ^ 0x55AA;
    int v4 = seed << 2;
    int v5 = seed >> 1;
    int v6 = seed + 100;
    int v7 = seed - 50;
    int v8 = seed * seed;
    int v9 = helper(seed);
    int v10 = v0 + v1;
    int v11 = v2 * v3;
    int v12 = v4 | v5;
    int v13 = v6 & v7;
    int v14 = v8 ^ v9;
    int v15 = v10 + v11;
    int v16 = v12 - v13;
    int v17 = v14 * v15;
    int v18 = v16 ^ v17;
    int v19 = helper(v18);
    int v20 = v19 + v0;
    int v21 = v1 - v2;
    int v22 = v3 * v4;
    int v23 = v5 | v6;
    int v24 = v7 & v8;
    int v25 = v9 ^ v10;
    int v26 = v11 + v12;
    int v27 = v13 - v14;
    int v28 = v15 * v16;
    int v29 = v17 ^ v18;
    
    /* Volatile side effect to prevent reordering */
    volatile int barrier = 0;
    barrier = v19;
    
    /* Complex conditional with overlapping live ranges */
    int result = 0;
    for (int i = 0; i < 4; i++) {
        /* Create def-use chains across iterations */
        if (i & 1) {
            v0 = v1 + v2;
            v3 = v4 * v5;
            v6 = v7 | v8;
            v9 = v10 ^ v11;
            result += v0 + v3;
        } else {
            v12 = v13 - v14;
            v15 = v16 * v17;
            v18 = v19 | v20;
            v21 = v22 ^ v23;
            result += v12 + v15;
        }
        
        /* Mix in asm to create register constraints */
        asm volatile ("# Dummy asm" : "+r" (v0), "+r" (v1), "+r" (v2) : : "memory");
        
        /* More computations to extend live ranges */
        v24 = v25 + v26;
        v27 = v28 - v29;
        v0 = v1 * v2;
        v3 = v4 | v5;
        
        /* Use helper to create side effects */
        v6 = helper(v6 + i);
        
        /* Cross-iteration dependencies */
        v7 = v8 + v9;
        v10 = v11 * v12;
        v13 = v14 | v15;
        v16 = v17 ^ v18;
        
        /* Another volatile access */
        barrier = i;
    }
    
    /* Final mixing of all values */
    result += v0 + v1 + v2 + v3 + v4 + v5;
    result += v6 + v7 + v8 + v9 + v10 + v11;
    result += v12 + v13 + v14 + v15 + v16 + v17;
    result += v18 + v19 + v20 + v21 + v22 + v23;
    result += v24 + v25 + v26 + v27 + v28 + v29;
    
    return result;
}

int main() {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_remat_pressure(i);
        total += test_remat_pressure(i * 3);
        total += test_remat_pressure(i ^ 0x1234);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

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
static int __attribute__((noinline)) test_remat(int seed) {
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
    int v16 = v5 + v6;
    int v17 = v7 - v8;
    int v18 = v9 * v10;
    int v19 = v11 & v12;
    int v20 = v13 | v14;
    int v21 = v15 ^ v16;
    int v22 = v17 + v18;
    int v23 = v19 - v20;
    int v24 = v21 * v22;
    int v25 = v23 ^ v24;
    
    /* Volatile variable to prevent optimization */
    volatile int barrier = 0;
    
    /* Complex loop with dependent computations to extend live ranges */
    for (int i = 0; i < 100; i++) {
        /* Create def-use chains across iterations */
        v0 = v1 + side_effect(i);
        v1 = v2 * v0;
        v2 = v3 ^ v1;
        v3 = v4 + v2;
        v4 = v5 - v3;
        v5 = v6 * v4;
        v6 = v7 ^ v5;
        v7 = v8 + v6;
        v8 = v9 - v7;
        v9 = v10 * v8;
        v10 = v11 ^ v9;
        v11 = v12 + v10;
        v12 = v13 - v11;
        v13 = v14 * v12;
        v14 = v15 ^ v13;
        v15 = v16 + v14;
        v16 = v17 - v15;
        v17 = v18 * v16;
        v18 = v19 ^ v17;
        v19 = v20 + v18;
        v20 = v21 - v19;
        v21 = v22 * v20;
        v22 = v23 ^ v21;
        v23 = v24 + v22;
        v24 = v25 - v23;
        v25 = barrier + v24;  /* Use volatile to create memory barrier */
        
        /* Conditional to create basic block boundaries */
        if (i & 1) {
            /* Different computation path to create divergent live ranges */
            v0 = v25 * 2;
            v5 = v0 + side_effect(v0);
            v10 = v5 ^ 0xAA;
            v15 = v10 | 0x55;
            v20 = v15 & 0xFF;
        } else {
            /* Alternative path with overlapping computations */
            v1 = v25 / 3;
            v6 = v1 ^ side_effect(v1);
            v11 = v6 | 0xCC;
            v16 = v11 & 0x33;
            v21 = v16 + 0x11;
        }
        
        /* Inline asm to clobber registers and increase pressure */
        asm volatile ("# Dummy asm" : : : "memory", "r0", "r1", "r2", "r3", "r4", "r5");
        
        /* Use results in conditional to keep them live */
        barrier = (v0 + v5 + v10 + v15 + v20 + v1 + v6 + v11 + v16 + v21) & 1;
    }
    
    /* Final computation using all variables to ensure they're live at end */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                 v20 + v21 + v22 + v23 + v24 + v25;
    
    return result ^ barrier;
}

/* Multiple test functions to create different register allocation patterns */
static int __attribute__((noinline)) test_remat2(int seed) {
    /* Similar but different computation pattern */
    int a = seed * 3;
    int b = seed + 7;
    int c = seed ^ 0x1234;
    int d = seed << 3;
    int e = seed >> 2;
    
    for (int i = 0; i < 50; i++) {
        a = b + side_effect(a);
        b = c * a;
        c = d ^ b;
        d = e + c;
        e = a - d;
        
        /* Create pseudo register references that might be rematerialized */
        int t1 = a * b;
        int t2 = c + d;
        int t3 = e ^ t1;
        int t4 = t2 & t3;
        int t5 = t4 | a;
        
        asm volatile ("# Another dummy" : : : "memory");
        
        if (i % 3 == 0) {
            a = t5 + side_effect(t5);
        }
    }
    
    return a + b + c + d + e;
}

int main() {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += test_remat(i);
        total += test_remat2(i * 7 + 3);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

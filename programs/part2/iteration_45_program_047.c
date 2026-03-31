/* Compile with: gcc -O2 -fno-schedule-insns -fno-schedule-insns2 -fno-peephole2 -o test test.c */
/* For debugging: gcc -O1 -dP -fdump-rtl-early_remat -o test test.c */

#include <stdio.h>
#include <stdlib.h>

/* Non-inlineable helper to create side effects */
static int __attribute__((noinline)) side_effect(int x) {
    static volatile int counter = 0;
    counter++;
    return x + (counter & 1);
}

/* Force register usage with asm clobbers */
#define CLOBBER_REGS asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15")

/* Main test function designed to create high register pressure */
static int __attribute__((noinline)) test_remat(int seed) {
    /* Declare many variables to create pseudo registers */
    register int r0 asm ("r12") = seed;
    register int r1 asm ("r11") = seed + 1;
    int v2 = seed * 2;
    int v3 = seed / 3;
    int v4 = seed ^ 0x55;
    int v5 = seed | 0xAA;
    int v6 = seed & 0xF0;
    int v7 = seed << 2;
    int v8 = seed >> 1;
    int v9 = ~seed;
    int v10 = seed + 100;
    int v11 = seed - 50;
    int v12 = seed * 3;
    int v13 = seed / 2;
    int v14 = seed ^ 0xFF;
    int v15 = seed | 0xCC;
    int v16 = seed & 0x0F;
    int v17 = seed << 3;
    int v18 = seed >> 2;
    int v19 = seed + 200;
    int v20 = seed - 100;
    int v21 = seed * 5;
    int v22 = seed / 4;
    int v23 = seed ^ 0xAA;
    int v24 = seed | 0x55;
    int v25 = seed & 0xF5;
    int v26 = seed << 1;
    int v27 = seed >> 3;
    
    volatile int memory_barrier = 0;
    
    /* Complex computation with overlapping live ranges */
    for (int i = 0; i < 100; i++) {
        /* Create long dependency chains */
        r0 = r1 + v2 + side_effect(i);
        v3 = r0 * v4 - v5;
        v6 = v3 ^ v7 | v8;
        v9 = v6 + v10 - v11;
        v12 = v9 * v13 / (v14 + 1);
        v15 = v12 | v16 & v17;
        r1 = v15 + v18 - v19;
        v20 = r1 * v21 + v22;
        v23 = v20 ^ v24 | v25;
        v26 = v23 + v27 - r0;
        v2 = v26 * v3 / (v4 + 1);
        v5 = v2 | v6 & v7;
        v8 = v5 + v9 - v10;
        v11 = v8 * v12 + v13;
        v14 = v11 ^ v15 | v16;
        v17 = v14 + v18 - v19;
        v22 = v17 * v20 / (v21 + 1);
        v25 = v22 | v23 & v24;
        v27 = v25 + v26 - r1;
        
        /* Force spill/reload candidates with memory access */
        memory_barrier = i;
        
        /* Mix in register variables */
        if (i & 1) {
            r0 = side_effect(r0) + v2;
            r1 = side_effect(r1) - v3;
        } else {
            r0 = side_effect(r0) - v2;
            r1 = side_effect(r1) + v3;
        }
        
        /* Create conditional uses that extend live ranges */
        v4 = (i % 3 == 0) ? r0 : r1;
        v7 = (i % 5 == 0) ? v4 + v5 : v4 - v5;
        
        /* Clobber registers to increase pressure */
        CLOBBER_REGS;
    }
    
    /* Final computation using all variables */
    int result = r0 + r1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 +
                 v19 + v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27;
    
    return result ^ memory_barrier;
}

/* Alternative test with different optimization characteristics */
static int __attribute__((noinline)) test_remat2(int seed) {
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    int i = seed + 8, j = seed + 9, k = seed + 10, l = seed + 11;
    int m = seed + 12, n = seed + 13, o = seed + 14, p = seed + 15;
    
    for (int iter = 0; iter < 50; iter++) {
        /* Duplicate computations - candidates for rematerialization */
        int t1 = a * b + c;
        int t2 = a * b + c;  /* Same computation */
        
        int t3 = d * e - f;
        int t4 = d * e - f;  /* Same computation */
        
        int t5 = g ^ h | i;
        int t6 = g ^ h | i;  /* Same computation */
        
        /* Use results in ways that create overlapping live ranges */
        a = t1 + t3;
        b = t2 - t4;
        c = t5 * t1;
        d = t6 / t2;
        
        /* Force side effects that prevent motion */
        volatile int* ptr = (volatile int*)&a;
        *ptr = side_effect(*ptr);
        
        /* More duplicate computations */
        e = (t1 + t2) * 3;
        f = (t1 + t2) * 3;  /* Same computation */
        
        g = (t3 | t4) ^ 0xFF;
        h = (t3 | t4) ^ 0xFF;  /* Same computation */
        
        /* Conditional that uses many variables */
        if (iter & 1) {
            i = a + b + c + d;
            j = e + f + g + h;
        } else {
            i = a - b - c - d;
            j = e - f - g - h;
        }
        
        k = i * j;
        l = k + side_effect(iter);
        m = l ^ seed;
        n = m | 0x55;
        o = n & 0xAA;
        p = o << 2;
    }
    
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
}

int main() {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 10; i++) {
        total += test_remat(i * 100);
        total += test_remat2(i * 50 + 1);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

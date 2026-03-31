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

/* Test function designed to create high register pressure */
static int __attribute__((noinline)) trigger_remat(int seed) {
    /* Declare many variables to create pseudo registers */
    register int v0 asm ("r12") = seed + 1;
    int v1 = seed * 2;
    int v2 = seed / 3;
    int v3 = seed ^ 0x55;
    int v4 = seed << 2;
    int v5 = seed >> 1;
    int v6 = seed | 0xFF;
    int v7 = seed & 0x7F;
    int v8 = seed + 100;
    int v9 = seed - 50;
    int v10 = seed * 3;
    int v11 = seed / 2;
    int v12 = seed ^ 0xAA;
    int v13 = seed << 1;
    int v14 = seed >> 2;
    int v15 = seed | 0x7F;
    int v16 = seed & 0x3F;
    int v17 = seed + 200;
    int v18 = seed - 100;
    int v19 = seed * 5;
    int v20 = seed / 4;
    int v21 = seed ^ 0x33;
    int v22 = seed << 3;
    int v23 = seed >> 3;
    int v24 = seed | 0x1F;
    int v25 = seed & 0x1F;
    
    volatile int barrier = 0;
    
    /* Complex computation with overlapping live ranges */
    for (int i = 0; i < 100; i++) {
        /* Create dependent chains that span multiple iterations */
        v0 = v1 + v2 + side_effect(i);
        v3 = v0 * v4 - v5;
        v6 = v3 ^ v7 | v8;
        v9 = v6 + v10 * v11;
        v12 = v9 - v13 / (v14 + 1);
        v15 = v12 | v16 & v17;
        v18 = v15 + v19 - v20;
        v21 = v18 * v22 ^ v23;
        v24 = v21 + v25 - v0;
        v1 = v24 * v2 + v3;
        v4 = v1 - v5 ^ v6;
        v7 = v4 | v8 & v9;
        v10 = v7 + v11 * v12;
        v13 = v10 - v14 / (v15 + 1);
        v16 = v13 | v17 & v18;
        v19 = v16 + v20 - v21;
        v22 = v19 * v23 ^ v24;
        v25 = v22 + v0 - v1;
        v2 = v25 * v3 + v4;
        
        /* Memory barrier to prevent reordering */
        barrier = i;
        
        /* More computation with register variables */
        asm volatile ("" : "+r" (v0), "+r" (v1), "+r" (v2) : : "memory");
        
        v5 = v2 + v6 * v7;
        v8 = v5 - v9 / (v10 + 1);
        v11 = v8 | v12 & v13;
        v14 = v11 + v15 - v16;
        v17 = v14 * v18 ^ v19;
        v20 = v17 + v21 - v22;
        v23 = v20 * v24 + v25;
        
        /* Conditional to create basic block boundaries */
        if (i & 1) {
            v0 = v23 + v1;
            v2 = v0 * v3;
        } else {
            v0 = v23 - v1;
            v2 = v0 / (v3 + 1);
        }
        
        /* Use results in next iteration */
        v3 = v2 + side_effect(v0);
    }
    
    /* Final computation using all variables */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                 v20 + v21 + v22 + v23 + v24 + v25;
    
    return result & 0xFF;
}

/* Alternative test with different optimization characteristics */
static int __attribute__((noinline)) trigger_remat_alt(int seed) {
    int vars[30];
    
    /* Initialize array with different values */
    for (int i = 0; i < 30; i++) {
        vars[i] = seed + i * 7;
    }
    
    volatile int sync = 0;
    
    /* Unrolled computation to create many pseudo registers */
    for (int i = 0; i < 50; i++) {
        vars[0] = vars[1] + vars[2] * vars[3];
        vars[4] = vars[0] - vars[5] / (vars[6] + 1);
        vars[7] = vars[4] | vars[8] & vars[9];
        vars[10] = vars[7] + vars[11] - vars[12];
        vars[13] = vars[10] * vars[14] ^ vars[15];
        vars[16] = vars[13] + vars[17] - vars[18];
        vars[19] = vars[16] * vars[20] + vars[21];
        vars[22] = vars[19] - vars[23] / (vars[24] + 1);
        vars[25] = vars[22] | vars[26] & vars[27];
        vars[28] = vars[25] + vars[29] - vars[0];
        vars[1] = vars[28] * vars[2] + vars[3];
        
        sync = i;
        
        /* Inline asm to clobber registers */
        asm volatile ("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7");
        
        if (i % 3 == 0) {
            vars[5] = vars[1] + side_effect(vars[2]);
        }
    }
    
    int sum = 0;
    for (int i = 0; i < 30; i++) {
        sum += vars[i];
    }
    
    return sum;
}

int main() {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 1000; i++) {
        total += trigger_remat(i);
        total += trigger_remat_alt(i * 3);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

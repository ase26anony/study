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

/* Force register usage with explicit register variables */
register int r0 asm ("r12");
register int r1 asm ("r13");

/* Main test function designed to create high register pressure */
static int __attribute__((noinline)) create_register_pressure(int seed) {
    /* Declare many local variables to create pseudo registers */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    
    /* Initialize with seed to prevent constant propagation */
    v0 = seed;
    v1 = side_effect(seed + 1);
    v2 = seed * 2;
    v3 = side_effect(seed + 3);
    v4 = seed / 2;
    v5 = side_effect(seed + 5);
    
    /* Use explicit register variables mixed with stack variables */
    r0 = v0 + v1;
    r1 = v2 - v3;
    
    /* Create complex dependency chain across basic blocks */
    for (int i = 0; i < 100; i++) {
        /* First basic block with arithmetic chain */
        v6 = v0 + v1 + i;
        v7 = v2 * v3 - i;
        v8 = v4 ^ v5 ^ i;
        v9 = v6 + v7 + v8;
        
        /* Conditional assignment creating different def-use chains */
        if (i & 1) {
            v10 = v9 * 3;
            v11 = side_effect(v10);
            v12 = v11 - r0;
        } else {
            v10 = v9 / 3;
            v11 = side_effect(v10);
            v12 = v11 + r1;
        }
        
        /* More arithmetic operations preventing register reuse */
        v13 = v10 * v11;
        v14 = v12 ^ v13;
        v15 = v14 + v6;
        v16 = v15 - v7;
        v17 = v16 * v8;
        v18 = v17 / (v9 + 1);
        v19 = v18 ^ v10;
        
        /* Use inline asm to create register pressure and clobbers */
        asm volatile ("# Force register clobber" 
                     : "=r" (v20), "=r" (v21), "=r" (v22)
                     : "0" (v13), "1" (v14), "2" (v15)
                     : "memory");
        
        /* Continue dependency chain */
        v23 = v20 + v21;
        v24 = v22 * v23;
        v25 = v24 - v16;
        v26 = v25 ^ v17;
        v27 = v26 + v18;
        v28 = v27 - v19;
        v29 = v28 * v20;
        
        /* Rotate values to maintain liveness across iterations */
        v0 = v29 & 0xFF;
        v1 = v21 + 1;
        v2 = v22 - 1;
        v3 = v23 * 2;
        v4 = v24 / 2;
        v5 = v25 ^ 0x55;
        
        /* Update register variables */
        r0 = r0 + v26;
        r1 = r1 ^ v27;
    }
    
    /* Final computation using all variables */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                 v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 +
                 r0 + r1;
    
    return result & 0xFFFF; /* Prevent overflow issues */
}

int main() {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 10; i++) {
        int result = create_register_pressure(i * 100);
        total += result;
        printf("Iteration %d: result = %d, total = %d\n", i, result, total);
    }
    
    return total != 0 ? 0 : 1;
}

/* Compile with: gcc -O2 -fno-schedule-insns -fno-schedule-insns2 -fno-peephole2 -o test test.c */
/* For debugging: gcc -O1 -dP -fdump-rtl-early_remat -o test test.c */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable helper to prevent optimization */
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
    
    /* Initialize with seed and side effects to prevent constant propagation */
    v0 = seed;
    v1 = side_effect(seed + 1);
    v2 = v0 + v1;
    v3 = side_effect(v2 * 2);
    
    /* Complex dependency chain across many variables */
    v4 = v1 * v2 - v3;
    v5 = v2 + v3 * v4;
    v6 = v3 - v4 / (v5 + 1);
    v7 = v4 * v5 + v6;
    v8 = v5 - v6 * v7;
    v9 = v6 + v7 / (v8 + 1);
    
    v10 = v7 * v8 - v9;
    v11 = v8 + v9 * v10;
    v12 = v9 - v10 / (v11 + 1);
    v13 = v10 * v11 + v12;
    v14 = v11 - v12 * v13;
    v15 = v12 + v13 / (v14 + 1);
    
    v16 = v13 * v14 - v15;
    v17 = v14 + v15 * v16;
    v18 = v15 - v16 / (v17 + 1);
    v19 = v16 * v17 + v18;
    v20 = v17 - v18 * v19;
    
    /* Use explicit register variables in computation */
    r0 = v18 + v19;
    r1 = v19 - v20;
    
    /* More computations mixing register and stack variables */
    v21 = r0 * v20 + side_effect(r1);
    v22 = r1 - v21 * r0;
    v23 = v21 + v22 / (r0 + 1);
    v24 = v22 * v23 - r1;
    v25 = v23 + v24 * side_effect(v25);
    
    /* Conditional assignments to create different basic blocks */
    if (seed & 1) {
        v26 = v24 * 3 + r0;
        v27 = v25 - r1 * 2;
    } else {
        v26 = v24 / 3 + r0;
        v27 = v25 + r1 * 2;
    }
    
    /* Loop with dependent computations to extend live ranges */
    for (int i = 0; i < 4; i++) {
        v28 = v26 + v27 * i;
        v29 = v27 - v28 / (i + 2);
        
        /* Inline asm to create register clobbering */
        asm volatile ("# Force register clobber" : : : "memory");
        
        /* Recompute values to encourage rematerialization */
        v26 = side_effect(v28) + v29;
        v27 = v28 - side_effect(v29);
    }
    
    /* Final computation using all variables */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                 v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 +
                 r0 + r1;
    
    return result;
}

int main() {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += create_register_pressure(i);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

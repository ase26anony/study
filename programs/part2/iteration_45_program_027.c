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
register int reg1 asm ("r12");
register int reg2 asm ("r13");

/* Main test function designed to create high register pressure */
static int __attribute__((noinline)) create_register_pressure(int seed) {
    /* Declare many local variables to create pseudo registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Initialize with seed and side effects */
    v1 = seed;
    v2 = side_effect(seed) * 3;
    v3 = v1 + v2;
    v4 = side_effect(v3) - 7;
    
    /* Use explicit register variables in computation */
    reg1 = v4;
    reg2 = v3;
    
    /* Complex dependent computations to create long live ranges */
    for (int i = 0; i < 100; i++) {
        /* Unrolled sequence of dependent operations */
        v5 = v1 * v2 + reg1;
        v6 = v3 - v4 * reg2;
        v7 = side_effect(v5) | v6;
        v8 = v7 ^ v1;
        v9 = v8 + v2 - v3;
        v10 = v9 * v4 / (reg1 + 1);
        
        v11 = v5 + v6 + v7;
        v12 = v8 - v9 + v10;
        v13 = side_effect(v11) & v12;
        v14 = v13 | v5;
        v15 = v14 * v6;
        v16 = v15 / (v7 + 1);
        
        v17 = v8 + v9 + v10 + v11;
        v18 = v12 - v13 + v14 - v15;
        v19 = side_effect(v16) ^ v17;
        v20 = v18 & v19;
        v21 = v20 | v8;
        v22 = v21 * v9;
        
        v23 = v10 + v11 + v12 + v13;
        v24 = v14 - v15 + v16 - v17;
        v25 = side_effect(v18) | v19;
        v26 = v20 ^ v21;
        v27 = v22 & v23;
        v28 = v24 | v25;
        
        v29 = v26 + v27 + v28;
        v30 = side_effect(v29) * 2;
        
        /* Mix results back to create circular dependencies */
        v1 = v30 - v29;
        v2 = v28 + v27;
        v3 = v26 | v25;
        v4 = v24 & v23;
        
        /* Update register variables with side effects */
        reg1 = side_effect(reg1 + i);
        reg2 = side_effect(reg2 - i);
    }
    
    /* Conditional assignments to create different basic blocks */
    if (seed & 1) {
        v1 = v2 + v3;
        v4 = v5 - v6;
        reg1 = v7 * v8;
    } else {
        v1 = v3 - v2;
        v4 = v6 + v5;
        reg1 = v8 / (v7 + 1);
    }
    
    /* More computations using all variables */
    v9 = v1 + v2 + v3 + v4;
    v10 = reg1 * reg2;
    v11 = side_effect(v9) - side_effect(v10);
    
    /* Final result uses many variables to keep them live */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + 
           v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
           v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30 +
           reg1 + reg2;
}

/* Alternative test with different optimization characteristics */
static int __attribute__((noinline)) create_remat_candidate(int x) {
    /* Create identical computations in multiple places */
    int a = x * 3 + 7;
    int b = side_effect(a) - 5;
    
    /* Duplicate computation that could be rematerialized */
    int c = x * 3 + 7;  /* Same as 'a' */
    int d = side_effect(c) - 5;  /* Same as 'b' */
    
    /* Use in complex expression */
    int e = (a + b) * (c + d);
    
    /* More duplicates */
    int f = x * 3 + 7;  /* Another duplicate */
    int g = side_effect(f) - 5;
    
    /* Force different execution paths */
    if (e > 1000) {
        return a + c + f + side_effect(g);
    } else {
        return b + d + g + side_effect(a);
    }
}

int main() {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total += create_register_pressure(i);
        total += create_remat_candidate(i * 17);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

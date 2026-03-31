/* early_remat_trigger.c
 * Designed to trigger GCC's early rematerialization pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fno-web -fno-gcse early_remat_trigger.c -o early_remat_trigger
 * Or for aggressive remat: gcc -O3 -fearly-remat -fno-rename-registers -fno-tree-pre early_remat_trigger.c -o early_remat_trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and interprocedural optimization */
__attribute__((noinline, noipa))
static int high_pressure_compute(volatile int a, volatile int b, 
                                 volatile int c, volatile int d,
                                 volatile int iterations) {
    volatile int result = 0;
    
    /* Loop to create multiple basic blocks */
    for (volatile int i = 0; i < iterations; i++) {
        /* Declare many local variables to create high register pressure */
        int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
        
        /* Complex expression that will be reused - candidate for rematerialization */
        int complex_expr = (a * b) + (c << 2) - d;
        
        /* First block of independent arithmetic operations */
        v1 = a + b + i;
        v2 = b * c - i;
        v3 = c ^ d ^ i;
        v4 = d << (a & 3);
        v5 = (a * b) + (c << 2) - d;  /* Same as complex_expr - potential remat */
        
        /* Compiler barrier to prevent reordering */
        __asm__ volatile ("" : : : "memory");
        
        v6 = v1 * v2 + v3;
        v7 = v2 | v3 | v4;
        v8 = v3 & v4 & v5;
        v9 = v4 ^ v5 ^ v1;
        v10 = (a * b) + (c << 2) - d;  /* Another copy of complex_expr */
        
        /* More operations to increase register pressure */
        v11 = v5 + v6 + v7;
        v12 = v6 * v7 * v8;
        v13 = v7 | v8 | v9;
        v14 = v8 & v9 & v10;
        v15 = v9 ^ v10 ^ v5;
        
        __asm__ volatile ("" : : : "memory");
        
        /* Conditional branch to split basic block */
        volatile int condition = a & 1;
        if (condition) {
            /* Branch 1: More computations */
            v16 = v10 + v11 + v12;
            v17 = v11 * v12 * v13;
            v18 = v12 | v13 | v14;
            v19 = v13 & v14 & v15;
            v20 = (a * b) + (c << 2) - d;  /* Another copy */
            
            v21 = v14 + v15 + v16;
            v22 = v15 * v16 * v17;
            v23 = v16 | v17 | v18;
            v24 = v17 & v18 & v19;
            v25 = v18 ^ v19 ^ v20;
        } else {
            /* Branch 2: Different computations */
            v16 = v10 - v11 - v12;
            v17 = v11 / (v12 ? v12 : 1);
            v18 = v12 & v13 & v14;
            v19 = v13 | v14 | v15;
            v20 = (a * b) + (c << 2) - d;  /* Another copy */
            
            v21 = v14 - v15 - v16;
            v22 = v15 / (v16 ? v16 : 1);
            v23 = v16 & v17 & v18;
            v24 = v17 | v18 | v19;
            v25 = v18 ^ v19 ^ v20;
        }
        
        __asm__ volatile ("" : : : "memory");
        
        /* Merge point - more computations using values from both branches */
        v26 = v20 + v21 + v22;
        v27 = v21 * v22 * v23;
        v28 = v22 | v23 | v24;
        v29 = v23 & v24 & v25;
        v30 = (a * b) + (c << 2) - d;  /* Final copy of complex_expr */
        
        /* Use all variables to prevent dead code elimination */
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                  v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                  v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30 +
                  complex_expr;  /* Use the original variable too */
        
        /* Another compiler barrier */
        __asm__ volatile ("" : : : "memory");
    }
    
    return result;
}

int main(void) {
    srand(time(NULL));
    
    /* Use volatile inputs to prevent constant propagation */
    volatile int a = rand() % 100 + 1;
    volatile int b = rand() % 100 + 1;
    volatile int c = rand() % 100 + 1;
    volatile int d = rand() % 100 + 1;
    volatile int iterations = 10;  /* Small enough to run, large enough for loop unrolling */
    
    printf("Starting computation with a=%d, b=%d, c=%d, d=%d, iterations=%d\n",
           a, b, c, d, iterations);
    
    int result = high_pressure_compute(a, b, c, d, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}

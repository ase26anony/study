/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fno-web -fno-gcse -fearly-remat -fno-rename-registers -fno-tree-pre -fdump-rtl-early_remat -da -o remat_test remat_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and interprocedural optimization */
__attribute__((noinline, noipa))
static int high_pressure_compute(volatile int a, volatile int b, 
                                 volatile int c, volatile int d,
                                 volatile int iter_count) {
    volatile int result = 0;
    
    /* Force many pseudo registers with complex live ranges */
    for (volatile int i = 0; i < iter_count; i++) {
        /* Declare many local variables to create register pressure */
        int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
        
        /* Complex expression that will be reused - candidate for rematerialization */
        int common_expr = (a * b) + (c << 2) - (d / 3);
        
        /* First sequence of independent computations */
        v1 = a + b + common_expr;
        v2 = b * c - common_expr;
        v3 = c ^ d ^ common_expr;
        v4 = d + a * common_expr;
        v5 = (a << 3) | (b >> 1) + common_expr;
        
        /* Compiler barrier to prevent reordering */
        __asm__ volatile ("" : : : "memory");
        
        /* More computations using the same common expression */
        v6 = common_expr * 2 - v1;
        v7 = common_expr / 2 + v2;
        v8 = common_expr ^ v3;
        v9 = common_expr & v4;
        v10 = common_expr | v5;
        
        /* Another compiler barrier */
        __asm__ volatile ("" : : : "memory");
        
        /* Create more register pressure with different expressions */
        v11 = v1 * v2 + v3;
        v12 = v2 - v3 * v4;
        v13 = v3 ^ v4 ^ v5;
        v14 = v4 + v5 * v6;
        v15 = (v5 << 2) | (v6 >> 2);
        
        /* Reuse common_expr again in different context */
        v16 = common_expr + v11 - v12;
        v17 = common_expr * v13 / 7;
        v18 = common_expr ^ v14 ^ v15;
        
        /* Compiler barrier */
        __asm__ volatile ("" : : : "memory");
        
        /* More independent computations */
        v19 = v6 + v7 * v8;
        v20 = v7 - v8 / v9;
        v21 = v8 ^ v9 ^ v10;
        v22 = v9 + v10 * v11;
        v23 = (v10 << 1) | (v11 >> 1);
        
        /* Another reuse of common_expr */
        v24 = common_expr - v19 + v20;
        v25 = common_expr & v21 | v22;
        
        /* Final computations */
        v26 = v12 + v13 - v14;
        v27 = v13 * v14 / v15;
        v28 = v14 ^ v15 ^ v16;
        v29 = v15 + v16 * v17;
        v30 = (v16 << 3) | (v17 >> 3);
        
        /* Control flow split based on volatile condition */
        if (a & 1) {
            /* Use all variables in true branch */
            result += v1 + v3 + v5 + v7 + v9 + v11 + v13 + v15 + v17 + v19 +
                     v21 + v23 + v25 + v27 + v29;
            /* More computations in true branch */
            int t1 = common_expr * 3;
            int t2 = common_expr + v2;
            int t3 = common_expr - v4;
            result += t1 + t2 + t3;
        } else {
            /* Use different variables in false branch */
            result += v2 + v4 + v6 + v8 + v10 + v12 + v14 + v16 + v18 + v20 +
                     v22 + v24 + v26 + v28 + v30;
            /* Different computations with common_expr */
            int f1 = common_expr / 4;
            int f2 = common_expr ^ v1;
            int f3 = common_expr | v3;
            result += f1 + f2 + f3;
        }
        
        /* Compiler barrier at loop end */
        __asm__ volatile ("" : : : "memory");
        
        /* Force all variables to appear live */
        result += v1 - v2 + v3 - v4 + v5 - v6 + v7 - v8 + v9 - v10 +
                 v11 - v12 + v13 - v14 + v15 - v16 + v17 - v18 + v19 - v20 +
                 v21 - v22 + v23 - v24 + v25 - v26 + v27 - v28 + v29 - v30;
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
    volatile int iterations = 10; /* Small iteration count to avoid overflow */
    
    printf("Inputs: a=%d, b=%d, c=%d, d=%d, iter=%d\n", 
           a, b, c, d, iterations);
    
    int result = high_pressure_compute(a, b, c, d, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}

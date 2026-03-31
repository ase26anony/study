/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fno-web -fno-gcse -fearly-remat -fno-rename-registers -fno-tree-pre -fdump-rtl-early_remat -fdump-rtl-all -da -o remat_test remat_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and interprocedural optimization */
__attribute__((noinline, noipa))
static int high_pressure_compute(volatile int a, volatile int b, 
                                 volatile int c, volatile int d,
                                 volatile int iterations) {
    volatile int result = 0;
    
    /* Loop to create multiple basic blocks and extend live ranges */
    for (volatile int i = 0; i < iterations; i++) {
        /* Declare many local variables to create high register pressure */
        int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
        
        /* Complex expression that will be reused - candidate for rematerialization */
        int common_expr = (a * b) + (c << 2) - (d & 0xFF);
        
        /* First sequence of independent arithmetic operations */
        v1 = a + b;
        v2 = b * c;
        v3 = c - d;
        v4 = d ^ a;
        v5 = (a << 3) | b;
        v6 = (b >> 2) + c;
        v7 = c * d * a;
        v8 = (d & 0xF0) | (a & 0x0F);
        v9 = v1 * v2 - v3;
        v10 = v4 + v5 * v6;
        
        /* Use the common expression multiple times with different operations */
        v11 = common_expr + v1;
        v12 = common_expr - v2;
        v13 = common_expr * v3;
        v14 = common_expr & v4;
        
        /* Compiler barrier to prevent reordering/coalescing */
        __asm__ volatile ("" : : : "memory");
        
        /* Second sequence - more independent operations */
        v15 = v1 * v6 + v7;
        v16 = v2 ^ v8 | v9;
        v17 = (v3 << 4) + (v4 >> 2);
        v18 = v5 * v10 - v11;
        v19 = v6 + v12 * v13;
        v20 = v7 & v14 | v15;
        
        /* Reuse common_expr again in different contexts */
        v21 = common_expr + v16;
        v22 = common_expr - v17;
        v23 = common_expr * v18;
        v24 = common_expr | v19;
        
        /* Another compiler barrier */
        __asm__ volatile ("" : : : "memory");
        
        /* Third sequence - continue creating many live values */
        v25 = v8 + v9 * v10;
        v26 = v11 ^ v12 & v13;
        v27 = (v14 << 1) + (v15 >> 3);
        v28 = v16 * v17 - v18;
        v29 = v19 + v20 * v21;
        v30 = v22 & v23 | v24;
        
        /* Final reuse of common_expr */
        int final1 = common_expr + v25;
        int final2 = common_expr - v26;
        int final3 = common_expr * v27;
        int final4 = common_expr & v28;
        
        /* Conditional branch to split basic blocks */
        volatile int condition = a + i;
        if (condition & 1) {
            /* True branch - different computations */
            v1 = v1 * 2 + v30;
            v2 = v2 - v29 | v28;
            v3 = (v3 << 2) & 0xFF;
            result += v1 + v2 + v3 + final1;
            
            /* More operations in true branch */
            v4 = v4 * v5 / (v6 + 1);
            v5 = v7 ^ v8 & v9;
            result += v4 + v5 + final2;
        } else {
            /* False branch - alternative computations */
            v10 = v10 * 3 - v27;
            v11 = v11 + v26 ^ v25;
            v12 = (v12 >> 1) | 0x80;
            result += v10 + v11 + v12 + final3;
            
            /* More operations in false branch */
            v13 = v13 * v14 / (v15 + 1);
            v14 = v16 ^ v17 & v18;
            result += v13 + v14 + final4;
        }
        
        /* Mix all results to prevent dead code elimination */
        result += v19 + v20 + v21 + v22 + v23 + v24;
        
        /* Another compiler barrier before loop continues */
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
    volatile int iterations = 10; /* Small loop to avoid excessive runtime */
    
    printf("Inputs: a=%d, b=%d, c=%d, d=%d, iterations=%d\n", 
           a, b, c, d, iterations);
    
    int result = high_pressure_compute(a, b, c, d, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}

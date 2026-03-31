/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fno-web -fno-gcse -fearly-remat -fno-rename-registers -fno-tree-pre -fdump-rtl-early_remat -fdump-rtl-all -da */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and interprocedural optimization */
__attribute__((noinline, noipa))
static int high_pressure_compute(volatile int a, volatile int b, 
                                 volatile int c, volatile int d,
                                 volatile int iterations)
{
    volatile int result = 0;
    
    for (volatile int i = 0; i < iterations; i++) {
        /* Declare many local variables to create register pressure */
        int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
        
        /* Use volatile inputs to create true dependencies */
        int input1 = a + i;
        int input2 = b - i;
        int input3 = c * (i + 1);
        int input4 = d ^ i;
        
        /* Create a complex sub-expression that will be reused */
        /* This is the rematerialization candidate */
        int complex_expr = (input1 * input2) + (input3 << 2) - input4;
        
        /* Force multiple uses of the same complex expression */
        /* Each assignment creates a register copy that could be rematerialized */
        v1 = complex_expr + 1;
        v2 = complex_expr - 1;
        v3 = complex_expr * 2;
        v4 = complex_expr / 2;
        v5 = complex_expr ^ 0xFF;
        
        /* Compiler barrier to prevent reordering/coalescing */
        __asm__ volatile ("" : : : "memory");
        
        /* More independent calculations using the complex expression */
        v6 = complex_expr + v1;
        v7 = complex_expr - v2;
        v8 = complex_expr * v3;
        v9 = complex_expr | v4;
        v10 = complex_expr & v5;
        
        /* Another complex expression reuse */
        v11 = complex_expr + input1;
        v12 = complex_expr + input2;
        v13 = complex_expr + input3;
        v14 = complex_expr + input4;
        
        __asm__ volatile ("" : : : "memory");
        
        /* Additional independent arithmetic to increase pressure */
        v15 = v1 * v2 + v3;
        v16 = v4 - v5 * v6;
        v17 = v7 ^ v8 | v9;
        v18 = v10 << (v11 & 3);
        v19 = v12 >> (v13 % 4);
        v20 = v14 + v15 * v16;
        
        v21 = v17 - v18 + v19;
        v22 = v20 * v21 / (v1 + 1);
        v23 = (v2 << 2) + (v3 >> 1);
        v24 = v4 ^ v5 ^ v6;
        v25 = v7 | v8 | v9;
        
        __asm__ volatile ("" : : : "memory");
        
        /* Control flow to split basic blocks */
        volatile int condition = (i % 3);
        if (condition == 0) {
            /* Use the complex expression again in the true branch */
            v26 = complex_expr + v10;
            v27 = complex_expr - v11;
            v28 = complex_expr * v12;
            v29 = complex_expr | v13;
            v30 = complex_expr & v14;
            
            result += v26 + v27 + v28 + v29 + v30;
        } else if (condition == 1) {
            /* Different computations in another branch */
            v26 = v15 + v16;
            v27 = v17 - v18;
            v28 = v19 * v20;
            v29 = v21 | v22;
            v30 = v23 ^ v24;
            
            result += v26 * v27 - v28 + v29 ^ v30;
        } else {
            /* Yet another branch with more complex_expr reuse */
            v26 = complex_expr + v25;
            v27 = complex_expr - v24;
            v28 = complex_expr * v23;
            v29 = complex_expr | v22;
            v30 = complex_expr & v21;
            
            result += v26 - v27 + v28 - v29 + v30;
        }
        
        /* More computations after the conditional */
        int temp1 = v26 + v27;
        int temp2 = v28 - v29;
        int temp3 = v30 * complex_expr;  /* Reuse complex_expr again */
        int temp4 = temp1 ^ temp2;
        int temp5 = temp3 | temp4;
        
        result += temp5;
        
        __asm__ volatile ("" : : : "memory");
        
        /* Create cross-dependencies between variables */
        v1 = v2 + v3;
        v2 = v4 - v5;
        v3 = v6 * v7;
        v4 = v8 ^ v9;
        v5 = v10 | v11;
        v6 = v12 + v13;
        v7 = v14 - v15;
        v8 = v16 * v17;
        v9 = v18 ^ v19;
        v10 = v20 | v21;
        
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    }
    
    return result;
}

int main(void)
{
    srand(time(NULL));
    
    /* Use volatile to prevent constant propagation */
    volatile int a = rand() % 100 + 1;
    volatile int b = rand() % 100 + 1;
    volatile int c = rand() % 100 + 1;
    volatile int d = rand() % 100 + 1;
    volatile int iterations = 10;  /* Small enough to run, large enough for pressure */
    
    int result = high_pressure_compute(a, b, c, d, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}

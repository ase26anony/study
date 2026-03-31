/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fno-web -fno-gcse -fearly-remat -fno-rename-registers -fno-tree-pre -fdump-rtl-early_remat -da -o remat_test remat_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and interprocedural optimization */
__attribute__((noinline, noipa))
static int high_pressure_compute(volatile int a, volatile int b, 
                                 volatile int c, volatile int d,
                                 volatile int iterations) {
    volatile int result = 0;
    
    /* Force many pseudo registers with complex expressions */
    for (volatile int i = 0; i < iterations; i++) {
        /* Declare many local variables to create register pressure */
        int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
        
        /* Complex expression that will be reused - rematerialization candidate */
        int common_expr = (a * b) + (c << 2) - (d & 0xFF);
        
        /* Independent arithmetic operations creating many live values */
        v1 = a + b + common_expr;
        v2 = b * c - common_expr;
        v3 = (c << 3) | (d >> 1);
        v4 = common_expr * 2 - v1;
        v5 = (a ^ b) + (c & d);
        v6 = v1 + v2 + common_expr;  /* Reuse common_expr */
        v7 = (b << 4) - (c >> 2);
        v8 = common_expr + v3 * 2;   /* Reuse common_expr */
        v9 = (a * d) + (b * c);
        v10 = v4 - v5 + common_expr; /* Reuse common_expr */
        
        /* Compiler barrier to prevent reordering/coalescing */
        __asm__ volatile ("" : : : "memory");
        
        /* More operations to increase pressure */
        v11 = v6 * v7 + common_expr; /* Reuse common_expr */
        v12 = (v8 << 1) | (v9 >> 1);
        v13 = common_expr - v10;     /* Reuse common_expr */
        v14 = (a + b) * (c - d);
        v15 = v11 ^ v12 ^ v13;
        v16 = common_expr * 3 + v14; /* Reuse common_expr */
        v17 = (b * 7) - (c * 3);
        v18 = v15 + v16 + common_expr; /* Reuse common_expr */
        v19 = (d << 2) + (a >> 1);
        v20 = common_expr & 0xFFFF;  /* Reuse common_expr */
        
        /* Another compiler barrier */
        __asm__ volatile ("" : : : "memory");
        
        /* Control flow to split basic blocks */
        if (a & 0x1) {  /* Volatile condition ensures both paths possible */
            /* Additional computations in true path */
            v21 = v18 * 2 - common_expr; /* Reuse common_expr */
            v22 = (v19 + v20) * 3;
            v23 = common_expr | v21;     /* Reuse common_expr */
            v24 = v22 - (a * b);
            v25 = (c * d) + common_expr; /* Reuse common_expr */
            v26 = v23 ^ v24 ^ v25;
            v27 = common_expr << 1;      /* Reuse common_expr */
            v28 = v26 + v27 * 2;
            v29 = (b + d) * common_expr; /* Reuse common_expr */
            v30 = v28 - v29;
            
            result += v30;
        } else {
            /* Different computations in false path */
            v21 = common_expr + v18;     /* Reuse common_expr */
            v22 = v19 * v20;
            v23 = common_expr - v21;     /* Reuse common_expr */
            v24 = (a << 3) + (b << 2);
            v25 = v22 * common_expr;     /* Reuse common_expr */
            v26 = v23 | v24 | v25;
            v27 = common_expr >> 1;      /* Reuse common_expr */
            v28 = v26 + v27;
            v29 = (c * 5) - common_expr; /* Reuse common_expr */
            v30 = v28 * v29;
            
            result -= v30;
        }
        
        /* Final barrier before loop continues */
        __asm__ volatile ("" : : : "memory");
        
        /* Force use of all variables to prevent dead code elimination */
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        result += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
        result += v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
    }
    
    return result;
}

int main(void) {
    srand(time(NULL));
    
    /* Use volatile to prevent constant propagation */
    volatile int a = rand() % 100 + 1;
    volatile int b = rand() % 100 + 1;
    volatile int c = rand() % 100 + 1;
    volatile int d = rand() % 100 + 1;
    volatile int iterations = 10; /* Small loop to avoid overflow */
    
    printf("Inputs: a=%d, b=%d, c=%d, d=%d, iterations=%d\n", 
           a, b, c, d, iterations);
    
    int result = high_pressure_compute(a, b, c, d, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}

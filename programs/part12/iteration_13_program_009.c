/* Compile with: gcc -O2 -fearly-remat -fno-rename-registers -fno-tree-pre -fno-schedule-insns -fno-web -fno-gcse -fdump-rtl-early_remat -fdump-rtl-all -da -o remat_test remat_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_compute(volatile int a, volatile int b, 
                                          volatile int c, volatile int d,
                                          volatile int iterations) {
    volatile int result = 0;
    
    /* Loop with volatile iteration count to prevent unrolling */
    for (volatile int i = 0; i < iterations; i++) {
        /* Declare many local variables to create register pressure */
        int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
        
        /* Complex expression that will be reused multiple times */
        /* This creates a rematerialization candidate */
        int common_expr = (a * b) + (c << 2) - (d & 0xFF);
        
        /* First block of independent arithmetic operations */
        v1 = a + b + common_expr;
        v2 = b * c - common_expr;
        v3 = (a << 3) | (b & 0xF);
        v4 = (c * d) + common_expr;
        v5 = (a ^ b) + (c & d);
        v6 = common_expr * 2 - v1;
        v7 = (b << 2) + common_expr;
        v8 = (c * 3) | (d & 0x7F);
        v9 = common_expr + (a >> 1);
        v10 = (b * d) - common_expr;
        
        /* Compiler barrier to prevent reordering/coalescing */
        __asm__ volatile ("" : : : "memory");
        
        /* Second block with more operations */
        v11 = v1 + v2 + common_expr;
        v12 = v3 * v4 - common_expr;
        v13 = (v5 << 1) | (v6 & 0xF);
        v14 = (v7 * v8) + common_expr;
        v15 = (v9 ^ v10) + (v11 & v12);
        v16 = common_expr * 3 - v13;
        v17 = (v14 << 2) + common_expr;
        v18 = (v15 * 5) | (v16 & 0x3F);
        v19 = common_expr + (v17 >> 2);
        v20 = (v18 * v19) - common_expr;
        
        /* Another compiler barrier */
        __asm__ volatile ("" : : : "memory");
        
        /* Third block with even more operations */
        v21 = v11 + v12 + common_expr;
        v22 = v13 * v14 - common_expr;
        v23 = (v15 << 2) | (v16 & 0x1F);
        v24 = (v17 * v18) + common_expr;
        v25 = (v19 ^ v20) + (v21 & v22);
        v26 = common_expr * 4 - v23;
        v27 = (v24 << 1) + common_expr;
        v28 = (v25 * 7) | (v26 & 0x7F);
        v29 = common_expr + (v27 >> 3);
        v30 = (v28 * v29) - common_expr;
        
        /* Control flow split based on volatile condition */
        volatile int condition = a & 1;
        if (condition) {
            /* Use all variables in true branch */
            result += v1 + v3 + v5 + v7 + v9 + v11 + v13 + v15 + v17 + v19 +
                     v21 + v23 + v25 + v27 + v29 + common_expr;
        } else {
            /* Use different variables in false branch */
            result += v2 + v4 + v6 + v8 + v10 + v12 + v14 + v16 + v18 + v20 +
                     v22 + v24 + v26 + v28 + v30 - common_expr;
        }
        
        /* Modify inputs slightly to prevent complete optimization */
        a = (a + 1) & 0x7F;
        b = (b + 2) & 0x7F;
        c = (c + 3) & 0x7F;
        d = (d + 4) & 0x7F;
        
        /* Final compiler barrier in loop */
        __asm__ volatile ("" : : : "memory");
    }
    
    return result;
}

int main(void) {
    srand(time(NULL));
    
    /* Initialize volatile inputs to prevent constant propagation */
    volatile int a = rand() % 100;
    volatile int b = rand() % 100;
    volatile int c = rand() % 100;
    volatile int d = rand() % 100;
    volatile int iterations = 10 + (rand() % 5);  /* Small loop to keep runtime reasonable */
    
    printf("Starting computation with a=%d, b=%d, c=%d, d=%d, iterations=%d\n",
           a, b, c, d, iterations);
    
    volatile int result = high_pressure_compute(a, b, c, d, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}

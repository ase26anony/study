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
    
    for (volatile int i = 0; i < iterations; i++) {
        /* Declare many local variables to create register pressure */
        int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
        
        /* Complex expression that will be reused - candidate for rematerialization */
        int complex_expr = (a * b) + (c << 2) - d;
        
        /* First block of independent arithmetic operations */
        v1 = a + b;
        v2 = b * c;
        v3 = c - d;
        v4 = d << 1;
        v5 = a ^ b;
        v6 = b | c;
        v7 = c & d;
        v8 = a * 3;
        v9 = b + 5;
        v10 = c - 7;
        
        /* Compiler barrier to prevent reordering */
        __asm__ volatile ("" : : : "memory");
        
        /* Reuse the complex expression multiple times */
        v11 = complex_expr + v1;
        v12 = complex_expr - v2;
        v13 = complex_expr * v3;
        v14 = complex_expr & v4;
        v15 = complex_expr | v5;
        
        /* More independent operations */
        v16 = v1 * v2;
        v17 = v3 + v4;
        v18 = v5 ^ v6;
        v19 = v7 << 2;
        v20 = v8 >> 1;
        
        /* Another compiler barrier */
        __asm__ volatile ("" : : : "memory");
        
        /* Control flow split based on volatile condition */
        volatile int condition = a & 1;
        if (condition) {
            /* Branch 1: More computations reusing complex_expr */
            v21 = complex_expr + v16;
            v22 = complex_expr - v17;
            v23 = complex_expr * v18;
            v24 = v19 + complex_expr;
            v25 = v20 ^ complex_expr;
            
            v26 = v21 * v22;
            v27 = v23 + v24;
            v28 = v25 << 1;
            v29 = v26 >> 2;
            v30 = v27 & v28;
            
            result += v29 + v30;
        } else {
            /* Branch 2: Different computations but still reusing complex_expr */
            v21 = complex_expr << 1;
            v22 = complex_expr >> 1;
            v23 = complex_expr + v16;
            v24 = complex_expr - v17;
            v25 = complex_expr * v18;
            
            v26 = v21 + v22;
            v27 = v23 * v24;
            v28 = v25 ^ v19;
            v29 = v26 & v27;
            v30 = v28 | v20;
            
            result -= v29 - v30;
        }
        
        /* Final computations mixing all values */
        int final1 = v11 + v12 + v13 + v14 + v15;
        int final2 = v16 + v17 + v18 + v19 + v20;
        int final3 = v21 + v22 + v23 + v24 + v25;
        int final4 = v26 + v27 + v28 + v29 + v30;
        
        /* One more compiler barrier */
        __asm__ volatile ("" : : : "memory");
        
        /* Accumulate results */
        result += final1 + final2 + final3 + final4;
        
        /* Modify inputs slightly for next iteration */
        a ^= i;
        b += 1;
        c -= 1;
        d = (d << 1) | 1;
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
    volatile int iterations = 10; /* Small enough to run, large enough for loop unrolling */
    
    printf("Starting computation with a=%d, b=%d, c=%d, d=%d, iterations=%d\n",
           a, b, c, d, iterations);
    
    int result = high_pressure_compute(a, b, c, d, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}

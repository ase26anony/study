/* early_remat_trigger.c
 * Program designed to trigger GCC's early rematerialization pass
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and interprocedural optimization */
__attribute__((noinline, noipa))
static int high_pressure_compute(volatile int a, volatile int b, 
                                 volatile int c, volatile int d,
                                 volatile int iter_count) {
    volatile int result = 0;
    
    /* Loop with volatile iteration count to prevent unrolling */
    for (volatile int i = 0; i < iter_count; i++) {
        /* Declare many local variables to create register pressure */
        int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
        int v31, v32, v33, v34, v35, v36, v37, v38, v39, v40;
        
        /* Complex expression that will be reused - rematerialization candidate */
        int complex_expr = (a * b) + (c << 2) - (d * 3);
        
        /* First set of independent arithmetic operations */
        v1 = a + b + i;
        v2 = b - c * i;
        v3 = c + d + v1;
        v4 = d * a - v2;
        v5 = (a << 3) | (b & 0xFF);
        v6 = (b >> 2) ^ c;
        v7 = c * 7 + d;
        v8 = d / 2 + a;
        v9 = (a & b) | (c ^ d);
        v10 = (b * c) - (d << 1);
        
        /* Reuse the complex expression - creates copy pattern */
        v11 = complex_expr + v1;
        v12 = complex_expr - v2;
        
        /* Compiler barrier - prevents reordering/coalescing */
        __asm__ volatile ("" : : : "memory");
        
        /* More independent operations */
        v13 = v1 * v3 + v5;
        v14 = v2 / 4 + v6;
        v15 = v3 ^ v7 + v9;
        v16 = v4 | v8 * v10;
        v17 = v5 - v9 << 1;
        v18 = v6 + v10 * 3;
        v19 = v7 & v1 | v3;
        v20 = v8 ^ v2 + v4;
        
        /* Conditional branch to split basic blocks */
        volatile int condition = a & 1;
        if (condition) {
            /* Another reuse of complex expression in different block */
            v21 = complex_expr * 2;
            v22 = complex_expr / 2;
            
            /* More operations in this branch */
            v23 = v11 + v13 * v15;
            v24 = v12 - v14 / v16;
            v25 = v17 | v19 & v21;
            v26 = v18 ^ v20 * v22;
            v27 = v23 << (v24 & 3);
            v28 = v25 >> (v26 % 4);
            
            /* Compiler barrier */
            __asm__ volatile ("" : : : "memory");
            
            /* Even more operations */
            v29 = v27 + v28 * 3;
            v30 = v29 - complex_expr;  /* Another reuse */
            v31 = v30 & 0xFFFF;
            v32 = v31 | 0x1000;
            
            result += v32;
        } else {
            /* Alternative path with different computations */
            v33 = complex_expr + i;  /* Reuse in else branch */
            v34 = v11 * v12 - v13;
            v35 = v14 + v15 ^ v16;
            v36 = v17 & v18 | v19;
            v37 = v20 - v33 * 2;
            v38 = v34 / (v35 + 1);
            v39 = v36 << (v37 & 7);
            v40 = v38 >> (v39 % 3);
            
            /* Compiler barrier */
            __asm__ volatile ("" : : : "memory");
            
            /* More computations using complex_expr */
            v21 = complex_expr + v40;  /* Another reuse */
            v22 = v21 * v33 - v34;
            v23 = v35 + v36 & v37;
            v24 = v38 | v39 ^ v40;
            
            result -= v24;
        }
        
        /* Final mixing of values across both paths */
        int final_mix = (v21 + v22) * (v23 - v24);
        if (condition) {
            final_mix += v31 * v32;
        } else {
            final_mix -= v39 * v40;
        }
        
        result += final_mix & 0xFF;
        
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
    volatile int iterations = 10;  /* Small enough to run, large enough for pressure */
    
    printf("Starting high-pressure computation...\n");
    printf("Inputs: a=%d, b=%d, c=%d, d=%d, iterations=%d\n", 
           a, b, c, d, iterations);
    
    int result = high_pressure_compute(a, b, c, d, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}

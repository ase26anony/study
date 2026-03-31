/* early_remat_trigger.c
 * Designed to trigger GCC's early rematerialization pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fno-web -fno-gcse early_remat_trigger.c -o early_remat_trigger
 * Or for more aggressive remat: gcc -O3 -fearly-remat -fno-rename-registers -fno-tree-pre early_remat_trigger.c -o early_remat_trigger
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
        int common_expr = (a * b) + (c << 2) - (d / 3);
        
        /* Force true data dependencies with volatile inputs */
        v1 = a + b;
        v2 = b * c;
        v3 = c - d;
        v4 = d + a;
        v5 = a * c;
        v6 = b - d;
        
        /* Compiler barrier to prevent reordering/coalescing */
        __asm__ volatile ("" : : : "memory");
        
        /* Use the common expression multiple times with different operations */
        v7 = common_expr + v1;
        v8 = common_expr - v2;
        v9 = common_expr * v3;
        v10 = common_expr & v4;
        
        /* More independent calculations */
        v11 = (v1 << 3) | (v2 >> 1);
        v12 = (v3 * v4) + (v5 / 2);
        v13 = (v6 ^ v1) & (v2 | v3);
        v14 = (v4 - v5) * (v6 + v1);
        v15 = (v2 << 2) + (v3 >> 1);
        
        __asm__ volatile ("" : : : "memory");
        
        /* Conditional branch to split basic blocks */
        volatile int condition = a > (b + i);
        if (condition) {
            /* Different computations in the taken branch */
            v16 = common_expr + (v7 * 2);
            v17 = common_expr - (v8 / 2);
            v18 = (v9 << 1) | common_expr;
            v19 = (v10 ^ common_expr) & 0xFF;
            v20 = common_expr * common_expr;
            
            v21 = v16 + v17;
            v22 = v18 - v19;
            v23 = v20 * v21;
            v24 = v22 & v23;
            v25 = (v24 << 3) >> 1;
        } else {
            /* Alternative computations in the not-taken branch */
            v16 = common_expr * 3;
            v17 = common_expr / 4;
            v18 = common_expr | 0x7F;
            v19 = common_expr ^ 0xAA;
            v20 = common_expr & 0x55;
            
            v21 = v16 - v17;
            v22 = v18 + v19;
            v23 = v20 * v21;
            v24 = v22 ^ v23;
            v25 = (v24 >> 2) << 1;
        }
        
        __asm__ volatile ("" : : : "memory");
        
        /* More computations after the conditional */
        v26 = v21 + v22;
        v27 = v23 - v24;
        v28 = v25 * v26;
        v29 = v27 & v28;
        v30 = (v29 << 1) | (v26 >> 1);
        
        /* Reuse common_expr again */
        v26 += common_expr;
        v27 -= common_expr;
        v28 *= common_expr;
        v29 &= common_expr;
        v30 ^= common_expr;
        
        __asm__ volatile ("" : : : "memory");
        
        /* Final aggregation */
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                  v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                  v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
        
        /* Modify inputs slightly to prevent loop invariant removal */
        a += 1;
        b -= 1;
        c ^= 0x01;
        d |= 0x01;
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
    volatile int iterations = 10; /* Small enough to run, large enough for pressure */
    
    printf("Starting computation with a=%d, b=%d, c=%d, d=%d, iterations=%d\n",
           a, b, c, d, iterations);
    
    int result = high_pressure_compute(a, b, c, d, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}

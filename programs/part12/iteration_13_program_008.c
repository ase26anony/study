/* early_remat_trigger.c
 * Designed to trigger GCC's early rematerialization pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fno-web -fno-gcse early_remat_trigger.c -o early_remat_trigger
 * For more aggressive remat: gcc -O3 -fearly-remat -fno-rename-registers -fno-tree-pre early_remat_trigger.c -o early_remat_trigger
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
        /* Declare many local variables to create register pressure */
        int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
        int v31, v32, v33, v34, v35, v36, v37, v38, v39, v40;
        
        /* Complex expression that will be reused - candidate for rematerialization */
        int complex_expr = (a * b) + (c << 2) - (d & 0xFF);
        
        /* First block of independent calculations */
        v1 = a + b;
        v2 = b * c;
        v3 = c - d;
        v4 = d ^ a;
        v5 = (a << 3) | (b >> 1);
        v6 = complex_expr;  /* First use of complex expression */
        
        /* Compiler barrier to prevent reordering */
        __asm__ volatile ("" : : : "memory");
        
        v7 = v1 * v2;
        v8 = v3 + v4;
        v9 = v5 ^ v6;
        v10 = complex_expr + v1;  /* Second use of complex expression */
        v11 = (v7 << 1) + (v8 >> 2);
        v12 = v9 * 0x1234;
        
        /* More calculations to increase register pressure */
        v13 = a * v1 + b * v2;
        v14 = c * v3 - d * v4;
        v15 = (v5 & 0xF0F0) | (v6 & 0x0F0F);
        v16 = complex_expr * 2;  /* Third use of complex expression */
        v17 = v7 + v8 + v9 + v10;
        v18 = v11 ^ v12 ^ v13;
        
        __asm__ volatile ("" : : : "memory");
        
        /* Conditional branch to split basic block */
        volatile int condition = a & 1;
        if (condition) {
            /* True branch calculations */
            v19 = v14 * v15 + v16;
            v20 = v17 - v18;
            v21 = complex_expr >> 2;  /* Fourth use in true branch */
            v22 = (v19 & v20) | (v21 ^ 0xFFFF);
            v23 = v22 * 3 + 7;
            v24 = (v23 << 4) - (complex_expr & 0xFF);  /* Fifth use */
            
            __asm__ volatile ("" : : : "memory");
            
            v25 = v24 * a + b;
            v26 = v25 * c - d;
            v27 = complex_expr + v24;  /* Sixth use */
            v28 = v26 ^ v27;
            v29 = (v28 * 0xABCD) & 0x7FFF;
            v30 = complex_expr | v29;  /* Seventh use */
            
            result += v30;
        } else {
            /* False branch calculations */
            v31 = v14 + v15 - v16;
            v32 = v17 * v18;
            v33 = complex_expr << 1;  /* Fourth use in false branch */
            v34 = (v31 | v32) & (v33 ^ 0xAAAA);
            v35 = v34 / 2 + 11;
            v36 = (v35 >> 3) + (complex_expr % 256);  /* Fifth use */
            
            __asm__ volatile ("" : : : "memory");
            
            v37 = v36 * b + a;
            v38 = v37 * d - c;
            v39 = complex_expr - v36;  /* Sixth use */
            v40 = v38 & v39;
            int v41 = (v40 * 0xDCBA) | 0x8000;
            int v42 = complex_expr ^ v41;  /* Seventh use */
            
            result += v42;
        }
        
        /* More calculations after the conditional */
        int v43 = v1 + v2 + v3 + v4 + v5;
        int v44 = v6 * v7 * v8 * v9 * v10;
        int v45 = complex_expr + v43 - v44;  /* Eighth use */
        int v46 = (v11 & v12) | (v13 ^ v14);
        int v47 = v15 * v16 + v17 * v18;
        int v48 = complex_expr * v46 / (v47 + 1);  /* Ninth use */
        
        __asm__ volatile ("" : : : "memory");
        
        /* Force all variables to be used to prevent dead code elimination */
        result += v45 + v48;
        
        /* Additional independent expressions to increase pressure */
        int v49 = (a * 3) + (b * 5) - (c * 7) + (d * 11);
        int v50 = (a ^ b ^ c ^ d) * 0x12345678;
        int v51 = complex_expr + v49 - v50;  /* Tenth use */
        int v52 = (v49 << 4) | (v50 >> 4);
        int v53 = complex_expr * v52;  /* Eleventh use */
        int v54 = (v51 & 0xFF00) + (v53 & 0x00FF);
        int v55 = complex_expr | v54;  /* Twelfth use */
        
        result += v55;
        
        /* Modify inputs slightly for next iteration */
        a = (a + 1) & 0x7F;
        b = (b * 3) & 0xFF;
        c = (c - 1) & 0xFF;
        d = (d ^ i) & 0xFF;
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
    volatile int iterations = 10;  /* Small number to avoid long runtime */
    
    printf("Starting computation with a=%d, b=%d, c=%d, d=%d, iterations=%d\n",
           a, b, c, d, iterations);
    
    int result = high_pressure_compute(a, b, c, d, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}

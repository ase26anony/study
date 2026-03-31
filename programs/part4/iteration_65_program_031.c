/* test_mcf_coverage.c
 * Designed to trigger debug output in GCC's min-cost flow solver
 * when compiled with a GCC configured with --enable-checking
 */

#include <stdio.h>
#include <stdlib.h>

/* High register pressure function with complex control flow */
__attribute__((noinline, optimize("O3")))
void high_pressure_function(void) {
    /* 18 volatile variables to force register pressure */
    volatile int v0 = 1, v1 = 2, v2 = 3, v3 = 4, v4 = 5, v5 = 6;
    volatile int v6 = 7, v7 = 8, v8 = 9, v9 = 10, v10 = 11, v11 = 12;
    volatile int v12 = 13, v13 = 14, v14 = 15, v15 = 16, v16 = 17, v17 = 18;
    
    /* Additional non-volatile temporaries */
    int t0, t1, t2, t3, t4;
    
    /* Memory barrier to force liveness across it */
    asm volatile("" : : : "memory");
    
    /* Complex nested loops to create many live ranges */
    for (int i = 0; i < 4; i++) {  /* Constant bound encourages unrolling */
        /* Switch with multiple cases for complex CFG */
        switch (i % 5) {
            case 0:
                /* Chain of dependent operations */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 + v6;
                v7 = v5 * v8;
                /* Memory barrier */
                asm volatile("" : : : "memory");
                break;
                
            case 1:
                /* Different dependency chain */
                t2 = v9 - v10;
                t3 = v11 + v12;
                v13 = t2 * t3;
                v14 = v13 / v15;
                v16 = v14 + v17;
                /* Force all variables to be used */
                v0 = v16 + v1;
                asm volatile("" : : : "memory");
                break;
                
            case 2:
                /* Cross-case dependencies */
                v1 = v2 + v3;
                v4 = v1 * v5;
                v6 = v4 - v7;
                v8 = v6 + v9;
                v10 = v8 * v11;
                asm volatile("" : : : "memory");
                break;
                
            case 3:
                /* More complex operations */
                t4 = v12 * v13;
                v14 = t4 + v15;
                v16 = v14 - v17;
                v0 = v16 / v1;
                v2 = v0 * v3;
                asm volatile("" : : : "memory");
                break;
                
            case 4:
                /* Use all remaining variables */
                v4 = v5 + v6;
                v7 = v8 - v9;
                v10 = v11 * v12;
                v13 = v14 + v15;
                v16 = v17 * v0;
                v1 = v2 + v3;
                asm volatile("" : : : "memory");
                break;
        }
        
        /* Inner loop with more operations */
        for (int j = 0; j < 3; j++) {
            /* Mix variables from different sets */
            int temp = v0 + v1 + v2;
            v3 = temp - v4;
            v5 = v3 * v6;
            v7 = v5 + v8;
            v9 = v7 - v10;
            
            /* Conditional to create more CFG edges */
            if (j % 2 == 0) {
                v11 = v12 + v13;
                v14 = v11 * v15;
            } else {
                v16 = v17 + v0;
                v1 = v16 - v2;
            }
            
            /* Another memory barrier */
            asm volatile("" : : : "memory");
        }
        
        /* Additional operations between loop iterations */
        v2 = v3 + v4;
        v5 = v2 * v6;
        v7 = v5 - v8;
        v9 = v7 + v10;
    }
    
    /* Final complex expression using all variables */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17;
    
    /* Prevent dead code elimination */
    asm volatile("" : "+r"(result) : : "memory");
}

/* Secondary function with different pressure pattern */
__attribute__((noinline, optimize("O3")))
void secondary_pressure_function(int iterations) {
    volatile int a0 = 1, a1 = 2, a2 = 3, a3 = 4, a4 = 5;
    volatile int b0 = 6, b1 = 7, b2 = 8, b3 = 9, b4 = 10;
    volatile int c0 = 11, c1 = 12, c2 = 13, c3 = 14, c4 = 15;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex dependency web */
        int t0 = a0 + a1;
        int t1 = a2 * a3;
        int t2 = b0 - b1;
        int t3 = b2 + b3;
        int t4 = c0 * c1;
        
        a4 = t0 + t1;
        b4 = t2 - t3;
        c4 = t4 / a0;
        
        /* Cross-assignments */
        a0 = b4 + c4;
        b0 = a4 - c4;
        c0 = a0 * b0;
        
        /* Conditional with live variables */
        if (i % 3 == 0) {
            a1 = a2 + a3;
            b1 = b2 * b3;
            c1 = c2 - c3;
        } else if (i % 3 == 1) {
            a2 = a3 + a4;
            b2 = b3 * b4;
            c2 = c3 - c4;
        } else {
            a3 = a4 + a0;
            b3 = b4 * b0;
            c3 = c4 - c0;
        }
        
        asm volatile("" : : : "memory");
    }
}

/* Main function to ensure everything gets compiled */
int main(void) {
    /* Call high-pressure functions */
    high_pressure_function();
    secondary_pressure_function(5);
    
    return 0;
}

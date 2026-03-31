/* test_mcf_debug.c
 * 
 * Test program to trigger debug dumping in GCC's min-cost flow solver
 * for register allocation. Compile with a GCC built with --enable-checking
 * (defining MCF_DEBUG) using: gcc-debug -O3 -funroll-loops -c test_mcf_debug.c
 */

#include <stdio.h>
#include <stdlib.h>

/* High-pressure function with complex control flow and many live variables */
__attribute__((noinline, optimize("O3")))
void high_pressure_function(int iterations) {
    /* Declare many volatile variables to prevent optimization and increase pressure */
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    volatile int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    
    /* Additional non-volatile temporaries */
    int t0, t1, t2, t3, t4;
    
    /* Initialize variables with different values to create data dependencies */
    v0 = 1; v1 = 2; v2 = 3; v3 = 4; v4 = 5;
    v5 = 6; v6 = 7; v7 = 8; v8 = 9; v9 = 10;
    v10 = 11; v11 = 12; v12 = 13; v13 = 14; v14 = 15;
    v15 = 16; v16 = 17; v17 = 18; v18 = 19; v19 = 20;
    
    /* Memory barrier to force variables to be live across it */
    asm volatile("" : : : "memory");
    
    /* Complex nested loops to extend live ranges */
    for (int i = 0; i < iterations; i++) {
        /* Switch statement with multiple cases creating control flow edges */
        switch (i % 5) {
            case 0:
                /* Chain of arithmetic operations creating data dependencies */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 + v6;
                v7 = v5 * v8;
                
                /* Memory barrier to extend live ranges */
                asm volatile("" : : : "memory");
                
                t2 = v9 + v10;
                v11 = t2 - v12;
                v13 = v11 * v14;
                v15 = v13 + v16;
                break;
                
            case 1:
                /* Different operation pattern */
                t0 = v1 - v2;
                t1 = v3 * v4;
                v5 = t0 + t1;
                v6 = v5 - v7;
                v8 = v6 * v9;
                
                asm volatile("" : : : "memory");
                
                t2 = v10 + v11;
                v12 = t2 * v13;
                v14 = v12 - v15;
                v16 = v14 + v17;
                break;
                
            case 2:
                /* More complex data flow */
                t0 = v2 * v3;
                t1 = v4 + v5;
                v6 = t0 - t1;
                v7 = v6 * v8;
                v9 = v7 + v10;
                
                asm volatile("" : : : "memory");
                
                t2 = v11 - v12;
                t3 = v13 * v14;
                v15 = t2 + t3;
                v16 = v15 - v17;
                v18 = v16 * v19;
                break;
                
            case 3:
                /* Cross-case variable usage */
                t0 = v3 + v4;
                v5 = t0 * v6;
                v7 = v5 - v8;
                v9 = v7 + v10;
                
                asm volatile("" : : : "memory");
                
                /* Use variables from different cases */
                t1 = v0 + v11;
                v12 = t1 * v13;
                v14 = v12 - v15;
                v16 = v14 + v17;
                break;
                
            case 4:
                /* All variables used together */
                t0 = v0 + v1 + v2;
                t1 = v3 * v4 * v5;
                v6 = t0 - t1;
                v7 = v6 + v8 + v9;
                
                asm volatile("" : : : "memory");
                
                t2 = v10 * v11;
                t3 = v12 + v13;
                t4 = v14 - v15;
                v16 = t2 + t3;
                v17 = t4 * v18;
                v19 = v16 - v17;
                break;
        }
        
        /* Additional loop with constant bound to encourage unrolling */
        for (int j = 0; j < 4; j++) {
            /* Mix variables from different scopes */
            t0 = v0 + j;
            v1 = v1 * t0;
            v2 = v2 - v1;
            v3 = v3 + v2;
            
            /* Conditional inside inner loop */
            if (j % 2 == 0) {
                v4 = v4 * v5;
                v6 = v6 + v7;
            } else {
                v8 = v8 - v9;
                v10 = v10 * v11;
            }
            
            /* More arithmetic chains */
            t1 = v12 + v13;
            v14 = t1 * v15;
            v16 = v14 - v17;
            v18 = v16 + v19;
        }
        
        /* Final mixing of all variables */
        t0 = v0 + v1 + v2 + v3 + v4;
        t1 = v5 * v6 * v7 * v8 * v9;
        v10 = t0 - t1;
        
        t2 = v11 + v12 + v13 + v14 + v15;
        t3 = v16 * v17 * v18 * v19;
        v0 = t2 - t3;
        
        /* Force another memory barrier */
        asm volatile("" : : : "memory");
    }
    
    /* Use all variables in final computation to prevent dead code elimination */
    t0 = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
         v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
    
    /* Volatile store to ensure all computations are kept */
    volatile int result __attribute__((unused)) = t0;
}

/* Secondary function with different control flow pattern */
__attribute__((noinline, optimize("O3")))
void secondary_pressure_function(void) {
    volatile int a0, a1, a2, a3, a4, a5, a6, a7, a8, a9;
    volatile int b0, b1, b2, b3, b4, b5, b6, b7, b8, b9;
    
    /* Initialize */
    a0 = 1; a1 = 2; a2 = 3; a3 = 4; a4 = 5;
    a5 = 6; a6 = 7; a7 = 8; a8 = 9; a9 = 10;
    b0 = 11; b1 = 12; b2 = 13; b3 = 14; b4 = 15;
    b5 = 16; b6 = 17; b7 = 18; b8 = 19; b9 = 20;
    
    /* Deeply nested conditionals */
    if (a0 > 0) {
        if (a1 > 1) {
            if (a2 > 2) {
                a3 = a0 + a1 + a2;
                b0 = a3 * b0;
                
                asm volatile("" : : : "memory");
                
                for (int i = 0; i < 3; i++) {
                    a4 = a4 + b1;
                    b2 = b2 * a5;
                    
                    if (i == 1) {
                        a6 = a6 - b3;
                        b4 = b4 + a7;
                    } else {
                        a8 = a8 * b5;
                        b6 = b6 - a9;
                    }
                }
            }
        }
    }
    
    /* Use results */
    volatile int sum __attribute__((unused)) = a0 + b0 + a4 + b2 + a6 + b4 + a8 + b6;
}

int main(void) {
    /* Call high-pressure functions to ensure they're compiled */
    high_pressure_function(8);
    secondary_pressure_function();
    
    return 0;
}

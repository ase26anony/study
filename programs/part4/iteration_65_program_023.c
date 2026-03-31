/* test_mcf_debug.c - Min-Cost Flow Debug Coverage Test */
/* Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_debug.c -o test.o */
/* Requires GCC built with --enable-checking (defines MCF_DEBUG) */

#include <stdio.h>
#include <stdlib.h>

/* High-pressure function with complex control flow */
__attribute__((noinline, optimize("O3")))
void high_pressure_function(int iterations) {
    /* Declare many volatile variables to prevent optimization */
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    volatile int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    
    /* Additional non-volatile temporaries */
    int t0, t1, t2, t3, t4;
    
    /* Initialize variables with different values */
    v0 = 1; v1 = 2; v2 = 3; v3 = 4; v4 = 5;
    v5 = 6; v6 = 7; v7 = 8; v8 = 9; v9 = 10;
    v10 = 11; v11 = 12; v12 = 13; v13 = 14; v14 = 15;
    v15 = 16; v16 = 17; v17 = 18; v18 = 19; v19 = 20;
    
    /* Complex nested loops to create overlapping live ranges */
    for (int i = 0; i < iterations; i++) {
        /* Memory barrier to force liveness across it */
        asm volatile("" : : : "memory");
        
        /* Switch with multiple cases creating complex CFG */
        switch (i % 7) {
            case 0:
                /* Chain of dependent operations */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 + v6;
                v7 = v5 * v8;
                /* Keep many variables live */
                v9 = v10 + v11 + v12;
                v13 = v14 - v15;
                break;
                
            case 1:
                /* Different operation pattern */
                t2 = v1 * v3;
                t3 = v5 + v7;
                v0 = t2 - t3;
                v2 = v0 * v4;
                v6 = v2 + v8;
                v10 = v11 - v12;
                v14 = v15 * v16;
                break;
                
            case 2:
                /* More complex data flow */
                t4 = v3 + v6 + v9;
                v1 = t4 * v2;
                v4 = v1 - v7;
                v8 = v4 + v10;
                v12 = v8 * v13;
                v16 = v12 - v17;
                v18 = v16 + v19;
                break;
                
            case 3:
                /* Use most variables simultaneously */
                v0 = v1 + v2 + v3 + v4;
                v5 = v6 * v7 * v8;
                v9 = v10 - v11 - v12;
                v13 = v14 + v15 + v16;
                v17 = v18 * v19;
                break;
                
            case 4:
                /* Cross-case dependencies */
                t0 = v0 + v5 + v10 + v15;
                v1 = t0 * 2;
                v6 = v1 - v11;
                v11 = v6 * v16;
                v16 = v11 + v2 + v7 + v12;
                break;
                
            case 5:
                /* Another pattern */
                v3 = v8 * v13 * v18;
                v4 = v9 + v14 + v19;
                v7 = v2 - v12 - v17;
                v10 = v5 * v15;
                break;
                
            case 6:
                /* All variables used */
                t1 = v0 + v1 + v2;
                t2 = v3 + v4 + v5;
                t3 = v6 + v7 + v8;
                t4 = v9 + v10 + v11;
                v12 = t1 * t2;
                v13 = t3 - t4;
                v14 = v12 + v13;
                v15 = v14 * v16;
                v17 = v15 - v18;
                v19 = v17 + v0; /* Circular dependency */
                break;
        }
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
        
        /* Inner loop with small constant bound (encourages unrolling) */
        for (int j = 0; j < 4; j++) {
            /* Mix variables in inner loop */
            v0 = v0 + v19;
            v1 = v1 * v18;
            v2 = v2 - v17;
            v3 = v3 + v16;
            v4 = v4 * v15;
            v5 = v5 - v14;
            v6 = v6 + v13;
            v7 = v7 * v12;
            v8 = v8 - v11;
            v9 = v9 + v10;
            
            /* Swap some values */
            t0 = v0; v0 = v19; v19 = t0;
            t1 = v1; v1 = v18; v18 = t1;
        }
        
        /* Conditional block extending live ranges */
        if (i % 3 == 0) {
            v10 = v0 + v1 + v2 + v3 + v4;
            v11 = v5 * v6 * v7 * v8 * v9;
        } else if (i % 3 == 1) {
            v12 = v10 - v11 - v13 - v14;
            v13 = v15 + v16 + v17 + v18;
        } else {
            v14 = v19 * v0 * v1 * v2;
            v15 = v3 + v4 + v5 + v6;
        }
        
        /* Force all variables to be used in final computation */
        v16 = v7 + v8 + v9 + v10 + v11;
        v17 = v12 * v13 * v14 * v15;
        v18 = v16 - v17;
        v19 = v18 + i; /* Use loop index */
    }
    
    /* Final use to prevent dead code elimination */
    asm volatile("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4),
                     "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9),
                     "r"(v10), "r"(v11), "r"(v12), "r"(v13), "r"(v14),
                     "r"(v15), "r"(v16), "r"(v17), "r"(v18), "r"(v19) : "memory");
}

/* Secondary function with different control flow pattern */
__attribute__((noinline, optimize("O3")))
void secondary_pressure_function(int seed) {
    volatile int a0, a1, a2, a3, a4, a5, a6, a7, a8, a9;
    volatile int b0, b1, b2, b3, b4, b5, b6, b7, b8, b9;
    
    /* Initialize */
    a0 = seed; b0 = seed + 1;
    for (int i = 1; i < 10; i++) {
        a0 = a0 + i;
        b0 = b0 * i;
    }
    
    /* Complex if-else chain */
    if (seed % 2) {
        a1 = a0 * 2; b1 = b0 / 2;
        if (seed % 3) {
            a2 = a1 + b1; b2 = a1 - b1;
            if (seed % 5) {
                a3 = a2 * b2; b3 = a2 + b2;
            } else {
                a3 = a2 - b2; b3 = a2 * b2;
            }
        } else {
            a2 = a1 - b1; b2 = a1 * b1;
        }
    } else {
        a1 = a0 / 2; b1 = b0 * 2;
    }
    
    /* Loop with switch inside */
    for (int i = 0; i < 8; i++) {
        switch (i % 4) {
            case 0: a4 = a1 + i; b4 = b1 - i; break;
            case 1: a5 = a2 * i; b5 = b2 + i; break;
            case 2: a6 = a3 - i; b6 = b3 * i; break;
            case 3: a7 = a4 + b4; b7 = a5 - b5; break;
        }
        asm volatile("" : : : "memory");
    }
}

int main() {
    /* Call high-pressure functions with different parameters */
    high_pressure_function(12);  /* Multiple of 3 and 4 */
    secondary_pressure_function(42);
    
    /* Additional calls with different iteration counts */
    for (int i = 0; i < 3; i++) {
        high_pressure_function(4 + i * 2);
    }
    
    return 0;
}

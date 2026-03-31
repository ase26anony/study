/* test_mcf_coverage.c
 * Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_coverage.c -o test.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Force noinline to prevent optimization from removing the function */
#define NOINLINE __attribute__((noinline))

/* High-pressure function with aggressive optimization */
NOINLINE __attribute__((optimize("O3")))
void high_pressure_function(void) {
    /* Declare 18 volatile variables to prevent optimization and increase pressure */
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    volatile int v10, v11, v12, v13, v14, v15, v16, v17;
    
    /* Additional non-volatile temporaries */
    int t0, t1, t2, t3, t4;
    
    /* Initialize with different values to prevent constant propagation */
    v0 = 1; v1 = 2; v2 = 3; v3 = 4; v4 = 5; v5 = 6;
    v6 = 7; v7 = 8; v8 = 9; v9 = 10; v10 = 11; v11 = 12;
    v12 = 13; v13 = 14; v14 = 15; v15 = 16; v16 = 17; v17 = 18;
    
    /* Memory barrier to force variables to be live across it */
    asm volatile("" : : : "memory");
    
    /* Complex loop with constant bounds to encourage unrolling */
    for (int i = 0; i < 4; i++) {
        /* Switch statement creates complex control flow */
        switch (i & 3) {
            case 0:
                /* Chain dependencies to create data flow */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 + v6;
                v7 = v5 * v8;
                
                /* Memory barrier between operations */
                asm volatile("" : : : "memory");
                
                t2 = v9 + v10;
                t3 = v11 - v12;
                v13 = t2 * t3;
                v14 = v13 + v15;
                break;
                
            case 1:
                /* Different operation pattern */
                t0 = v1 + v3;
                t1 = v5 * v7;
                v9 = t0 - t1;
                v11 = v9 + v13;
                v15 = v11 * v17;
                
                asm volatile("" : : : "memory");
                
                t2 = v2 + v4;
                t3 = v6 - v8;
                v10 = t2 * t3;
                v12 = v10 + v14;
                break;
                
            case 2:
                /* More complex dependency chain */
                t0 = v0 * v2;
                t1 = v4 + v6;
                v8 = t0 - t1;
                v10 = v8 + v12;
                v14 = v10 * v16;
                
                asm volatile("" : : : "memory");
                
                t2 = v1 * v3;
                t3 = v5 + v7;
                v9 = t2 - t3;
                v11 = v9 + v13;
                v15 = v11 * v17;
                break;
                
            case 3:
                /* Mix all variables */
                t0 = v0 + v2 + v4;
                t1 = v6 * v8 * v10;
                v12 = t0 - t1;
                v14 = v12 + v16;
                
                asm volatile("" : : : "memory");
                
                t2 = v1 + v3 + v5;
                t3 = v7 * v9 * v11;
                v13 = t2 - t3;
                v15 = v13 + v17;
                break;
        }
        
        /* Cross-iteration dependencies to extend live ranges */
        if (i > 0) {
            t4 = v0 + v17;
            v1 = t4 * i;
            v2 = v1 + v16;
        }
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
        
        /* Nested conditional to create more control flow edges */
        if (v0 > 10) {
            v3 = v4 + v5;
            v6 = v7 * v8;
        } else {
            v9 = v10 - v11;
            v12 = v13 / (v14 ? v14 : 1);
        }
    }
    
    /* Final computations using all variables */
    t0 = v0 + v1 + v2 + v3;
    t1 = v4 + v5 + v6 + v7;
    t2 = v8 + v9 + v10 + v11;
    t3 = v12 + v13 + v14 + v15;
    t4 = v16 + v17;
    
    /* Force all results to be used */
    asm volatile("" 
                 : "=r"(t0), "=r"(t1), "=r"(t2), "=r"(t3), "=r"(t4)
                 : "0"(t0), "1"(t1), "2"(t2), "3"(t3), "4"(t4)
                 : "memory");
}

/* Secondary function with different control flow pattern */
NOINLINE __attribute__((optimize("O3")))
void secondary_pressure_function(int seed) {
    volatile int w0, w1, w2, w3, w4, w5, w6, w7, w8, w9;
    volatile int w10, w11, w12, w13, w14, w15;
    
    w0 = seed; w1 = seed + 1; w2 = seed + 2; w3 = seed + 3;
    w4 = seed + 4; w5 = seed + 5; w6 = seed + 6; w7 = seed + 7;
    w8 = seed + 8; w9 = seed + 9; w10 = seed + 10; w11 = seed + 11;
    w12 = seed + 12; w13 = seed + 13; w14 = seed + 14; w15 = seed + 15;
    
    /* Loop with early exit to create more CFG edges */
    for (int j = 0; j < 8; j++) {
        if (j == 4) continue;
        
        /* Deep if-else chain */
        if (w0 > w1) {
            w2 = w3 + w4;
            w5 = w6 * w7;
        } else if (w1 > w2) {
            w8 = w9 - w10;
            w11 = w12 / (w13 ? w13 : 1);
        } else if (w2 > w3) {
            w14 = w15 + w0;
            w1 = w2 * w3;
        } else {
            w4 = w5 + w6;
            w7 = w8 * w9;
        }
        
        asm volatile("" : : : "memory");
        
        /* Nested loop */
        for (int k = 0; k < 2; k++) {
            w10 = w11 + w12 + k;
            w13 = w14 * w15 * (k + 1);
        }
    }
}

/* Main function that calls pressure functions */
int main(void) {
    /* Call high-pressure function multiple times with different contexts */
    high_pressure_function();
    
    /* Call secondary function to create different allocation patterns */
    for (int x = 0; x < 3; x++) {
        secondary_pressure_function(x * 10);
    }
    
    return 0;
}

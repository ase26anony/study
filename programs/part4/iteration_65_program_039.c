/* test_mcf_debug.c
 * Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_debug.c -o test.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Prevent inlining to preserve complex control flow */
__attribute__((noinline, optimize("O3")))
void high_pressure_function(void) {
    /* 18 volatile variables to force register pressure */
    volatile int v0 = 0, v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10, v11 = 11;
    volatile int v12 = 12, v13 = 13, v14 = 14, v15 = 15, v16 = 16, v17 = 17;
    
    /* Additional non-volatile temporaries */
    int t0, t1, t2, t3, t4;
    
    /* Memory barrier to force liveness across it */
    asm volatile("" : : : "memory");
    
    /* Complex loop with constant bound to encourage unrolling */
    for (int i = 0; i < 4; i++) {
        /* Switch creates control flow edges */
        switch (i & 3) {
            case 0:
                /* Chain dependencies to create data flow */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 + v6;
                v7 = v5 * v8;
                
                /* Memory barrier keeps variables live */
                asm volatile("" : : : "memory");
                
                t2 = v9 + v10;
                v11 = t2 - v12;
                v13 = v11 * v14;
                break;
                
            case 1:
                /* Different operation pattern */
                t3 = v15 - v16;
                v0 = t3 + v17;
                v1 = v0 * v2;
                v3 = v1 - v4;
                
                asm volatile("" : : : "memory");
                
                v5 = v6 + v7;
                v8 = v5 * v9;
                v10 = v8 - v11;
                break;
                
            case 2:
                /* More chained operations */
                t4 = v12 + v13;
                v14 = t4 * v15;
                v16 = v14 - v17;
                v0 = v16 + v1;
                
                asm volatile("" : : : "memory");
                
                v2 = v3 * v4;
                v5 = v2 - v6;
                v7 = v5 + v8;
                break;
                
            case 3:
                /* Use all variables in complex expression */
                v9 = (v10 + v11) * (v12 - v13);
                v14 = v9 / (v15 + 1);
                v16 = v14 * v17;
                v0 = v16 - v1;
                
                asm volatile("" : : : "memory");
                
                v2 = v3 + v4 + v5 + v6;
                v7 = v8 * v9 * v10;
                break;
        }
        
        /* Cross-iteration dependencies */
        v0 = v0 + i;
        v1 = v1 - i;
        v2 = v2 * (i + 1);
        
        /* Force all variables to be used somewhere */
        if (i & 1) {
            v3 = v4 + v5;
            v6 = v7 - v8;
        } else {
            v9 = v10 * v11;
            v12 = v13 / (v14 + 1);
        }
        
        /* Nested conditional to increase CFG complexity */
        if (v0 > 100) {
            v15 = v16 + v17;
        } else if (v1 < 50) {
            v16 = v15 - v14;
        } else {
            v17 = v13 * v12;
        }
    }
    
    /* Final computation using all variables */
    t0 = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    t1 = v9 + v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(t0), "r"(t1) : "memory");
}

/* Secondary function with different pressure pattern */
__attribute__((noinline, optimize("O3")))
void secondary_pressure(void) {
    volatile int w0 = 0, w1 = 1, w2 = 2, w3 = 3, w4 = 4;
    volatile int w5 = 5, w6 = 6, w7 = 7, w8 = 8, w9 = 9;
    
    /* Loop with early exit to create more CFG edges */
    for (int j = 0; j < 8; j++) {
        if (j == 4) continue;
        
        /* Complex expression chain */
        w0 = w1 + w2;
        w3 = w0 * w4;
        w5 = w3 - w6;
        w7 = w5 + w8;
        w9 = w7 * w0;
        
        /* Conditional with multiple branches */
        switch (j % 3) {
            case 0: w1 = w2 + w3; break;
            case 1: w4 = w5 - w6; break;
            case 2: w7 = w8 * w9; break;
        }
    }
}

int main(void) {
    /* Call pressure functions to ensure they're compiled */
    high_pressure_function();
    secondary_pressure();
    
    return 0;
}

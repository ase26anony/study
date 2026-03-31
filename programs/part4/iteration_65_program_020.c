/* test_mcf_debug.c
 * Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_debug.c -o test_mcf_debug.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Force no inlining to preserve control flow structure */
__attribute__((noinline, optimize("O3")))
void high_pressure_function(void) {
    /* 18 volatile variables to force register pressure */
    volatile int v0 = 0, v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10, v11 = 11;
    volatile int v12 = 12, v13 = 13, v14 = 14, v15 = 15, v16 = 16, v17 = 17;
    
    /* Additional non-volatile temporaries */
    int t0, t1, t2, t3, t4;
    
    /* Complex control flow with nested loops */
    for (int i = 0; i < 4; i++) {  /* Will likely unroll at O3 */
        /* Memory barrier forces variables live across it */
        asm volatile("" : : : "memory");
        
        /* Switch creates multiple control flow edges */
        switch (i & 3) {
            case 0:
                /* Chain of dependent operations */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 + v6;
                v7 = v5 * v8;
                /* Keep many variables live */
                t2 = v9 + v10 + v11 + v12;
                v13 = t2 - v7;
                break;
                
            case 1:
                /* Different operation pattern */
                t0 = v1 - v2;
                t1 = v3 * v4;
                v6 = t0 + t1;
                v8 = v6 / (v5 + 1);
                t2 = v9 * v10 * v11;
                v12 = t2 + v8;
                v14 = v12 | v13;
                break;
                
            case 2:
                /* More complex data flow */
                t0 = v2 * v3 * v4;
                t1 = v5 + v6 + v7;
                v8 = t0 - t1;
                v9 = v8 ^ v10;
                t2 = v11 & v12 & v13;
                v14 = t2 | v9;
                v15 = v14 + v16;
                t3 = v17 * 2;
                v0 = v15 - t3;
                break;
                
            case 3:
                /* Use all variables in a long dependency chain */
                t0 = v0 + v1;
                t1 = v2 + v3 + v4;
                t2 = v5 * v6 * v7;
                t3 = v8 - v9 - v10;
                t4 = v11 ^ v12 ^ v13;
                
                v14 = t0 + t1;
                v15 = t2 - t3;
                v16 = t4 + v14;
                v17 = v15 * v16;
                
                /* Circular dependency to confuse allocator */
                v0 = v17 & 0xFF;
                v1 = v0 + 1;
                break;
        }
        
        /* Another memory barrier between iterations */
        asm volatile("" : : : "memory");
        
        /* Cross-iteration dependencies */
        if (i > 0) {
            v2 = v1 + old_v1;
            v3 = v0 * old_v0;
        }
        
        /* Save values for next iteration */
        int old_v0 = v0;
        int old_v1 = v1;
        
        /* Small inner loop to increase pressure */
        for (int j = 0; j < 2; j++) {
            /* Use different subsets of variables */
            t0 = v4 + v5 + j;
            t1 = v6 * v7 * (j + 1);
            v8 = t0 ^ t1;
            v9 = v8 + v10;
        }
    }
    
    /* Final computation using all variables */
    t0 = v0 + v1 + v2 + v3;
    t1 = v4 + v5 + v6 + v7;
    t2 = v8 + v9 + v10 + v11;
    t3 = v12 + v13 + v14 + v15;
    t4 = v16 + v17;
    
    /* Force all results to be used */
    volatile int result = t0 + t1 + t2 + t3 + t4;
    (void)result;  /* Suppress unused warning */
}

/* Second function with different control flow pattern */
__attribute__((noinline, optimize("O3")))
void alternative_pressure_function(int selector) {
    volatile int w0 = selector, w1 = 1, w2 = 2, w3 = 3, w4 = 4;
    volatile int w5 = 5, w6 = 6, w7 = 7, w8 = 8, w9 = 9;
    volatile int w10 = 10, w11 = 11, w12 = 12, w13 = 13, w14 = 14;
    
    /* Deep if-else chain */
    if (selector & 1) {
        w1 = w0 * w2;
        w3 = w1 + w4;
        if (selector & 2) {
            w5 = w3 - w6;
            w7 = w5 * w8;
        } else {
            w9 = w3 / w10;
            w11 = w9 | w12;
        }
    } else {
        w2 = w0 + w13;
        w4 = w2 * w14;
        if (selector & 4) {
            w6 = w4 ^ w7;
            w8 = w6 & w9;
        } else {
            w10 = w4 - w11;
            w12 = w10 + w13;
        }
    }
    
    /* Loop with early exit */
    for (int i = 0; i < 8; i++) {
        if (i == selector) break;
        w0 = w0 + w1 + w2;
        w3 = w3 * w4 * w5;
        asm volatile("" : : : "memory");
    }
}

int main(void) {
    /* Call pressure functions multiple times with different inputs */
    high_pressure_function();
    
    for (int i = 0; i < 3; i++) {
        alternative_pressure_function(i);
    }
    
    return 0;
}

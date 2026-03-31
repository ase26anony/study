/* test_mcf_coverage.c
 * Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_coverage.c -o test.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Force noinline to prevent optimization across function boundaries */
#define NOINLINE __attribute__((noinline))

/* Create a high register pressure function with complex control flow */
NOINLINE __attribute__((optimize("O3")))
void high_pressure_function(int iterations) {
    /* 18 volatile variables to force register allocation */
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
    
    /* Outer loop - will be unrolled due to constant bound */
    for (int i = 0; i < 4; i++) {
        /* Complex switch creates multiple control flow edges */
        switch (i & 3) {
            case 0:
                /* Chain dependencies to create data flow */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 + v6;
                v7 = v5 * v8;
                
                /* Memory barrier extends live ranges */
                asm volatile("" : : : "memory");
                
                t2 = v9 + v10;
                t3 = v11 - v12;
                v13 = t2 * t3;
                v14 = v13 + v15;
                break;
                
            case 1:
                /* Different operation pattern */
                t0 = v1 * v2;
                t1 = v3 + v4;
                v6 = t0 - t1;
                v7 = v6 * v8;
                v9 = v7 + v10;
                
                asm volatile("" : : : "memory");
                
                t2 = v11 * v12;
                t3 = v13 - v14;
                v15 = t2 + t3;
                v16 = v15 * v17;
                break;
                
            case 2:
                /* More complex data flow */
                t0 = v2 + v3 + v4;
                t1 = v5 * v6 * v7;
                v8 = t0 - t1;
                v9 = v8 + v10 + v11;
                
                asm volatile("" : : : "memory");
                
                t2 = v12 * v13;
                t3 = v14 + v15;
                t4 = v16 - v17;
                v0 = t2 + t3 + t4;
                break;
                
            case 3:
                /* Use all variables in a complex expression */
                t0 = v0 * v1 + v2 * v3;
                t1 = v4 - v5 + v6 - v7;
                v8 = t0 * t1;
                v9 = v8 + v10 * v11;
                
                asm volatile("" : : : "memory");
                
                t2 = v12 + v13 + v14 + v15;
                t3 = v16 * v17;
                v0 = t2 - t3;
                v1 = v0 * 2;
                break;
        }
        
        /* Nested conditional to create more control flow complexity */
        if (i & 1) {
            t0 = v0 + v2 + v4;
            v6 = t0 * v8;
            v10 = v6 - v12;
        } else {
            t1 = v1 + v3 + v5;
            v7 = t1 * v9;
            v11 = v7 - v13;
        }
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
        
        /* Cross-case variable usage to extend live ranges */
        if (i > 0) {
            v14 = v0 + v1 + v2 + v3;
            v15 = v4 * v5 * v6;
        }
    }
    
    /* Final complex computation using all variables */
    t0 = v0 + v1 + v2 + v3 + v4 + v5;
    t1 = v6 * v7 * v8 * v9;
    t2 = v10 + v11 + v12 + v13;
    t3 = v14 * v15 * v16 * v17;
    
    /* Force all results to be used */
    asm volatile("" 
                 : "=r"(t0), "=r"(t1), "=r"(t2), "=r"(t3)
                 : "0"(t0), "1"(t1), "2"(t2), "3"(t3)
                 : "memory");
}

/* Secondary function with different pattern to increase overall complexity */
NOINLINE __attribute__((optimize("O3")))
void secondary_pressure_function(void) {
    volatile int w0, w1, w2, w3, w4, w5, w6, w7, w8, w9;
    volatile int w10, w11, w12, w13, w14, w15;
    
    w0 = 100; w1 = 101; w2 = 102; w3 = 103; w4 = 104; w5 = 105;
    w6 = 106; w7 = 107; w8 = 108; w9 = 109; w10 = 110; w11 = 111;
    w12 = 112; w13 = 113; w14 = 114; w15 = 115;
    
    /* Loop with early exit to create more CFG edges */
    for (int j = 0; j < 8; j++) {
        if (j == 4) continue;
        
        int temp = w0 + w1;
        w2 = temp * w3;
        w4 = w2 - w5;
        
        asm volatile("" : : : "memory");
        
        if (j & 2) {
            w6 = w7 * w8;
            w9 = w6 + w10;
        } else {
            w11 = w12 - w13;
            w14 = w11 * w15;
        }
    }
}

/* Main function - triggers compilation of pressure functions */
int main(void) {
    /* Call pressure functions multiple times to ensure they're compiled */
    high_pressure_function(4);
    secondary_pressure_function();
    
    /* Call again with different parameter to prevent constant folding */
    high_pressure_function(5);
    
    return 0;
}

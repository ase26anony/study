/* test_mcf_debug.c
 * Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_debug.c -o test.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Force noinline to prevent optimization across function boundaries */
#define NOINLINE __attribute__((noinline))

/* High-pressure function with aggressive optimization */
NOINLINE __attribute__((optimize("O3")))
void high_pressure_function(void) {
    /* 18 volatile variables to force register pressure */
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    volatile int v10, v11, v12, v13, v14, v15, v16, v17;
    
    /* Additional non-volatile temporaries */
    int t0, t1, t2, t3, t4;
    
    /* Initialize variables with different values */
    v0 = 1; v1 = 2; v2 = 3; v3 = 4; v4 = 5; v5 = 6;
    v6 = 7; v7 = 8; v8 = 9; v9 = 10; v10 = 11; v11 = 12;
    v12 = 13; v13 = 14; v14 = 15; v15 = 16; v16 = 17; v17 = 18;
    
    /* Memory barrier to force liveness across it */
    asm volatile("" : : : "memory");
    
    /* Complex loop with constant bounds to encourage unrolling */
    for (int i = 0; i < 4; ++i) {
        /* Nested control flow with switch statement */
        switch (i & 3) {
            case 0:
                /* Chain of dependent operations */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 + v6;
                v7 = v5 * v8;
                
                /* Memory barrier between blocks */
                asm volatile("" : : : "memory");
                
                /* More operations using different subsets */
                t2 = v9 + v10;
                t3 = v11 - v12;
                v13 = t2 * t3;
                v14 = v13 + v15;
                break;
                
            case 1:
                /* Different operation pattern */
                t0 = v1 - v2;
                t1 = v3 * v4;
                v6 = t0 + t1;
                v7 = v6 - v8;
                v9 = v7 * v10;
                
                asm volatile("" : : : "memory");
                
                t2 = v11 + v12;
                t3 = v13 - v14;
                v15 = t2 * t3;
                v16 = v15 + v17;
                break;
                
            case 2:
                /* Yet another pattern */
                t0 = v2 + v3;
                t1 = v4 * v5;
                v8 = t0 - t1;
                v9 = v8 + v10;
                v11 = v9 * v12;
                
                asm volatile("" : : : "memory");
                
                t2 = v13 + v14;
                t3 = v15 - v16;
                v17 = t2 * t3;
                v0 = v17 + v1;
                break;
                
            case 3:
                /* Final pattern using remaining variables */
                t0 = v3 - v4;
                t1 = v5 * v6;
                v10 = t0 + t1;
                v11 = v10 - v12;
                v13 = v11 * v14;
                
                asm volatile("" : : : "memory");
                
                t2 = v15 + v16;
                t3 = v17 - v0;
                v1 = t2 * t3;
                v2 = v1 + v3;
                break;
        }
        
        /* Cross-iteration dependencies to extend live ranges */
        if (i > 0) {
            t4 = v0 + v1 + v2 + v3;
            v4 = t4 / 4;  /* Average-like operation */
        }
        
        /* Force all variables to be used somewhere */
        v5 = v0 + v1;
        v6 = v2 + v3;
        v7 = v4 + v5;
        v8 = v6 + v7;
        v9 = v8 + v10;
        v11 = v9 + v12;
        v13 = v11 + v14;
        v15 = v13 + v16;
        v17 = v15 + v0;  /* Circular dependency */
    }
    
    /* Final complex expression using all variables */
    t0 = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7;
    t1 = v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    t2 = v16 + v17 + t0 + t1;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(t0), "r"(t1), "r"(t2) : "memory");
}

/* Secondary function with different control flow to increase graph complexity */
NOINLINE __attribute__((optimize("O2")))
void secondary_pressure(void) {
    volatile int a0, a1, a2, a3, a4, a5;
    int b0, b1;
    
    a0 = 100; a1 = 200; a2 = 300; a3 = 400; a4 = 500; a5 = 600;
    
    /* Loop with if-else chain */
    for (int j = 0; j < 3; ++j) {
        if (j == 0) {
            b0 = a0 + a1;
            b1 = a2 * a3;
            a4 = b0 - b1;
        } else if (j == 1) {
            b0 = a1 - a2;
            b1 = a3 * a4;
            a5 = b0 + b1;
        } else {
            b0 = a2 + a3;
            b1 = a4 * a5;
            a0 = b0 - b1;
        }
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
}

/* Main function that calls pressure functions */
int main(void) {
    /* Call high-pressure function multiple times to ensure compilation */
    high_pressure_function();
    secondary_pressure();
    
    /* Call again with different context */
    for (int k = 0; k < 2; ++k) {
        high_pressure_function();
    }
    
    return 0;
}

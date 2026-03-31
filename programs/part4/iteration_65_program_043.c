/* test_mcf_coverage.c
 * Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_coverage.c -o test.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Prevent optimizations from eliminating critical variables */
#define KEEP_ALIVE asm volatile("" : : : "memory")

/* High-pressure function with complex control flow */
__attribute__((noinline, optimize("O3")))
void high_pressure_function(int iterations) {
    /* 18 volatile variables to force register pressure */
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    volatile int v10, v11, v12, v13, v14, v15, v16, v17;
    
    /* Additional non-volatile temporaries */
    int t0, t1, t2, t3, t4;
    
    /* Initialize all variables with different values */
    v0 = 1; v1 = 2; v2 = 3; v3 = 4; v4 = 5; v5 = 6;
    v6 = 7; v7 = 8; v8 = 9; v9 = 10; v10 = 11; v11 = 12;
    v12 = 13; v13 = 14; v14 = 15; v15 = 16; v16 = 17; v17 = 18;
    
    /* Complex loop with unrollable iterations */
    for (int i = 0; i < iterations; i++) {
        /* Memory barrier to force liveness across operations */
        KEEP_ALIVE;
        
        /* Switch with multiple cases creating complex CFG */
        switch (i % 5) {
            case 0:
                /* Chain of dependent operations */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 + v6;
                v7 = v5 * v8;
                KEEP_ALIVE;
                break;
                
            case 1:
                /* Different dependency pattern */
                t2 = v9 - v10;
                t3 = v11 + v12;
                v13 = t2 * t3;
                v14 = v13 / v15;
                v16 = v14 + v17;
                KEEP_ALIVE;
                break;
                
            case 2:
                /* Cross-case variable usage */
                v0 = v16 + v17;
                v1 = v0 * v2;
                v3 = v1 - v4;
                v5 = v3 + v6;
                KEEP_ALIVE;
                break;
                
            case 3:
                /* More complex arithmetic chains */
                t4 = v7 * v8;
                v9 = t4 + v10;
                v11 = v9 - v12;
                v13 = v11 * v14;
                v15 = v13 + v16;
                KEEP_ALIVE;
                break;
                
            case 4:
                /* Use all variables in a long chain */
                t0 = v0 + v1;
                t1 = v2 + v3;
                t2 = v4 + v5;
                t3 = v6 + v7;
                t4 = v8 + v9;
                v10 = t0 + t1;
                v11 = t2 + t3;
                v12 = t4 + v10;
                v13 = v11 + v12;
                v14 = v13 + v15;
                v16 = v14 + v17;
                KEEP_ALIVE;
                break;
        }
        
        /* Nested conditional to extend live ranges */
        if (i % 3 == 0) {
            v0 = v1 + v2;
            v3 = v4 - v5;
            KEEP_ALIVE;
        } else if (i % 3 == 1) {
            v6 = v7 * v8;
            v9 = v10 / v11;
            KEEP_ALIVE;
        } else {
            v12 = v13 + v14;
            v15 = v16 - v17;
            KEEP_ALIVE;
        }
        
        /* Force cross-iteration dependencies */
        v17 = v0 + i;
        v16 = v17 * 2;
    }
    
    /* Final computation using all variables */
    t0 = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    t1 = v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(t0), "r"(t1));
}

/* Secondary function with different pressure pattern */
__attribute__((noinline, optimize("O3")))
void pressure_helper(int x) {
    volatile int a0, a1, a2, a3, a4, a5, a6, a7;
    volatile int b0, b1, b2, b3, b4, b5, b6, b7;
    
    a0 = x; a1 = x+1; a2 = x+2; a3 = x+3;
    a4 = x+4; a5 = x+5; a6 = x+6; a7 = x+7;
    b0 = x*2; b1 = x*3; b2 = x*4; b3 = x*5;
    b4 = x*6; b5 = x*7; b6 = x*8; b7 = x*9;
    
    /* Loop with small constant bound to encourage unrolling */
    for (int i = 0; i < 4; i++) {
        KEEP_ALIVE;
        
        /* Complex expression using many variables */
        a0 = a1 + b0;
        a1 = a2 + b1;
        a2 = a3 + b2;
        a3 = a4 + b3;
        a4 = a5 + b4;
        a5 = a6 + b5;
        a6 = a7 + b6;
        a7 = b7 + i;
        
        /* Swap groups */
        int tmp = b0;
        b0 = b1; b1 = b2; b2 = b3; b3 = b4;
        b4 = b5; b5 = b6; b6 = b7; b7 = tmp;
        
        KEEP_ALIVE;
    }
    
    /* Force all variables to be used */
    asm volatile("" : : 
        "r"(a0), "r"(a1), "r"(a2), "r"(a3),
        "r"(a4), "r"(a5), "r"(a6), "r"(a7),
        "r"(b0), "r"(b1), "r"(b2), "r"(b3),
        "r"(b4), "r"(b5), "r"(b6), "r"(b7));
}

/* Main function to ensure compilation */
int main() {
    /* Call high-pressure functions with different parameters
     * to create varied allocation scenarios */
    high_pressure_function(8);
    pressure_helper(42);
    
    /* Additional calls with different values */
    for (int i = 0; i < 3; i++) {
        high_pressure_function(i + 2);
        pressure_helper(i * 10);
    }
    
    return 0;
}

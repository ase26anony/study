/* test_mcf_coverage.c
 * Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_coverage.c -o test.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Prevent optimizations from eliminating critical variables */
#define VOLATILE_VAR volatile int

/* Memory barrier to force variable liveness */
#define MEMORY_BARRIER() asm volatile("" : : : "memory")

/* Function attribute to ensure optimization and prevent inlining */
#define HIGH_PRESSURE_FN __attribute__((noinline, optimize("O3")))

/* High register pressure function */
HIGH_PRESSURE_FN
void high_pressure_function(int iterations) {
    /* Declare many volatile variables to force register pressure */
    VOLATILE_VAR v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    VOLATILE_VAR v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    
    /* Additional non-volatile temporaries */
    int t0, t1, t2, t3, t4;
    
    /* Initialize variables with different values to prevent constant propagation */
    v0 = 1; v1 = 2; v2 = 3; v3 = 4; v4 = 5;
    v5 = 6; v6 = 7; v7 = 8; v8 = 9; v9 = 10;
    v10 = 11; v11 = 12; v12 = 13; v13 = 14; v14 = 15;
    v15 = 16; v16 = 17; v17 = 18; v18 = 19; v19 = 20;
    
    /* Complex loop structure to create control flow edges */
    for (int i = 0; i < iterations; i++) {
        /* Memory barrier to force all variables live across */
        MEMORY_BARRIER();
        
        /* Nested conditional structure creating multiple control flow paths */
        switch (i % 5) {
            case 0:
                /* Chain of dependent operations */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 + v6;
                v7 = v5 * v8;
                v9 = v7 / (v10 + 1);
                v11 = v9 | v12;
                v13 = v11 & v14;
                break;
                
            case 1:
                /* Different chain using another subset */
                t2 = v15 - v16;
                t3 = v17 * v18;
                v19 = t2 + t3;
                v0 = v19 ^ v1;
                v2 = v0 << 2;
                v3 = v2 >> 1;
                v6 = v3 + v4;
                v8 = v6 * v7;
                break;
                
            case 2:
                /* Mix variables from both groups */
                t4 = v9 + v10;
                v11 = t4 - v12;
                v13 = v11 * v14;
                v15 = v13 / v16;
                v17 = v15 | v18;
                v19 = v17 & v0;
                v1 = v19 ^ v2;
                break;
                
            case 3:
                /* More complex arithmetic chains */
                v3 = (v4 * v5) + (v6 / v7);
                v8 = (v9 - v10) * (v11 + v12);
                v13 = v14 | v15;
                v16 = v17 ^ v18;
                v19 = v0 & v1;
                v2 = v3 << v4;
                break;
                
            case 4:
                /* Use all variables in a long dependency chain */
                t0 = v0 + v1;
                t1 = v2 + v3;
                t2 = v4 + v5;
                t3 = v6 + v7;
                t4 = v8 + v9;
                
                v10 = t0 * t1;
                v11 = t2 * t3;
                v12 = t4 * v10;
                v13 = v11 + v12;
                v14 = v13 - v15;
                v16 = v14 * v17;
                v18 = v16 / v19;
                v0 = v18 + v1;
                break;
        }
        
        /* Another memory barrier to extend live ranges */
        MEMORY_BARRIER();
        
        /* Additional control flow with if-else chains */
        if (i % 3 == 0) {
            v0 = v1 + v2;
            v3 = v4 - v5;
            v6 = v7 * v8;
        } else if (i % 3 == 1) {
            v9 = v10 / v11;
            v12 = v13 | v14;
            v15 = v16 ^ v17;
        } else {
            v18 = v19 << 2;
            v0 = v1 >> 1;
            v2 = v3 & v4;
        }
        
        /* Force cross-iteration dependencies */
        v5 = v6 + i;
        v7 = v8 * (i + 1);
        v9 = v10 - v11;
    }
    
    /* Final use of all variables to ensure they're live through the loop */
    t0 = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
         v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
    
    /* Prevent dead code elimination */
    MEMORY_BARRIER();
    asm volatile("" : "+r"(t0));
}

/* Secondary function with different control flow pattern */
HIGH_PRESSURE_FN
void secondary_pressure_function(void) {
    VOLATILE_VAR a0, a1, a2, a3, a4, a5, a6, a7, a8, a9;
    VOLATILE_VAR b0, b1, b2, b3, b4, b5, b6, b7, b8, b9;
    
    /* Initialize */
    a0 = 100; a1 = 101; a2 = 102; a3 = 103; a4 = 104;
    a5 = 105; a6 = 106; a7 = 107; a8 = 108; a9 = 109;
    b0 = 200; b1 = 201; b2 = 202; b3 = 203; b4 = 204;
    b5 = 205; b6 = 206; b7 = 207; b8 = 208; b9 = 209;
    
    /* Unrolled loop */
    for (int i = 0; i < 4; i++) {
        /* Complex expression with many operands */
        a0 = (a1 * a2) + (a3 / a4) - (a5 | a6) ^ (a7 & a8) + a9;
        b0 = (b1 - b2) * (b3 + b4) / (b5 ^ b6) | (b7 << b8) & b9;
        
        /* Swap groups */
        int tmp = a0; a0 = b0; b0 = tmp;
        tmp = a1; a1 = b1; b1 = tmp;
        
        MEMORY_BARRIER();
    }
}

/* Main function to ensure compilation */
int main(void) {
    /* Call high-pressure functions with compile-time known iteration count */
    high_pressure_function(8);  /* Multiple of 5 for all switch cases */
    secondary_pressure_function();
    
    return 0;
}

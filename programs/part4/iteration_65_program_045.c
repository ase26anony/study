/* test_mcf_coverage.c
 * Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_coverage.c -o test.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Force noinline to prevent optimization across function boundaries */
#define NOINLINE __attribute__((noinline))

/* Create a high register pressure function with complex control flow */
NOINLINE __attribute__((optimize("O3")))
void high_pressure_function(int iterations) {
    /* 18 volatile variables to force register allocation pressure */
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    volatile int v10, v11, v12, v13, v14, v15, v16, v17;
    
    /* Additional non-volatile temporaries */
    int t0, t1, t2, t3, t4;
    
    /* Initialize all variables with different values */
    v0 = 1; v1 = 2; v2 = 3; v3 = 4; v4 = 5; v5 = 6;
    v6 = 7; v7 = 8; v8 = 9; v9 = 10; v10 = 11; v11 = 12;
    v12 = 13; v13 = 14; v14 = 15; v15 = 16; v16 = 17; v17 = 18;
    
    /* Memory barrier to force variables to be live across it */
    asm volatile("" : : : "memory");
    
    /* Outer loop that will likely be unrolled */
    for (int i = 0; i < iterations; i++) {
        /* Complex switch statement creating multiple control flow edges */
        switch (i % 5) {
            case 0:
                /* Chain of dependent operations */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 + v6;
                v7 = v5 * v8;
                
                /* Memory barrier between blocks */
                asm volatile("" : : : "memory");
                
                /* More operations keeping many variables live */
                t2 = v9 + v10;
                t3 = v11 - v12;
                v13 = t2 * t3;
                v14 = v13 + v15;
                v16 = v14 - v17;
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
                t3 = v12 - v13;
                v14 = t2 * t3;
                v15 = v14 + v16;
                v17 = v15 - v0;
                break;
                
            case 2:
                /* Yet another pattern */
                t0 = v2 + v3;
                t1 = v4 * v5;
                v6 = t0 - t1;
                v7 = v6 + v8;
                v9 = v7 * v10;
                
                asm volatile("" : : : "memory");
                
                t2 = v11 + v12;
                t3 = v13 - v14;
                v15 = t2 * t3;
                v16 = v15 + v17;
                v0 = v16 - v1;
                break;
                
            case 3:
                /* Complex nested if-else chain */
                if (v0 > v1) {
                    t0 = v2 + v3;
                    if (v4 < v5) {
                        t1 = v6 * v7;
                        v8 = t0 - t1;
                    } else {
                        t1 = v8 * v9;
                        v10 = t0 + t1;
                    }
                } else {
                    t0 = v11 - v12;
                    if (v13 > v14) {
                        t1 = v15 * v16;
                        v17 = t0 - t1;
                    } else {
                        t1 = v0 * v1;
                        v2 = t0 + t1;
                    }
                }
                
                asm volatile("" : : : "memory");
                
                /* More operations */
                t2 = v3 + v4;
                t3 = v5 - v6;
                v7 = t2 * t3;
                v8 = v7 + v9;
                v10 = v8 - v11;
                break;
                
            case 4:
                /* Mix of all operations */
                t0 = v12 + v13;
                t1 = v14 * v15;
                v16 = t0 - t1;
                
                /* Inner loop to increase pressure */
                for (int j = 0; j < 2; j++) {
                    v17 = v16 + j;
                    v0 = v17 * v1;
                    v2 = v0 - v3;
                }
                
                asm volatile("" : : : "memory");
                
                t2 = v4 + v5;
                t3 = v6 - v7;
                v8 = t2 * t3;
                v9 = v8 + v10;
                v11 = v9 - v12;
                break;
        }
        
        /* Cross-iteration dependencies to extend live ranges */
        v0 = v1 + 1;
        v1 = v2 - 1;
        v2 = v3 * 2;
        v3 = v4 / 2;
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
        
        /* More operations keeping variables live */
        t4 = v5 + v6;
        v7 = t4 * v8;
        v9 = v7 - v10;
        v11 = v9 + v12;
    }
    
    /* Final computation using all variables */
    t0 = v0 + v1 + v2 + v3 + v4 + v5;
    t1 = v6 + v7 + v8 + v9 + v10 + v11;
    t2 = v12 + v13 + v14 + v15 + v16 + v17;
    
    /* Force result to be used */
    volatile int result = t0 + t1 + t2;
    (void)result;  /* Prevent unused variable warning */
}

/* Secondary function with different pressure pattern */
NOINLINE __attribute__((optimize("O3")))
void secondary_pressure_function(void) {
    volatile int a0, a1, a2, a3, a4, a5, a6, a7, a8, a9;
    volatile int b0, b1, b2, b3, b4, b5, b6, b7, b8, b9;
    
    /* Initialize */
    a0 = 1; a1 = 2; a2 = 3; a3 = 4; a4 = 5;
    a5 = 6; a6 = 7; a7 = 8; a8 = 9; a9 = 10;
    b0 = 11; b1 = 12; b2 = 13; b3 = 14; b4 = 15;
    b5 = 16; b6 = 17; b7 = 18; b8 = 19; b9 = 20;
    
    /* Complex expression tree */
    int r0 = a0 + a1 * a2 - a3 / a4 + a5 % a6;
    int r1 = b0 - b1 * b2 + b3 / b4 - b5 % b6;
    int r2 = a7 * b7 + a8 / b8 - a9 % b9;
    
    /* Chain of dependent operations */
    for (int i = 0; i < 3; i++) {
        r0 = r0 + r1;
        r1 = r1 - r2;
        r2 = r2 * r0;
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
        
        /* Use all variables */
        a0 = r0 + i;
        b0 = r1 - i;
        a1 = r2 * i;
    }
    
    volatile int final = r0 + r1 + r2 + a0 + b0 + a1;
    (void)final;
}

/* Main function to ensure everything gets compiled */
int main(void) {
    /* Call high pressure function with compile-time known iteration count */
    high_pressure_function(4);
    
    /* Call secondary function */
    secondary_pressure_function();
    
    return 0;
}

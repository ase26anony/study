/* test_mcf_debug.c
 * Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_debug.c -o test_mcf_debug.o
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
    int t0, t1, t2, t3;
    
    /* Initialize with different values to prevent constant propagation */
    v0 = 1; v1 = 2; v2 = 3; v3 = 4; v4 = 5; v5 = 6;
    v6 = 7; v7 = 8; v8 = 9; v9 = 10; v10 = 11; v11 = 12;
    v12 = 13; v13 = 14; v14 = 15; v15 = 16; v16 = 17; v17 = 18;
    
    /* Memory barrier to force liveness across it */
    asm volatile("" : : : "memory");
    
    /* Complex loop with constant bounds to encourage unrolling */
    for (int i = 0; i < 4; i++) {
        /* Switch creates complex control flow */
        switch (i & 3) {
            case 0:
                /* Chain of dependent operations */
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
                break;
                
            case 1:
                /* Different operation pattern */
                t0 = v15 - v16;
                t1 = v17 * v0;
                v1 = t0 + t1;
                v2 = v1 - v3;
                v4 = v2 * v5;
                
                asm volatile("" : : : "memory");
                
                t2 = v6 + v7;
                v8 = t2 * v9;
                v10 = v8 - v11;
                break;
                
            case 2:
                /* More operations mixing variables */
                t0 = v12 * v13;
                t1 = v14 + v15;
                v16 = t0 - t1;
                v17 = v16 * v0;
                v1 = v17 + v2;
                
                asm volatile("" : : : "memory");
                
                t2 = v3 - v4;
                v5 = t2 * v6;
                v7 = v5 + v8;
                break;
                
            case 3:
                /* Final pattern using remaining variables */
                t0 = v9 * v10;
                t1 = v11 - v12;
                v13 = t0 + t1;
                v14 = v13 * v15;
                v16 = v14 - v17;
                
                asm volatile("" : : : "memory");
                
                t2 = v0 + v1;
                v2 = t2 * v3;
                v4 = v2 - v5;
                break;
        }
        
        /* Cross-iteration dependencies to prevent dead code elimination */
        if (i > 0) {
            v0 = v0 + v17;
            v1 = v1 + v16;
            v2 = v2 + v15;
        }
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
    }
    
    /* Final computation using all variables to ensure they're live at end */
    t0 = v0 + v1 + v2 + v3;
    t1 = v4 + v5 + v6 + v7;
    t2 = v8 + v9 + v10 + v11;
    t3 = v12 + v13 + v14 + v15;
    
    /* Force use of results to prevent elimination */
    asm volatile("" : : "r"(t0), "r"(t1), "r"(t2), "r"(t3));
}

/* Nested function to increase graph complexity */
NOINLINE __attribute__((optimize("O2")))
void nested_pressure(int iterations) {
    volatile int a = 1, b = 2, c = 3, d = 4;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex if-else chain */
        if (i % 2 == 0) {
            a = b + c;
            b = c * d;
            asm volatile("" : : : "memory");
        } else if (i % 3 == 0) {
            c = d - a;
            d = a * b;
            asm volatile("" : : : "memory");
        } else {
            a = b - c;
            d = a + b;
        }
        
        /* Loop with internal branching */
        for (int j = 0; j < 2; j++) {
            if (j == 0) {
                a = a + 1;
                b = b - 1;
            } else {
                c = c * 2;
                d = d / 2;
            }
        }
    }
}

/* Main function that calls pressure functions */
int main(void) {
    /* Call high-pressure function multiple times with different paths */
    for (int k = 0; k < 3; k++) {
        high_pressure_function();
        
        /* Call nested function with different iteration counts */
        switch (k) {
            case 0: nested_pressure(2); break;
            case 1: nested_pressure(3); break;
            case 2: nested_pressure(4); break;
        }
    }
    
    return 0;
}

/* test_mcf_coverage.c
 * Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_coverage.c -o test.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Force noinline to prevent optimization from merging functions */
#define NOINLINE __attribute__((noinline))

/* High-pressure function with aggressive optimization */
NOINLINE __attribute__((optimize("O3")))
void high_pressure_function(void) {
    /* 18 volatile variables to force register pressure */
    volatile int v0 = 0, v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10, v11 = 11;
    volatile int v12 = 12, v13 = 13, v14 = 14, v15 = 15, v16 = 16, v17 = 17;
    
    /* Additional non-volatile temporaries */
    int t0, t1, t2, t3, t4;
    
    /* Memory barrier to force liveness across it */
    asm volatile("" : : : "memory");
    
    /* Complex loop with constant bounds to encourage unrolling */
    for (int i = 0; i < 4; i++) {
        /* Switch with multiple cases creating complex CFG */
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
                v11 = t2 - v12;
                v13 = v11 * v14;
                v15 = v13 + v16;
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
                v12 = t2 * v13;
                v14 = v12 - v15;
                v16 = v14 + v17;
                break;
                
            case 2:
                /* More complex chain */
                t0 = v2 * v3;
                t1 = v4 + v5;
                v6 = t0 - t1;
                t2 = v7 * v8;
                v9 = v6 + t2;
                
                asm volatile("" : : : "memory");
                
                t3 = v10 - v11;
                t4 = v12 * v13;
                v14 = t3 + t4;
                v15 = v14 - v16;
                v17 = v15 * v0;
                break;
                
            case 3:
                /* Long dependency chain */
                t0 = v3 + v4;
                v5 = t0 * v6;
                t1 = v7 - v8;
                v9 = v5 + t1;
                
                asm volatile("" : : : "memory");
                
                t2 = v10 * v11;
                v12 = t2 - v13;
                t3 = v14 + v15;
                v16 = v12 * t3;
                v17 = v16 - v0;
                break;
        }
        
        /* Cross-iteration dependencies to extend live ranges */
        if (i > 0) {
            v0 = v17 + i;
            v1 = v16 - i;
        }
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
        
        /* Nested conditional to create more CFG edges */
        if (i % 2 == 0) {
            t0 = v0 + v1;
            v2 = t0 * v3;
        } else {
            t0 = v4 - v5;
            v6 = t0 + v7;
        }
    }
    
    /* Final complex expression using all variables */
    t0 = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    t1 = v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(t0), "r"(t1) : "memory");
}

/* Secondary function with different pattern to increase overall pressure */
NOINLINE __attribute__((optimize("O3")))
void secondary_pressure_function(int seed) {
    volatile int w0 = seed, w1 = seed + 1, w2 = seed + 2;
    volatile int w3 = seed + 3, w4 = seed + 4, w5 = seed + 5;
    volatile int w6 = seed + 6, w7 = seed + 7, w8 = seed + 8;
    
    int u0, u1, u2;
    
    /* Loop with early exit to create more CFG complexity */
    for (int j = 0; j < 8; j++) {
        if (j == 4) continue;
        
        u0 = w0 + w1;
        u1 = w2 * w3;
        w4 = u0 - u1;
        
        asm volatile("" : : : "memory");
        
        if (j > 2) {
            u2 = w5 + w6;
            w7 = u2 * w8;
        } else {
            u2 = w5 - w6;
            w7 = w8 + u2;
        }
        
        /* Rotate values to create cross-iteration dependencies */
        w0 = w1;
        w1 = w2;
        w2 = w3;
        w3 = w4;
        w4 = w5;
        w5 = w6;
        w6 = w7;
        w7 = w8;
        w8 = u0 + j;
    }
    
    asm volatile("" : : "r"(w0), "r"(w1), "r"(w2) : "memory");
}

/* Main function that calls pressure functions */
int main(void) {
    /* Call high-pressure functions multiple times with different seeds */
    high_pressure_function();
    secondary_pressure_function(42);
    secondary_pressure_function(100);
    
    /* Small loop in main to add some pressure to main's context too */
    volatile int x = 0;
    for (int k = 0; k < 3; k++) {
        x += k;
        asm volatile("" : : "r"(x) : "memory");
    }
    
    return x;
}

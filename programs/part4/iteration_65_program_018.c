/* test_mcf_coverage.c
 * Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_coverage.c -o test.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Prevent optimizations from eliminating our pressure variables */
#define VOLATILE_INT volatile int

/* Memory barrier to force liveness across operations */
#define MEMORY_BARRIER() asm volatile("" : : : "memory")

/* Function attribute to ensure aggressive optimization */
#define HIGH_PRESSURE __attribute__((noinline, optimize("O3")))

/* High-pressure function that should trigger complex IRA fixup graph */
HIGH_PRESSURE void create_register_pressure(void) {
    /* 18 volatile variables to create high register pressure */
    VOLATILE_INT v0 = 1, v1 = 2, v2 = 3, v3 = 4, v4 = 5, v5 = 6;
    VOLATILE_INT v6 = 7, v7 = 8, v8 = 9, v9 = 10, v10 = 11, v11 = 12;
    VOLATILE_INT v12 = 13, v13 = 14, v14 = 15, v15 = 16, v16 = 17, v17 = 18;
    
    /* Additional non-volatile temporaries */
    int t0, t1, t2, t3, t4;
    
    /* Complex control flow with nested loops */
    for (int outer = 0; outer < 4; ++outer) {
        MEMORY_BARRIER();
        
        /* Switch creates multiple control flow edges */
        switch (outer & 3) {
            case 0:
                /* Chain of dependent operations */
                v0 = v1 + v2;
                v3 = v0 * v4;
                v5 = v3 - v6;
                v7 = v5 / (v8 + 1);
                t0 = v7 + v9;
                v10 = t0 * v11;
                break;
                
            case 1:
                /* Different dependency chain */
                v12 = v13 - v14;
                v15 = v12 * v16;
                v17 = v15 + v0;
                t1 = v17 - v1;
                v2 = t1 * v3;
                v4 = v2 + v5;
                break;
                
            case 2:
                /* More complex operations */
                v6 = v7 * v8;
                v9 = v6 - v10;
                v11 = v9 + v12;
                t2 = v11 * v13;
                v14 = t2 - v15;
                v16 = v14 / (v17 + 1);
                break;
                
            case 3:
                /* All variables used together */
                t3 = v0 + v1 + v2 + v3 + v4 + v5;
                t4 = v6 + v7 + v8 + v9 + v10 + v11;
                v12 = t3 * t4;
                v13 = v12 - (v14 + v15 + v16 + v17);
                break;
        }
        
        MEMORY_BARRIER();
        
        /* Inner loop with unrollable iterations */
        for (int inner = 0; inner < 3; ++inner) {
            /* Cross-case variable usage extends live ranges */
            if (inner == 0) {
                v0 = v1 + v2;
                v3 = v4 - v5;
            } else if (inner == 1) {
                v6 = v7 * v8;
                v9 = v10 / (v11 + 1);
            } else {
                v12 = v13 + v14;
                v15 = v16 - v17;
            }
            
            /* More arithmetic to create data flow */
            v1 = v0 + outer;
            v2 = v1 * inner;
            v4 = v3 - v2;
            v5 = v4 + v6;
            v7 = v5 * v9;
            v8 = v7 - v12;
            v10 = v8 + v15;
            v11 = v10 * v13;
            v14 = v11 - v16;
            v17 = v14 + v0;  /* Circular dependency */
        }
        
        MEMORY_BARRIER();
        
        /* Conditional block with variable usage */
        if (outer & 1) {
            v0 = v1 + v17;
            v2 = v3 * v16;
            v4 = v5 - v15;
            v6 = v7 + v14;
            v8 = v9 * v13;
            v10 = v11 - v12;
        } else {
            v1 = v0 * v17;
            v3 = v2 + v16;
            v5 = v4 * v15;
            v7 = v6 - v14;
            v9 = v8 + v13;
            v11 = v10 * v12;
        }
    }
    
    /* Final use to prevent dead code elimination */
    MEMORY_BARRIER();
    v0 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + 
         v11 + v12 + v13 + v14 + v15 + v16 + v17;
    
    /* Prevent elimination of entire function */
    asm volatile("" : : "r"(v0));
}

/* Secondary pressure function with different pattern */
HIGH_PRESSURE void more_pressure(int seed) {
    VOLATILE_INT w0 = seed, w1 = seed + 1, w2 = seed + 2;
    VOLATILE_INT w3 = seed + 3, w4 = seed + 4, w5 = seed + 5;
    VOLATILE_INT w6 = seed + 6, w7 = seed + 7, w8 = seed + 8;
    
    /* Loop with multiple exits to create complex CFG */
    for (int i = 0; i < 8; ++i) {
        if (i == 1) {
            w0 = w1 * w2;
            w3 = w4 - w5;
            continue;
        }
        if (i == 3) {
            w6 = w7 + w8;
            w1 = w0 * w3;
            break;
        }
        if (i == 5) {
            w2 = w4 / (w6 + 1);
            w5 = w7 - w8;
            continue;
        }
        
        /* Default operations */
        w0 = w1 + i;
        w2 = w3 * i;
        w4 = w5 - i;
        w6 = w7 + i;
        w8 = w0 * i;
    }
    
    /* Use results */
    asm volatile("" : : "r"(w0), "r"(w1), "r"(w2), "r"(w3), 
                       "r"(w4), "r"(w5), "r"(w6), "r"(w7), "r"(w8));
}

/* Main function to ensure compilation */
int main(void) {
    create_register_pressure();
    more_pressure(42);
    return 0;
}

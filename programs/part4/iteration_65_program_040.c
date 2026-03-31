/* test_mcf_coverage.c
 * Designed to trigger debug dumps in GCC's min-cost flow solver
 * Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_coverage.c -o test.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Force noinline to prevent optimization across function boundaries */
#define NOINLINE __attribute__((noinline))
/* Force aggressive optimization on this function */
#define HIGH_PRESSURE NOINLINE __attribute__((optimize("O3")))

/* Memory barrier to force variables to be live across it */
#define MEMORY_BARRIER() asm volatile("" : : : "memory")

/* Function to create extreme register pressure */
HIGH_PRESSURE
int create_register_pressure(int seed) {
    /* Declare many volatile variables to prevent optimization */
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    volatile int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    
    /* Additional non-volatile temporaries */
    int t0, t1, t2, t3, t4, t5;
    
    /* Initialize with seed to create data dependencies */
    v0 = seed;
    v1 = seed * 2;
    v2 = seed + 1;
    v3 = seed - 1;
    v4 = seed ^ 0x55;
    v5 = seed | 0xAA;
    v6 = seed & 0xFF;
    v7 = seed << 2;
    v8 = seed >> 1;
    v9 = ~seed;
    v10 = seed + 100;
    v11 = seed * 3;
    v12 = seed / 2;
    v13 = seed % 7;
    v14 = seed + 200;
    v15 = seed * 4;
    v16 = seed - 50;
    v17 = seed ^ 0x33;
    v18 = seed | 0xCC;
    v19 = seed & 0xF0;
    
    /* Complex control flow with nested loops */
    for (int i = 0; i < 4; i++) {  /* Small constant for potential unrolling */
        /* Switch creates multiple control flow edges */
        switch (i % 3) {
            case 0:
                /* Chain of dependent operations */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 * v6;
                v7 = v5 + v8;
                
                MEMORY_BARRIER();  /* Force all variables live across barrier */
                
                t2 = v9 + v10;
                t3 = v11 * v12;
                v13 = t2 - t3;
                v14 = v13 * v15;
                v16 = v14 + v17;
                break;
                
            case 1:
                /* Different chain of operations */
                t4 = v1 + v3;
                t5 = v5 * v7;
                v0 = t4 - t5;
                v2 = v0 * v4;
                v6 = v2 + v8;
                
                MEMORY_BARRIER();
                
                v9 = v10 + v12;
                v11 = v13 * v15;
                v14 = v9 - v11;
                v16 = v14 * v18;
                v19 = v16 + v17;
                break;
                
            case 2:
                /* Yet another chain */
                v0 = v1 + v2 + v3;
                v4 = v5 * v6 * v7;
                v8 = v9 - v10 - v11;
                v12 = v13 * v14 / 2;
                
                MEMORY_BARRIER();
                
                /* Cross-dependencies between variables */
                v15 = v0 + v4;
                v16 = v8 * v12;
                v17 = v15 - v16;
                v18 = v17 * v19;
                v19 = v18 + seed;
                break;
        }
        
        /* Inner loop with more operations */
        for (int j = 0; j < 2; j++) {
            /* Mix all variables together */
            t0 = v0 + v2 + v4 + v6 + v8;
            t1 = v1 + v3 + v5 + v7 + v9;
            v10 = t0 - t1;
            v11 = v10 * (j + 1);
            
            /* Conditional inside inner loop */
            if (j == 0) {
                v12 = v11 + v13;
                v14 = v12 * v15;
            } else {
                v16 = v11 + v17;
                v18 = v16 * v19;
            }
            
            MEMORY_BARRIER();
        }
        
        /* More arithmetic with all variables */
        v0 = v1 + v2;
        v3 = v4 * v5;
        v6 = v7 - v8;
        v9 = v10 / (i + 1);
        v11 = v12 ^ v13;
        v14 = v15 | v16;
        v17 = v18 & v19;
        
        MEMORY_BARRIER();
    }
    
    /* Final computation using all variables */
    int result = 0;
    result += v0 + v1 + v2 + v3 + v4;
    result += v5 + v6 + v7 + v8 + v9;
    result += v10 + v11 + v12 + v13 + v14;
    result += v15 + v16 + v17 + v18 + v19;
    
    /* Additional computation to create more register pressure */
    for (int k = 0; k < 3; k++) {
        result = (result * 1103515245 + 12345) & 0x7fffffff;
        result ^= v0 ^ v1 ^ v2 ^ v3 ^ v4;
        result += v5 + v6 + v7 + v8 + v9;
    }
    
    return result;
}

/* Trivial main function to make the file compilable */
int main(void) {
    int result = create_register_pressure(42);
    
    /* Use result to prevent dead code elimination */
    asm volatile("" : : "r"(result) : "memory");
    
    return 0;
}

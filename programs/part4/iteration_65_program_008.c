/* test_mcf_coverage.c
 * Designed to trigger debug dumps in GCC's min-cost flow solver
 * Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_coverage.c -o test.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Prevent inlining to preserve control flow structure */
#define NOINLINE __attribute__((noinline))
/* Force aggressive optimization within the function */
#define AGGRESSIVE __attribute__((optimize("O3")))

/* High register pressure function */
NOINLINE AGGRESSIVE
static int high_pressure_function(int seed) {
    /* 18 volatile variables to force register allocation pressure */
    volatile int v0 = seed + 1;
    volatile int v1 = seed + 2;
    volatile int v2 = seed + 3;
    volatile int v3 = seed + 4;
    volatile int v4 = seed + 5;
    volatile int v5 = seed + 6;
    volatile int v6 = seed + 7;
    volatile int v7 = seed + 8;
    volatile int v8 = seed + 9;
    volatile int v9 = seed + 10;
    volatile int v10 = seed + 11;
    volatile int v11 = seed + 12;
    volatile int v12 = seed + 13;
    volatile int v13 = seed + 14;
    volatile int v14 = seed + 15;
    volatile int v15 = seed + 16;
    volatile int v16 = seed + 17;
    volatile int v17 = seed + 18;
    
    /* Additional non-volatile temporaries */
    int t0, t1, t2, t3, t4;
    
    /* Memory barrier to force liveness across operations */
    #define MEMORY_BARRIER() asm volatile("" : : : "memory")
    
    /* Complex control flow with nested loops */
    for (int i = 0; i < 4; i++) {  /* Constant bound encourages unrolling */
        /* Switch creates multiple control flow edges */
        switch (i & 3) {
            case 0:
                /* Chain of dependent operations */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 * v6;
                MEMORY_BARRIER();
                
                t2 = v7 + v8;
                t3 = v9 - v10;
                v11 = t2 * t3;
                v12 = v11 / (v13 + 1);
                break;
                
            case 1:
                /* Different operation pattern */
                t0 = v1 * v2;
                t1 = v3 + v4;
                v5 = t0 - t1;
                v6 = v5 * v7;
                MEMORY_BARRIER();
                
                t2 = v8 + v9;
                t3 = v10 * v11;
                v12 = t2 - t3;
                v13 = v12 + v14;
                break;
                
            case 2:
                /* More complex dependency chain */
                t0 = v2 + v3 + v4;
                t1 = v5 * v6;
                t2 = v7 - v8;
                v9 = t0 * t1;
                v10 = t2 + v9;
                MEMORY_BARRIER();
                
                t3 = v11 + v12;
                t4 = v13 * v14;
                v15 = t3 - t4;
                v16 = v15 * v17;
                break;
                
            case 3:
                /* All variables used together */
                t0 = v0 + v1 + v2 + v3;
                t1 = v4 * v5 * v6;
                t2 = v7 - v8 - v9;
                t3 = v10 * v11 + v12;
                t4 = v13 + v14 + v15;
                
                v16 = t0 * t1;
                v17 = t2 + t3 + t4;
                MEMORY_BARRIER();
                
                /* Cross-case variable usage */
                v0 = v16 + 1;
                v1 = v17 - 1;
                break;
        }
        
        /* Loop-carried dependencies */
        if (i > 0) {
            v0 = v0 + v1;
            v2 = v2 * v3;
            v4 = v4 - v5;
            MEMORY_BARRIER();
        }
        
        /* Nested conditional */
        if (v0 > 100) {
            v6 = v6 / 2;
            v7 = v7 * 2;
        } else {
            v8 = v8 + v9;
            v10 = v10 - v11;
        }
        
        /* Another memory barrier */
        MEMORY_BARRIER();
        
        /* Use all volatile variables in a complex expression */
        t0 = v0 + v1 + v2 + v3 + v4 + v5;
        t1 = v6 * v7 * v8 * v9;
        t2 = v10 + v11 + v12 + v13;
        t3 = v14 * v15 * v16;
        t4 = v17 + seed;
        
        /* Force spill pressure with large expression */
        v0 = (t0 * t1) + (t2 - t3) * t4;
    }
    
    /* Final aggregation using all variables */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17;
    
    return result;
}

/* Secondary function with different control flow pattern */
NOINLINE AGGRESSIVE
static int alternate_pressure_function(int base) {
    volatile int a = base * 2;
    volatile int b = base * 3;
    volatile int c = base * 4;
    volatile int d = base * 5;
    volatile int e = base * 6;
    volatile int f = base * 7;
    volatile int g = base * 8;
    volatile int h = base * 9;
    
    int sum = 0;
    
    /* Loop with early exit creates more complex CFG */
    for (int j = 0; j < 8; j++) {
        if (j & 1) {
            a = b + c;
            b = c * d;
            asm volatile("" : : : "memory");
        } else {
            e = f - g;
            f = h * a;
            asm volatile("" : : : "memory");
        }
        
        /* Nested loop */
        for (int k = 0; k < 2; k++) {
            c = d + e;
            d = f - g;
        }
        
        sum += a + b + c + d + e + f + g + h;
    }
    
    return sum;
}

/* Main function to ensure compilation */
int main(int argc, char **argv) {
    int result1 = high_pressure_function(argc);
    int result2 = alternate_pressure_function(argc);
    
    /* Use results to prevent dead code elimination */
    return result1 + result2;
}

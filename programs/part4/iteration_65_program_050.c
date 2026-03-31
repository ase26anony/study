/* test_mcf_coverage.c
 * Designed to trigger debug output in GCC's min-cost flow solver
 * Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_coverage.c -o test.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Force noinline to prevent optimization across function boundaries */
#define NOINLINE __attribute__((noinline))

/* High register pressure function with complex control flow */
NOINLINE __attribute__((optimize("O3")))
static void high_pressure_function(void)
{
    /* 18 volatile variables to force register pressure */
    volatile int v0 = 0, v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10, v11 = 11;
    volatile int v12 = 12, v13 = 13, v14 = 14, v15 = 15, v16 = 16, v17 = 17;
    
    /* Additional non-volatile temporaries */
    int t0, t1, t2, t3, t4;
    
    /* Memory barrier to force liveness across operations */
    #define MEMORY_BARRIER() asm volatile("" : : : "memory")
    
    /* Complex control flow with nested loops */
    for (int i = 0; i < 4; i++) {  /* Constant bound encourages unrolling */
        /* Switch creates multiple control flow edges */
        switch (i & 3) {
            case 0:
                /* Chain dependencies to create data flow */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 + t1;
                v5 = v4 - v6;
                MEMORY_BARRIER();
                
                /* Use many variables simultaneously */
                t2 = v7 + v8 + v9 + v10;
                v11 = t2 * v12;
                v13 = v14 - v15 + v16;
                MEMORY_BARRIER();
                break;
                
            case 1:
                /* Different variable usage pattern */
                t0 = v1 * v3 * v5;
                v7 = t0 + v9;
                t1 = v11 + v13 + v15;
                v17 = t1 - v0;
                MEMORY_BARRIER();
                
                t2 = v2 * v4 * v6;
                v8 = t2 + v10;
                t3 = v12 + v14 + v16;
                v0 = t3 - v1;
                MEMORY_BARRIER();
                break;
                
            case 2:
                /* More complex arithmetic chains */
                t0 = (v0 * v2) + (v4 * v6);
                t1 = (v8 * v10) + (v12 * v14);
                v16 = t0 + t1;
                v17 = v16 * v1;
                MEMORY_BARRIER();
                
                t2 = v3 + v5 + v7 + v9;
                t3 = v11 + v13 + v15 + v17;
                v0 = t2 * t3;
                MEMORY_BARRIER();
                break;
                
            case 3:
                /* Use all variables in a massive expression */
                t0 = v0 + v2 + v4 + v6 + v8 + v10 + v12 + v14 + v16;
                t1 = v1 + v3 + v5 + v7 + v9 + v11 + v13 + v15 + v17;
                t2 = t0 * t1;
                t3 = t2 / (i + 1);
                
                /* Distribute results back to variables */
                v0 = t3 + v0;
                v1 = t3 + v1;
                v2 = t3 + v2;
                v3 = t3 + v3;
                MEMORY_BARRIER();
                
                v4 = t3 + v4;
                v5 = t3 + v5;
                v6 = t3 + v6;
                v7 = t3 + v7;
                MEMORY_BARRIER();
                break;
        }
        
        /* Inner loop to extend live ranges */
        for (int j = 0; j < 2; j++) {
            /* Cross-case variable usage */
            t4 = v0 + v8 + v16;
            v1 = v1 * t4;
            v9 = v9 + t4;
            v17 = v17 - t4;
            MEMORY_BARRIER();
        }
        
        /* Conditional that uses many live variables */
        if (i & 1) {
            t0 = v0 + v2 + v4;
            t1 = v6 + v8 + v10;
            v12 = t0 * t1;
            MEMORY_BARRIER();
        } else {
            t0 = v1 + v3 + v5;
            t1 = v7 + v9 + v11;
            v13 = t0 * t1;
            MEMORY_BARRIER();
        }
    }
    
    /* Final computation using all variables */
    t0 = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7;
    t1 = v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    t2 = v16 + v17;
    t3 = t0 * t1 * t2;
    
    /* Force use of result to prevent dead code elimination */
    asm volatile("" : "+r"(t3));
}

/* Secondary function with different pressure pattern */
NOINLINE __attribute__((optimize("O3")))
static void alternate_pressure_pattern(int seed)
{
    volatile int a0 = seed, a1 = seed+1, a2 = seed+2, a3 = seed+3;
    volatile int b0 = seed*2, b1 = seed*3, b2 = seed*4, b3 = seed*5;
    volatile int c0, c1, c2, c3, d0, d1, d2, d3;
    
    int temp;
    
    /* Unrolled loop */
    for (int i = 0; i < 3; i++) {
        c0 = a0 * b0 + i;
        c1 = a1 * b1 + i;
        c2 = a2 * b2 + i;
        c3 = a3 * b3 + i;
        
        asm volatile("" : : : "memory");
        
        d0 = c0 + c1;
        d1 = c2 + c3;
        d2 = d0 * d1;
        d3 = d2 - i;
        
        asm volatile("" : : : "memory");
        
        a0 = d0 + 1;
        a1 = d1 + 2;
        a2 = d2 + 3;
        a3 = d3 + 4;
    }
    
    temp = a0 + a1 + a2 + a3 + b0 + b1 + b2 + b3;
    asm volatile("" : "+r"(temp));
}

/* Main function that calls pressure functions */
int main(void)
{
    /* Call high pressure function multiple times with different contexts */
    high_pressure_function();
    
    /* Call alternate pattern to increase overall compilation complexity */
    for (int i = 0; i < 3; i++) {
        alternate_pressure_pattern(i);
    }
    
    return 0;
}

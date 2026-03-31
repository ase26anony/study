/* test_mcf_debug.c
 * Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_debug.c -o test.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Prevent inlining to preserve control flow structure */
#define NOINLINE __attribute__((noinline))
/* Force aggressive optimization on this function */
#define AGGRESSIVE __attribute__((optimize("O3")))

/* Memory barrier to force variable liveness across operations */
#define MEMORY_BARRIER() asm volatile("" : : : "memory")

/* High register pressure function */
NOINLINE AGGRESSIVE
static void high_pressure_function(void) {
    /* 18 volatile variables to force register allocation pressure */
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    volatile int v10, v11, v12, v13, v14, v15, v16, v17;
    
    /* Additional non-volatile temporaries */
    int t0, t1, t2, t3, t4;
    
    /* Initialize variables with different values to prevent constant propagation */
    v0 = 1; v1 = 2; v2 = 3; v3 = 4; v4 = 5; v5 = 6;
    v6 = 7; v7 = 8; v8 = 9; v9 = 10; v10 = 11; v11 = 12;
    v12 = 13; v13 = 14; v14 = 15; v15 = 16; v16 = 17; v17 = 18;
    
    /* Complex control flow with nested loops */
    for (int i = 0; i < 4; i++) {  /* Small constant for potential unrolling */
        MEMORY_BARRIER();
        
        /* Switch with multiple cases creating different control flow paths */
        switch (i & 3) {
            case 0:
                /* Chain of dependent operations */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 + v6;
                v7 = v5 * v8;
                v9 = v7 / (v10 + 1);
                MEMORY_BARRIER();
                break;
                
            case 1:
                /* Different dependency pattern */
                t2 = v11 ^ v12;
                t3 = v13 | v14;
                v15 = t2 & t3;
                v16 = v15 << 2;
                v17 = v16 >> 1;
                v0 = v17 + v1;
                MEMORY_BARRIER();
                break;
                
            case 2:
                /* More arithmetic chains */
                t4 = v2 * v3 * v4;
                v5 = t4 + v6 + v7;
                v8 = v5 - v9 - v10;
                v11 = v8 * v12;
                v13 = v11 / (v14 + 1);
                MEMORY_BARRIER();
                break;
                
            case 3:
                /* Mixed operations keeping many variables live */
                v0 = v1 + v2;
                v3 = v4 - v5;
                v6 = v7 * v8;
                v9 = v10 ^ v11;
                v12 = v13 | v14;
                v15 = v16 & v17;
                MEMORY_BARRIER();
                
                /* Nested conditional */
                if (v0 > v3) {
                    v4 = v6 + v9;
                    v12 = v15 * 2;
                } else {
                    v5 = v12 - v15;
                    v8 = v9 / 2;
                }
                MEMORY_BARRIER();
                break;
        }
        
        /* Loop-carried dependencies */
        v0 = v0 + i;
        v1 = v1 - i;
        v2 = v2 * (i + 1);
        
        /* Another memory barrier to extend live ranges */
        MEMORY_BARRIER();
        
        /* Inner loop with constant bound */
        for (int j = 0; j < 2; j++) {
            /* Use different subsets of variables */
            if (j == 0) {
                v3 = v4 + v5;
                v6 = v7 - v8;
            } else {
                v9 = v10 * v11;
                v12 = v13 / (v14 + 1);
            }
            
            /* Cross-variable operations */
            v15 = v3 + v6 + v9 + v12;
            v16 = v15 ^ (v17 + j);
            
            MEMORY_BARRIER();
        }
        
        /* Final mixing of all variables */
        t0 = v0 + v1 + v2 + v3 + v4;
        t1 = v5 + v6 + v7 + v8 + v9;
        t2 = v10 + v11 + v12 + v13 + v14;
        t3 = v15 + v16 + v17;
        
        /* Force all results to be used */
        v0 = t0 - t1;
        v1 = t1 - t2;
        v2 = t2 - t3;
        v3 = t3 - t0;
        
        MEMORY_BARRIER();
    }
    
    /* Final use to prevent dead code elimination */
    volatile int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                         v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17;
    (void)result;  /* Suppress unused warning */
}

/* Secondary function with different control flow pattern */
NOINLINE AGGRESSIVE
static void alternate_pressure_path(int selector) {
    volatile int a0, a1, a2, a3, a4, a5, a6, a7, a8, a9;
    volatile int b0, b1, b2, b3, b4, b5, b6, b7;
    
    /* Initialize */
    a0 = 100; a1 = 101; a2 = 102; a3 = 103; a4 = 104;
    a5 = 105; a6 = 106; a7 = 107; a8 = 108; a9 = 109;
    b0 = 200; b1 = 201; b2 = 202; b3 = 203;
    b4 = 204; b5 = 205; b6 = 206; b7 = 207;
    
    /* Deeply nested conditionals */
    if (selector > 0) {
        if (selector > 10) {
            a0 = a1 + a2;
            a3 = a4 * a5;
            b0 = b1 - b2;
            b3 = b4 / (b5 + 1);
            MEMORY_BARRIER();
        } else {
            a6 = a7 ^ a8;
            a9 = a0 | a1;
            b6 = b7 & b0;
            b1 = b2 << 1;
            MEMORY_BARRIER();
        }
        
        /* Small unrolled loop */
        for (int k = 0; k < 3; k++) {
            a0 = a0 + k;
            a1 = a1 - k;
            a2 = a2 * (k + 1);
            b0 = b0 + a0;
            b1 = b1 + a1;
            b2 = b2 + a2;
            MEMORY_BARRIER();
        }
    }
    
    /* Use results */
    volatile int sum = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 +
                      b0 + b1 + b2 + b3 + b4 + b5 + b6 + b7;
    (void)sum;
}

/* Main function that calls pressure functions */
int main(void) {
    /* Call high pressure function multiple times with different paths */
    high_pressure_function();
    
    /* Call alternate path to create different allocation patterns */
    for (int s = 0; s < 5; s++) {
        alternate_pressure_path(s);
    }
    
    return 0;
}

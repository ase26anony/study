/* test-mcf-debug.c
 * Test program to trigger MCF_DEBUG output in GCC's min-cost flow solver.
 * Compile with: gcc-debug -O3 -funroll-loops -c test-mcf-debug.c -o test.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Force noinline to prevent optimization from removing the pressure function */
#define NOINLINE __attribute__((noinline))

/* Create high register pressure with volatile variables */
NOINLINE __attribute__((optimize("O3")))
static void high_pressure_function(void) {
    /* 18 volatile variables to create many simultaneously live values */
    volatile int v0 = 0, v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10, v11 = 11;
    volatile int v12 = 12, v13 = 13, v14 = 14, v15 = 15, v16 = 16, v17 = 17;
    
    /* Additional non-volatile temporaries to increase register working set */
    int t0, t1, t2, t3, t4, t5;
    
    /* Memory barrier to force variables to be live across it */
    asm volatile("" : : : "memory");
    
    /* Complex control flow with nested loops */
    for (int i = 0; i < 4; i++) {  /* Small constant loop encourages unrolling */
        /* Switch with multiple cases creates complex CFG edges */
        switch (i & 3) {
            case 0:
                /* Chain dependencies to create data flow */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 + v6;
                /* Memory barrier between operations */
                asm volatile("" : : : "memory");
                t2 = v7 ^ v8;
                v9 = v5 | t2;
                v10 = v9 & v11;
                break;
                
            case 1:
                /* Different operation pattern */
                t3 = v12 - v13;
                t4 = v14 / (v15 + 1);  /* Avoid division by zero */
                v16 = t3 * t4;
                v17 = v16 << 2;
                asm volatile("" : : : "memory");
                v0 = v1 ^ v17;
                v2 = v0 & 0xFF;
                v3 = v2 | 0x80;
                break;
                
            case 2:
                /* More chained operations */
                t5 = v4 + v5 + v6;
                v7 = t5 * v8;
                v9 = v7 - v10;
                asm volatile("" : : : "memory");
                v11 = v9 ^ v12;
                v13 = v11 | v14;
                v15 = v13 & 0x7F;
                break;
                
            case 3:
                /* All variables used together */
                v0 = v1 + v2 + v3 + v4;
                v5 = v6 * v7 * v8;
                v9 = v10 - v11 - v12;
                asm volatile("" : : : "memory");
                v13 = v14 ^ v15 ^ v16;
                v17 = v0 | v5 | v9 | v13;
                break;
        }
        
        /* Cross-iteration dependencies to extend live ranges */
        if (i > 0) {
            v1 = v0 + v17;
            v2 = v1 * i;
            asm volatile("" : : : "memory");
            v3 = v2 - v16;
            v4 = v3 ^ 0x55;
        }
        
        /* Inner loop to create more pressure */
        for (int j = 0; j < 2; j++) {
            v5 = v6 + j;
            v7 = v8 * (j + 1);
            asm volatile("" : : : "memory");
            v9 = v10 - v5;
            v11 = v7 ^ v9;
        }
    }
    
    /* Final computation using all variables */
    t0 = v0 + v1 + v2 + v3 + v4 + v5;
    t1 = v6 + v7 + v8 + v9 + v10 + v11;
    t2 = v12 + v13 + v14 + v15 + v16 + v17;
    asm volatile("" : : : "memory");
    
    /* Force all results to be used */
    volatile int result = t0 + t1 + t2;
    (void)result;  /* Suppress unused warning */
}

/* Secondary pressure function with different pattern */
NOINLINE __attribute__((optimize("O3")))
static void more_pressure(void) {
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
    volatile int g = 7, h = 8, i = 9, j = 10, k = 11, l = 12;
    
    /* Complex if-else chain */
    for (int x = 0; x < 3; x++) {
        if (x == 0) {
            a = b + c;
            d = e * f;
            asm volatile("" : : : "memory");
        } else if (x == 1) {
            g = h - i;
            j = k ^ l;
            asm volatile("" : : : "memory");
        } else {
            a = g + j;
            d = h * k;
            asm volatile("" : : : "memory");
        }
        
        /* Nested switch */
        switch (x) {
            case 0: b = c + d; break;
            case 1: e = f + g; break;
            case 2: h = i + j; break;
        }
    }
}

/* Main function to ensure everything gets compiled */
int main(void) {
    high_pressure_function();
    more_pressure();
    return 0;
}

/* test-mcf-debug.c
 * Compile with: gcc-debug -O3 -funroll-loops -c test-mcf-debug.c -o test.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Force noinline to prevent optimization across function boundaries */
#define NOINLINE __attribute__((noinline))

/* High register pressure function with complex control flow */
NOINLINE __attribute__((optimize("O3")))
static void high_pressure_function(int seed) {
    /* Declare many volatile variables to prevent optimization and increase pressure */
    volatile int v0 = seed;
    volatile int v1 = seed + 1;
    volatile int v2 = seed + 2;
    volatile int v3 = seed + 3;
    volatile int v4 = seed + 4;
    volatile int v5 = seed + 5;
    volatile int v6 = seed + 6;
    volatile int v7 = seed + 7;
    volatile int v8 = seed + 8;
    volatile int v9 = seed + 9;
    volatile int v10 = seed + 10;
    volatile int v11 = seed + 11;
    volatile int v12 = seed + 12;
    volatile int v13 = seed + 13;
    volatile int v14 = seed + 14;
    volatile int v15 = seed + 15;
    volatile int v16 = seed + 16;
    volatile int v17 = seed + 17;
    
    /* Additional non-volatile temporaries */
    int t0, t1, t2, t3, t4;
    
    /* Memory barrier to force variables live across it */
    asm volatile("" : : : "memory");
    
    /* Complex nested loop to create overlapping live ranges */
    for (int i = 0; i < 4; i++) {  /* Small constant for potential unrolling */
        /* Switch with multiple cases for complex CFG */
        switch ((seed + i) % 5) {
            case 0:
                /* Chain of dependent operations */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 + v6;
                /* Memory barrier */
                asm volatile("" : : : "memory");
                v7 = v5 * v8;
                v9 = v7 / (v10 + 1);
                break;
                
            case 1:
                /* Different operation chain */
                t2 = v11 | v12;
                t3 = v13 & v14;
                v15 = t2 ^ t3;
                v16 = v15 << 2;
                /* Memory barrier */
                asm volatile("" : : : "memory");
                v17 = v16 >> 1;
                v0 = v17 + v1;
                break;
                
            case 2:
                /* More arithmetic with data dependencies */
                t4 = v2 + v3;
                v4 = t4 * v5;
                v6 = v4 - v7;
                v8 = v6 / (v9 + 1);
                /* Memory barrier */
                asm volatile("" : : : "memory");
                v10 = v8 | v11;
                v12 = v10 & v13;
                break;
                
            case 3:
                /* Cross-case variable usage */
                v14 = v0 + v15;
                v16 = v1 * v17;
                v2 = v14 - v16;
                v3 = v2 + v4;
                /* Memory barrier */
                asm volatile("" : : : "memory");
                v5 = v3 * v6;
                v7 = v5 / (v8 + 1);
                break;
                
            case 4:
                /* All variables used together */
                t0 = v0 + v1 + v2 + v3;
                t1 = v4 * v5 * v6;
                v7 = t0 - t1;
                v8 = v7 + v9 + v10;
                /* Memory barrier */
                asm volatile("" : : : "memory");
                v11 = v8 * v12;
                v13 = v11 / (v14 + 1);
                v15 = v13 | v16;
                v17 = v15 & v0;
                break;
        }
        
        /* Loop-carried dependencies */
        v0 = v0 + i;
        v1 = v1 - i;
        v2 = v2 * (i + 1);
        
        /* Conditional inside loop for more CFG edges */
        if (i % 2 == 0) {
            v3 = v3 + v4;
            v5 = v5 - v6;
            /* Memory barrier */
            asm volatile("" : : : "memory");
        } else {
            v7 = v7 * v8;
            v9 = v9 / (v10 + 1);
        }
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
    }
    
    /* Final computation using all variables */
    t0 = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7;
    t1 = v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    t2 = v16 + v17;
    
    /* Force result to be observable */
    asm volatile("" : "+r"(t0), "+r"(t1), "+r"(t2) : : "memory");
}

/* Secondary function with different pressure pattern */
NOINLINE __attribute__((optimize("O3")))
static void alternate_pressure_function(int seed) {
    volatile int w0 = seed * 2;
    volatile int w1 = seed * 3;
    volatile int w2 = seed * 4;
    volatile int w3 = seed * 5;
    volatile int w4 = seed * 6;
    volatile int w5 = seed * 7;
    volatile int w6 = seed * 8;
    volatile int w7 = seed * 9;
    volatile int w8 = seed * 10;
    volatile int w9 = seed * 11;
    
    int temp;
    
    /* Deeply nested conditionals */
    for (int j = 0; j < 3; j++) {
        if (seed > 0) {
            if (j % 2 == 0) {
                temp = w0 + w1;
                w2 = temp * w3;
                /* Memory barrier */
                asm volatile("" : : : "memory");
            } else {
                temp = w4 - w5;
                w6 = temp / (w7 + 1);
            }
            
            if (temp > 100) {
                w8 = w8 | w9;
                w0 = w0 & w1;
            } else {
                w2 = w2 ^ w3;
                w4 = w4 << 1;
            }
        }
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
        
        /* Small unrolled section */
        w5 = w5 + 1;
        w6 = w6 - 1;
        w7 = w7 * 2;
        w8 = w8 / 2;
    }
}

/* Main function that calls pressure functions */
int main(void) {
    /* Call high pressure function multiple times with different seeds */
    high_pressure_function(42);
    high_pressure_function(100);
    high_pressure_function(255);
    
    /* Call alternate function */
    alternate_pressure_function(50);
    alternate_pressure_function(200);
    
    return 0;
}

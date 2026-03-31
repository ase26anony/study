/* test-mcf-debug.c
 * Test program to trigger MCF_DEBUG output in GCC's min-cost flow solver.
 * Compile with: gcc-debug -O3 -funroll-loops -c test-mcf-debug.c -o test.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Force optimization level for high-pressure function */
#ifdef __GNUC__
#define OPT_O3 __attribute__((optimize("O3")))
#define NOINLINE __attribute__((noinline))
#else
#define OPT_O3
#define NOINLINE
#endif

/* Memory barrier to force variable liveness */
#define MEMORY_BARRIER() asm volatile("" : : : "memory")

/* High register pressure function */
NOINLINE OPT_O3
static int high_pressure_function(int seed) {
    /* Declare many volatile variables to prevent optimization
     * and increase register pressure */
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
    
    /* Complex control flow with nested loops */
    for (int i = 0; i < 4; i++) {  /* Small constant for potential unrolling */
        MEMORY_BARRIER();
        
        /* Switch with multiple cases creating different control flow paths */
        switch (i & 3) {
            case 0:
                /* Chain dependencies to create data flow */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 + v6;
                v7 = v5 * v8;
                MEMORY_BARRIER();
                break;
                
            case 1:
                /* Different dependency chain */
                t2 = v9 - v10;
                t3 = v11 + v12;
                v13 = t2 * t3;
                v14 = v13 / (v15 + 1);
                v16 = v14 | v17;
                MEMORY_BARRIER();
                break;
                
            case 2:
                /* More complex operations */
                t4 = (v0 & v1) | (v2 ^ v3);
                v4 = t4 + v5;
                v6 = v4 * v7;
                v8 = v6 - v9;
                v10 = v8 >> 2;
                MEMORY_BARRIER();
                break;
                
            case 3:
                /* Mixed operations */
                v11 = (v12 + v13) * (v14 - v15);
                v16 = v11 & 0xFF;
                v17 = v16 | v0;
                t0 = v1 + v2 + v3;
                v4 = t0 * v5;
                MEMORY_BARRIER();
                break;
        }
        
        /* Cross-iteration dependencies to extend live ranges */
        if (i > 0) {
            v0 = v0 + v17;
            v1 = v1 - v16;
            v2 = v2 * v15;
            v3 = v3 | v14;
        }
        
        /* Nested conditional to create more control flow edges */
        if (v0 & 1) {
            v4 = v4 + v5;
            v6 = v6 * v7;
        } else {
            v8 = v8 - v9;
            v10 = v10 ^ v11;
        }
        
        /* Another memory barrier */
        MEMORY_BARRIER();
        
        /* Force all variables to be used in computation */
        t1 = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
        t2 = v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17;
        v0 = (t1 - t2) & 0xFF;
    }
    
    /* Final computation using all variables */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17;
    
    return result;
}

/* Secondary function with different control flow pattern */
NOINLINE OPT_O3
static int secondary_pressure(int x) {
    volatile int a = x * 2;
    volatile int b = x + 3;
    volatile int c = x - 4;
    volatile int d = x / 2;
    volatile int e = x & 0xF;
    volatile int f = x | 0x10;
    volatile int g = x ^ 0x20;
    volatile int h = ~x;
    
    /* Loop with if-else chain */
    for (int i = 0; i < 3; i++) {
        MEMORY_BARRIER();
        
        if (i == 0) {
            a = b + c;
            d = e * f;
        } else if (i == 1) {
            g = h - a;
            b = c ^ d;
        } else {
            e = f | g;
            h = a & b;
        }
        
        /* Cross-variable operations */
        a = a + d;
        b = b - e;
        c = c * f;
        d = d ^ g;
    }
    
    return a + b + c + d + e + f + g + h;
}

/* Main function that calls pressure functions */
int main(void) {
    int result = 0;
    
    /* Call high-pressure function multiple times with different seeds
     * to potentially create different allocation patterns */
    for (int i = 0; i < 10; i++) {
        result += high_pressure_function(i);
        result += secondary_pressure(i);
    }
    
    /* Prevent dead code elimination */
    volatile int sink = result;
    
    return sink & 1;
}

/* test-mcf-debug.c
 * Test program to trigger MCF_DEBUG output in GCC's min-cost flow solver.
 * Compile with: gcc-debug -O3 -funroll-loops -c test-mcf-debug.c -o test.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Force optimization level for the high-pressure function */
#ifdef __GNUC__
#define OPT_O3 __attribute__((optimize("O3")))
#define NOINLINE __attribute__((noinline))
#else
#define OPT_O3
#define NOINLINE
#endif

/* Memory barrier to force variable liveness */
#define MEMORY_BARRIER() asm volatile("" : : : "memory")

/* Function to create extreme register pressure */
NOINLINE OPT_O3
static int create_register_pressure(int seed) {
    /* Declare many volatile variables to prevent optimization
     * and force register allocation */
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
    int result = 0;
    
    /* Outer loop - will be unrolled */
    for (int i = 0; i < 4; i++) {
        MEMORY_BARRIER();
        
        /* Switch creates multiple control flow edges */
        switch (i % 3) {
            case 0:
                /* Chain dependencies to create data flow */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                t2 = v5 + v6;
                v7 = t2 * v4;
                result += v7;
                
                /* Use many variables in computation */
                t3 = v8 + v9 + v10;
                v11 = t3 * v0;
                v12 = v11 - v1;
                break;
                
            case 1:
                /* Different computation pattern */
                t0 = v2 + v3 + v4;
                t1 = v5 * v6;
                v13 = t0 - t1;
                t2 = v7 + v8;
                v14 = t2 * v13;
                result += v14;
                
                t3 = v9 + v10 + v11;
                v15 = t3 * v2;
                v16 = v15 - v3;
                break;
                
            case 2:
                /* Yet another pattern */
                t0 = v4 + v5 + v6 + v7;
                t1 = v8 * v9 * v10;
                v17 = t0 - t1;
                t2 = v11 + v12;
                v0 = t2 * v17;  /* Reuse v0 */
                result += v0;
                
                t3 = v13 + v14 + v15;
                v1 = t3 * v4;   /* Reuse v1 */
                v2 = v1 - v5;   /* Reuse v2 */
                break;
        }
        
        MEMORY_BARRIER();
        
        /* Inner conditional to extend live ranges */
        if (i & 1) {
            /* Use another set of variables */
            t4 = v3 + v6 + v9 + v12;
            v13 = t4 * i;
            v14 = v13 + v15;
            result -= v14;
        } else {
            t4 = v1 + v4 + v7 + v10;
            v16 = t4 * i;
            v17 = v16 + v0;
            result += v17;
        }
        
        /* Cross-iteration dependencies */
        v0 = v0 + result;
        v1 = v1 - result;
        v2 = v2 * (result | 1);  /* Avoid multiplication by zero */
        
        MEMORY_BARRIER();
    }
    
    /* Final computation using all variables */
    t0 = v0 + v1 + v2 + v3;
    t1 = v4 + v5 + v6 + v7;
    t2 = v8 + v9 + v10 + v11;
    t3 = v12 + v13 + v14 + v15;
    t4 = v16 + v17;
    
    result = t0 * t1 + t2 * t3 + t4;
    
    /* Force all variables to be used */
    asm volatile("" 
                 : "+r"(result) 
                 : "r"(v0), "r"(v1), "r"(v2), "r"(v3),
                   "r"(v4), "r"(v5), "r"(v6), "r"(v7),
                   "r"(v8), "r"(v9), "r"(v10), "r"(v11),
                   "r"(v12), "r"(v13), "r"(v14), "r"(v15),
                   "r"(v16), "r"(v17)
                 : "memory");
    
    return result;
}

/* Secondary function with different control flow pattern */
NOINLINE OPT_O3
static int more_pressure(int x) {
    volatile int a = x, b = x+1, c = x+2, d = x+3;
    volatile int e = x+4, f = x+5, g = x+6, h = x+7;
    volatile int i = x+8, j = x+9, k = x+10, l = x+11;
    
    int sum = 0;
    
    /* Loop with early exit to create more edges */
    for (int n = 0; n < 8; n++) {
        if (n == 4) {
            MEMORY_BARRIER();
            /* Complex expression chain */
            a = b + c;
            d = e * f;
            g = a - d;
            h = i + j;
            k = g * h;
            sum += k;
            continue;
        }
        
        /* Default computation */
        a = a + n;
        b = b - n;
        c = c * (n | 1);
        sum += a + b + c;
        
        MEMORY_BARRIER();
        
        /* Nested if-else chain */
        if (n & 1) {
            d = d + e;
            f = f * g;
        } else if (n & 2) {
            h = h - i;
            j = j / (n | 1);
        } else {
            k = k + l;
            a = a * 2;
        }
    }
    
    return sum + a + b + c + d + e + f + g + h + i + j + k + l;
}

/* Main function - trivial, just to ensure compilation */
int main(void) {
    int result = 0;
    
    /* Call pressure functions multiple times with different seeds */
    result += create_register_pressure(42);
    result += more_pressure(100);
    result += create_register_pressure(123);
    
    return result & 0xFF;  /* Return non-zero but bounded value */
}

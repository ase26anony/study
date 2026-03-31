/* test-mcf-debug.c
 * Test program to trigger MCF_DEBUG code paths in GCC's min-cost flow solver.
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

/* Memory barrier to prevent optimization across operations */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Function to create extreme register pressure */
NOINLINE OPT_O3 static int create_register_pressure(int seed) {
    /* Declare many volatile variables to force register allocation */
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
    int t1, t2, t3, t4, t5;
    
    int result = 0;
    
    /* Complex loop structure to create control flow edges */
    for (int i = 0; i < 4; i++) {  /* Small constant for potential unrolling */
        COMPILER_BARRIER();
        
        /* Switch with multiple cases creating different live ranges */
        switch (i & 3) {
            case 0:
                /* Chain operations keeping many variables live */
                t1 = v0 + v1;
                t2 = v2 * v3;
                v4 = t1 - t2;
                v5 = v4 + v6;
                v7 = v5 * v8;
                result += v7;
                
                /* Keep more variables live across barrier */
                t3 = v9 + v10;
                v11 = t3 - v12;
                COMPILER_BARRIER();
                v13 = v11 * v14;
                v15 = v13 + v16;
                result -= v15;
                break;
                
            case 1:
                /* Different operation pattern */
                t1 = v1 - v2;
                t2 = v3 * v4;
                v5 = t1 + t2;
                v6 = v5 - v7;
                v8 = v6 * v9;
                result += v8;
                
                t3 = v10 + v11;
                v12 = t3 * v13;
                COMPILER_BARRIER();
                v14 = v12 - v15;
                v16 = v14 + v17;
                result ^= v16;
                break;
                
            case 2:
                /* More complex dependency chain */
                t1 = v2 * v3;
                t2 = v4 + v5;
                v6 = t1 - t2;
                v7 = v6 * v8;
                v9 = v7 + v10;
                result |= v9;
                
                t3 = v11 - v12;
                t4 = v13 * v14;
                COMPILER_BARRIER();
                v15 = t3 + t4;
                v16 = v15 - v17;
                v0 = v16 * v1;  /* Wrap around to v0 */
                result &= v0;
                break;
                
            case 3:
                /* Use all variables in a long chain */
                t1 = v0 + v1;
                t2 = v2 - v3;
                t3 = v4 * v5;
                t4 = v6 + v7;
                t5 = v8 - v9;
                
                v10 = t1 * t2;
                v11 = t3 - t4;
                v12 = t5 + v10;
                COMPILER_BARRIER();
                v13 = v11 * v12;
                v14 = v13 - v15;
                v16 = v14 + v17;
                v0 = v16 * v0;  /* Self-modifying */
                result = v0;
                break;
        }
        
        /* Nested conditional to extend live ranges */
        if (i & 1) {
            /* Use another subset of variables */
            t1 = v3 + v5;
            v7 = t1 * v9;
            v11 = v7 - v13;
            COMPILER_BARRIER();
            result += v11 * v15;
        } else {
            /* Alternative path */
            t1 = v2 * v4;
            v6 = t1 + v8;
            v10 = v6 - v12;
            COMPILER_BARRIER();
            result -= v10 * v14;
        }
        
        /* Small inner loop to increase complexity */
        for (int j = 0; j < 2; j++) {
            /* Mix variables across iterations */
            v0 = v0 + v1 + j;
            v2 = v2 - v3 * (j + 1);
            COMPILER_BARRIER();
            v4 = v4 ^ v5;
            v6 = v6 | v7;
        }
    }
    
    /* Final mixing of all variables */
    t1 = v0 + v1 + v2;
    t2 = v3 * v4 * v5;
    t3 = v6 - v7 + v8;
    t4 = v9 ^ v10 ^ v11;
    t5 = v12 | v13 | v14;
    
    COMPILER_BARRIER();
    
    result = result + t1 - t2 + t3 ^ t4 | t5 + v15 - v16 * v17;
    
    return result;
}

/* Secondary function with different pressure pattern */
NOINLINE OPT_O3 static int more_pressure(int x) {
    volatile int a = x * 2;
    volatile int b = x + 3;
    volatile int c = x - 4;
    volatile int d = x ^ 5;
    volatile int e = x | 6;
    volatile int f = x & 7;
    volatile int g = x << 2;
    volatile int h = x >> 1;
    
    int sum = 0;
    
    /* Loop with multiple exits to create complex CFG */
    for (int i = 0; i < 8; i++) {
        if (i == 3) {
            a = b + c;
            b = d - e;
            COMPILER_BARRIER();
            continue;
        }
        
        if (i == 5) {
            c = f * g;
            d = h + a;
            COMPILER_BARRIER();
            break;
        }
        
        a = a + i;
        b = b - i;
        c = c * (i + 1);
        COMPILER_BARRIER();
        d = d ^ i;
        e = e | i;
        f = f & i;
        
        sum += a + b + c + d + e + f + g + h;
    }
    
    return sum;
}

/* Main function that calls pressure functions */
int main(void) {
    int total = 0;
    
    /* Call multiple times to ensure compilation */
    total += create_register_pressure(42);
    total += more_pressure(100);
    
    /* Prevent dead code elimination */
    asm volatile("" : "+r" (total));
    
    return total == 0 ? 0 : 1;
}

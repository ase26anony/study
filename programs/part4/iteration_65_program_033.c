/* test_mcf_coverage.c
 * Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_coverage.c -o test.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Force noinline to prevent optimization from removing the pressure function */
#define NOINLINE __attribute__((noinline))
#define OPT_O3 __attribute__((optimize("O3")))

/* High register pressure function with complex control flow */
NOINLINE OPT_O3
static int high_pressure_function(int seed) {
    /* 18 volatile variables to force register pressure */
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
    
    /* Memory barrier to force liveness across statements */
    #define MEMORY_BARRIER asm volatile("" : : : "memory")
    
    /* Complex nested control flow with loops */
    for (int i = 0; i < 4; i++) {  /* Small constant loop for unrolling */
        /* Switch creates multiple control flow edges */
        switch (i % 5) {
            case 0:
                /* Chain of dependent operations */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 + v6;
                MEMORY_BARRIER;
                
                t2 = v7 | v8;
                v9 = t2 ^ v10;
                v11 = v9 >> 2;
                MEMORY_BARRIER;
                break;
                
            case 1:
                /* Different operation pattern */
                t0 = v12 - v13;
                t1 = v14 & v15;
                v16 = t0 | t1;
                v17 = v16 * 3;
                MEMORY_BARRIER;
                
                t2 = v1 + v3;
                t3 = v5 + v7;
                v0 = t2 * t3;
                MEMORY_BARRIER;
                break;
                
            case 2:
                /* More data flow chains */
                t0 = v2 + v4 + v6;
                t1 = v8 + v10 + v12;
                v14 = t0 * t1;
                v15 = v14 / 7;
                MEMORY_BARRIER;
                
                t2 = v3 | v5;
                t3 = v7 | v9;
                v11 = t2 & t3;
                MEMORY_BARRIER;
                break;
                
            case 3:
                /* Cross-variable dependencies */
                t0 = v0 * v2;
                t1 = v4 * v6;
                t2 = v8 * v10;
                t3 = v12 * v14;
                t4 = v16 * v17;
                
                v1 = t0 + t1;
                v3 = t2 + t3;
                v5 = t4 + seed;
                MEMORY_BARRIER;
                
                v7 = v1 - v3;
                v9 = v5 * i;
                MEMORY_BARRIER;
                break;
                
            case 4:
                /* All variables used together */
                t0 = v0 + v2 + v4 + v6 + v8;
                t1 = v10 + v12 + v14 + v16;
                v1 = t0 - t1;
                
                t2 = v3 * v5 * v7;
                t3 = v9 * v11 * v13;
                v15 = t2 + t3;
                
                v17 = v1 | v15;
                MEMORY_BARRIER;
                break;
        }
        
        /* Loop-carried dependencies */
        if (i > 0) {
            v0 = v0 + v17;
            v2 = v2 + v16;
            v4 = v4 + v15;
            MEMORY_BARRIER;
        }
        
        /* Conditional block extending live ranges */
        if (v0 > 1000) {
            v1 = v1 / 2;
            v3 = v3 / 2;
            v5 = v5 / 2;
            MEMORY_BARRIER;
        } else {
            v1 = v1 * 2;
            v3 = v3 * 2;
            v5 = v5 * 2;
            MEMORY_BARRIER;
        }
    }
    
    /* Final computation using all variables */
    t0 = v0 + v1 + v2 + v3;
    t1 = v4 + v5 + v6 + v7;
    t2 = v8 + v9 + v10 + v11;
    t3 = v12 + v13 + v14 + v15;
    t4 = v16 + v17;
    
    /* Force all volatiles to be used */
    asm volatile("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3),
                   "r"(v4), "r"(v5), "r"(v6), "r"(v7),
                   "r"(v8), "r"(v9), "r"(v10), "r"(v11),
                   "r"(v12), "r"(v13), "r"(v14), "r"(v15),
                   "r"(v16), "r"(v17) : "memory");
    
    return t0 + t1 + t2 + t3 + t4;
}

/* Secondary function with different pattern to increase graph complexity */
NOINLINE OPT_O3
static int secondary_pressure(int x) {
    volatile int a = x;
    volatile int b = x * 2;
    volatile int c = x * 3;
    volatile int d = x * 4;
    volatile int e = x * 5;
    
    int sum = 0;
    
    /* Unrolled loop with conditionals */
    for (int j = 0; j < 8; j++) {
        if (j & 1) {
            a = b + c;
            d = e - a;
        } else {
            b = c + d;
            e = a - b;
        }
        
        asm volatile("" : : : "memory");
        
        /* Nested condition */
        if (a > b) {
            c = d << 1;
        } else if (a < b) {
            c = d >> 1;
        } else {
            c = d;
        }
        
        sum += a + b + c + d + e;
    }
    
    return sum;
}

/* Main function that calls pressure functions */
int main(void) {
    int result = 0;
    
    /* Call high pressure function multiple times with different seeds */
    for (int k = 0; k < 3; k++) {
        result += high_pressure_function(k * 100);
        result += secondary_pressure(k * 50);
    }
    
    /* Prevent optimization of entire program */
    asm volatile("" : "+r"(result) : : "memory");
    
    return result != 0 ? 0 : 1;
}

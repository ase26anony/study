/* test_mcf_coverage.c
 * Designed to trigger debug output in GCC's min-cost flow solver
 * Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_coverage.c -o test.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Prevent inlining to preserve control flow structure */
#define NOINLINE __attribute__((noinline))
/* Force aggressive optimization within the function */
#define AGGRESSIVE __attribute__((optimize("O3","unroll-loops")))

/* High register pressure function */
NOINLINE AGGRESSIVE
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
    
    /* Memory barrier to force liveness across operations */
    #define MEMORY_BARRIER() asm volatile("" : : : "memory")
    
    /* Complex control flow with nested loops */
    for (int i = 0; i < 4; i++) {  /* Will likely unroll */
        /* Switch creates multiple control flow edges */
        switch (i & 3) {
            case 0:
                /* Chain dependencies to create data flow */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 ^ v6;
                MEMORY_BARRIER();
                
                t2 = v7 | v8;
                v9 = t2 + v10;
                v11 = v9 >> 2;
                break;
                
            case 1:
                /* Different variable subset */
                t3 = v12 - v13;
                v14 = t3 * v15;
                v16 = v14 & 0xFF;
                MEMORY_BARRIER();
                
                v0 = v16 + v1;  /* Reuse v0 */
                v2 = v0 * 3;
                break;
                
            case 2:
                /* More complex operations */
                t4 = v3 + v4 + v5;
                v6 = t4 - v7;
                v8 = v6 * 2;
                v17 = v8 | v9;
                MEMORY_BARRIER();
                
                v10 = v11 ^ v12;
                v13 = v10 + 1;
                break;
                
            case 3:
                /* Use all remaining variables */
                v14 = v15 + v16 + v17;
                v0 = v14 * v1;
                v2 = v0 / (v3 + 1);
                v4 = v2 | v5;
                MEMORY_BARRIER();
                
                /* Nested if-else chain */
                if (v6 > 100) {
                    v7 = v8 + v9;
                } else if (v6 > 50) {
                    v7 = v10 - v11;
                } else {
                    v7 = v12 * v13;
                }
                break;
        }
        
        /* Cross-iteration dependencies */
        if (i > 0) {
            v1 = v2 + v3;
            v4 = v5 - v6;
            MEMORY_BARRIER();
        }
        
        /* Inner loop to increase pressure */
        for (int j = 0; j < 2; j++) {
            v8 = v9 + v10 + j;
            v11 = v12 * v13 - j;
            /* Force all variables live across barrier */
            MEMORY_BARRIER();
        }
    }
    
    /* Final computation using all variables */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17;
    
    return result;
}

/* Second function with different control flow pattern */
NOINLINE AGGRESSIVE
static int alternative_pressure_function(int x) {
    volatile int a = x;
    volatile int b = x * 2;
    volatile int c = x * 3;
    volatile int d = x * 4;
    volatile int e = x * 5;
    volatile int f = x * 6;
    volatile int g = x * 7;
    volatile int h = x * 8;
    volatile int i = x * 9;
    volatile int j = x * 10;
    volatile int k = x * 11;
    volatile int l = x * 12;
    volatile int m = x * 13;
    volatile int n = x * 14;
    volatile int o = x * 15;
    volatile int p = x * 16;
    volatile int q = x * 17;
    volatile int r = x * 18;
    
    /* Complex if-else chain with overlapping live ranges */
    int temp;
    if (x > 1000) {
        temp = a + b + c;
        a = temp * d;
        b = a >> 2;
    } else if (x > 500) {
        temp = e + f + g;
        c = temp * h;
        d = c ^ 0x55;
    } else if (x > 250) {
        temp = i + j + k;
        e = temp - l;
        f = e & 0xFF;
    } else {
        temp = m + n + o;
        g = temp / 2;
        h = g | 1;
    }
    
    /* Loop with switch inside */
    for (int iter = 0; iter < 3; iter++) {
        switch (iter) {
            case 0: i = j + k; break;
            case 1: l = m - n; break;
            case 2: o = p * q; break;
        }
        asm volatile("" : : : "memory");
    }
    
    return a + b + c + d + e + f + g + h + i + j + 
           k + l + m + n + o + p + q + r;
}

/* Main function to ensure compilation */
int main(int argc, char **argv) {
    int result1 = high_pressure_function(argc);
    int result2 = alternative_pressure_function(argc + 1);
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(result1), "r"(result2));
    
    return result1 + result2;
}

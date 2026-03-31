/* test_mcf_coverage.c
 * Designed to trigger debug dumping in GCC's min-cost flow solver
 * Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_coverage.c -o test.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Prevent inlining to preserve control flow structure */
__attribute__((noinline, optimize("O3")))
int high_pressure_function(int seed) {
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
    
    /* Memory barrier to force liveness across it */
    asm volatile("" : : : "memory");
    
    /* Complex control flow with nested loops */
    for (int i = 0; i < 4; i++) {  /* Constant bound encourages unrolling */
        /* Switch creates multiple control flow edges */
        switch (i % 3) {
            case 0:
                /* Chain dependencies to create data flow */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 + v6;
                
                /* More operations keeping many vars live */
                t2 = v7 + v8;
                v9 = t2 * v10;
                v11 = v9 - v12;
                break;
                
            case 1:
                /* Different operation pattern */
                t3 = v13 + v14;
                v15 = t3 * v16;
                v0 = v15 + v17;  /* Reuse v0 */
                
                /* Cross-case dependencies */
                v1 = v2 + v3;
                v4 = v5 * v6;
                break;
                
            case 2:
                /* Third pattern with more variables */
                t4 = v7 + v8 + v9;
                v10 = t4 * v11;
                v12 = v10 - v13;
                v14 = v12 + v15;
                v16 = v14 * v17;
                
                /* Complex expression chain */
                v0 = (v1 + v2) * (v3 - v4) / (v5 + 1);
                break;
        }
        
        /* Memory barrier inside loop - forces many variables to be live across */
        asm volatile("" : : : "memory");
        
        /* More operations between barriers */
        if (i & 1) {
            v1 = v2 + v3;
            v4 = v5 * v6;
            v7 = v8 - v9;
        } else {
            v10 = v11 + v12;
            v13 = v14 * v15;
            v16 = v17 - v0;
        }
        
        /* Nested conditional */
        for (int j = 0; j < 2; j++) {
            /* Use different variable subsets */
            int temp = v0 + v1 + v2;
            v3 = temp * (j + 1);
            v4 = v3 + v5;
            
            if (j == 0) {
                v6 = v7 + v8;
                v9 = v10 * v11;
            } else {
                v12 = v13 + v14;
                v15 = v16 * v17;
            }
        }
    }
    
    /* Final computation using all variables */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17;
    
    /* One more memory barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Second function with different control flow pattern */
__attribute__((noinline, optimize("O3")))
int another_high_pressure_function(int x) {
    volatile int a = x;
    volatile int b = x * 2;
    volatile int c = x * 3;
    volatile int d = x * 4;
    volatile int e = x * 5;
    volatile int f = x * 6;
    volatile int g = x * 7;
    volatile int h = x * 8;
    
    /* Deep if-else chain */
    if (x > 0) {
        a = b + c;
        if (x > 10) {
            d = e * f;
            if (x > 20) {
                g = h + a;
            } else {
                g = h - a;
            }
        } else {
            d = e / f;
        }
    } else if (x < 0) {
        a = b - c;
        d = e % (f + 1);
    } else {
        a = b * c;
        d = e + f;
    }
    
    /* Loop with switch inside */
    for (int i = 0; i < 3; i++) {
        switch (i) {
            case 0: a = b + 1; break;
            case 1: c = d * 2; break;
            case 2: e = f - 3; break;
        }
        asm volatile("" : : : "memory");
    }
    
    return a + c + e + g;
}

/* Main function to ensure everything gets compiled */
int main() {
    int result1 = high_pressure_function(42);
    int result2 = another_high_pressure_function(-5);
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(result1), "r"(result2) : "memory");
    
    return result1 + result2;
}

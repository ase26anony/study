/* test_mcf_debug.c
 * Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_debug.c -o test_mcf_debug.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Force noinline to prevent optimization across function boundaries */
#define NOINLINE __attribute__((noinline))

/* High register pressure function with complex control flow */
NOINLINE __attribute__((optimize("O3")))
static int high_pressure_function(int seed) {
    /* 18 volatile variables to force register pressure */
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
    int t0, t1, t2, t3, t4, t5;
    
    /* Memory barrier to force liveness across statements */
    asm volatile("" : : : "memory");
    
    /* Complex nested control flow with loops */
    for (int i = 0; i < 4; i++) {  /* Constant bound encourages unrolling */
        /* Switch creates multiple control flow edges */
        switch (i & 3) {
            case 0:
                /* Chain dependencies to create data flow */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 + v6;
                /* Memory barrier keeps variables live */
                asm volatile("" : : : "memory");
                break;
                
            case 1:
                t2 = v7 ^ v8;
                t3 = v9 | v10;
                v11 = t2 & t3;
                v12 = v11 << 2;
                asm volatile("" : : : "memory");
                break;
                
            case 2:
                t4 = v13 - v14;
                t5 = v15 * v16;
                v17 = t4 + t5;
                v0 = v17 >> 1;
                asm volatile("" : : : "memory");
                break;
                
            case 3:
                /* Mix all variable groups */
                v1 = v2 + v3 + v4;
                v5 = v6 - v7 - v8;
                v9 = v10 * v11 * v12;
                asm volatile("" : : : "memory");
                break;
        }
        
        /* Cross-iteration dependencies */
        if (i > 0) {
            v13 = v0 + v1 + v2;
            v14 = v3 * v4 * v5;
            asm volatile("" : : : "memory");
        }
        
        /* Nested loop to increase pressure */
        for (int j = 0; j < 2; j++) {
            /* Use different variable subsets */
            int tmp = v6 + v7 + v8 + j;
            v9 = v9 ^ tmp;
            v10 = v10 | (v11 + j);
            asm volatile("" : : : "memory");
        }
    }
    
    /* Final complex expression using all variables */
    int result = 
        v0 + v1 - v2 + v3 * v4 +
        v5 / (v6 + 1) + v7 % (v8 + 1) +
        v9 ^ v10 | v11 & v12 +
        v13 - v14 * v15 + v16 - v17;
    
    /* Force all variables to be considered live at return */
    asm volatile("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3),
                     "r"(v4), "r"(v5), "r"(v6), "r"(v7),
                     "r"(v8), "r"(v9), "r"(v10), "r"(v11),
                     "r"(v12), "r"(v13), "r"(v14), "r"(v15),
                     "r"(v16), "r"(v17) : "memory");
    
    return result;
}

/* Second high-pressure function with different pattern */
NOINLINE __attribute__((optimize("O3")))
static int another_pressure_function(int x) {
    volatile int a = x * 2;
    volatile int b = x * 3;
    volatile int c = x * 4;
    volatile int d = x * 5;
    volatile int e = x * 6;
    volatile int f = x * 7;
    volatile int g = x * 8;
    volatile int h = x * 9;
    volatile int i = x * 10;
    volatile int j = x * 11;
    volatile int k = x * 12;
    volatile int l = x * 13;
    volatile int m = x * 14;
    volatile int n = x * 15;
    volatile int o = x * 16;
    volatile int p = x * 17;
    volatile int q = x * 18;
    volatile int r = x * 19;
    
    /* Deeply nested conditionals */
    if (x > 0) {
        if (x > 10) {
            if (x > 20) {
                a = b + c + d;
                e = f * g * h;
                asm volatile("" : : : "memory");
            } else {
                i = j - k - l;
                m = n / (o + 1);
                asm volatile("" : : : "memory");
            }
        } else {
            p = q ^ r ^ a;
            b = c | d | e;
            asm volatile("" : : : "memory");
        }
    }
    
    /* Loop with early exit */
    for (int iter = 0; iter < 8; iter++) {
        if (iter == 4) break;
        a = a + b + iter;
        c = c - d - iter;
        asm volatile("" : : : "memory");
    }
    
    return a + b + c + d + e + f + g + h + i + j + 
           k + l + m + n + o + p + q + r;
}

/* Main function that calls pressure functions */
int main(void) {
    int result = 0;
    
    /* Call first pressure function */
    result += high_pressure_function(42);
    
    /* Call second pressure function */
    result += another_pressure_function(24);
    
    /* Use result to prevent dead code elimination */
    asm volatile("" : : "r"(result) : "memory");
    
    return result != 0 ? 0 : 1;
}

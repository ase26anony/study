/* test-mcf-debug.c
 * Test case for GCC's min-cost flow solver debug output.
 * Compile with: gcc-debug -O3 -funroll-loops -c test-mcf-debug.c -o test.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Force optimization level for high_pressure_function */
__attribute__((noinline, optimize("O3")))
void high_pressure_function(int seed) {
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
    for (int i = 0; i < 4; i++) {  /* Will likely unroll */
        /* Switch creates multiple control flow edges */
        switch (i % 3) {
            case 0:
                /* Chain dependencies to create data flow */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 + v6;
                v7 = v5 * v8;
                
                /* Memory barrier between blocks */
                asm volatile("" : : : "memory");
                
                t2 = v9 + v10;
                t3 = v11 - v12;
                v13 = t2 * t3;
                v14 = v13 + v15;
                break;
                
            case 1:
                /* Different operation pattern */
                t0 = v1 - v2;
                t1 = v3 * v4;
                v6 = t0 + t1;
                v7 = v6 - v8;
                v9 = v7 * v10;
                
                asm volatile("" : : : "memory");
                
                t2 = v11 + v12;
                t3 = v13 - v14;
                v15 = t2 * t3;
                v16 = v15 + v17;
                break;
                
            case 2:
                /* Yet another pattern */
                t0 = v2 * v3;
                t1 = v4 + v5;
                v8 = t0 - t1;
                v9 = v8 * v10;
                v11 = v9 + v12;
                
                asm volatile("" : : : "memory");
                
                t2 = v13 - v14;
                t3 = v15 * v16;
                v17 = t2 + t3;
                v0 = v17 - v1;
                break;
        }
        
        /* Cross-iteration dependencies */
        if (i > 0) {
            t4 = v0 + v1 + v2;
            v3 = t4 * i;
            v4 = v3 - v5;
        }
        
        /* More arithmetic to increase live ranges */
        v5 = v6 + v7;
        v8 = v9 * v10;
        v11 = v12 - v13;
        v14 = v15 + v16;
        
        /* Force all variables to appear live */
        asm volatile(""
            : 
            : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4),
              "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9),
              "r"(v10), "r"(v11), "r"(v12), "r"(v13), "r"(v14),
              "r"(v15), "r"(v16), "r"(v17)
            : "memory");
    }
    
    /* Final computation using all variables */
    t0 = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
         v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17;
    
    /* Use the result to prevent dead code elimination */
    asm volatile("" : : "r"(t0));
}

/* Secondary function with different control flow pattern */
__attribute__((noinline, optimize("O3")))
void another_high_pressure_function(int n) {
    volatile int a0 = n, a1 = n+1, a2 = n+2, a3 = n+3, a4 = n+4;
    volatile int b0 = n+5, b1 = n+6, b2 = n+7, b3 = n+8, b4 = n+9;
    volatile int c0 = n+10, c1 = n+11, c2 = n+12, c3 = n+13, c4 = n+14;
    
    int sum = 0;
    
    /* Loop with if-else chain */
    for (int i = 0; i < 8; i++) {
        if (i & 1) {
            sum += a0 + a1 - a2 * a3 + a4;
            a0 = a1 + 1;
            a1 = a2 - 1;
        } else if (i & 2) {
            sum += b0 * b1 + b2 - b3 / (b4 + 1);
            b0 = b1 * 2;
            b1 = b2 / 2;
        } else {
            sum += c0 - c1 + c2 * c3 - c4;
            c0 = c1 + c2;
            c1 = c3 - c4;
        }
        
        /* Mix variables across branches */
        a2 = b0 + c0;
        b2 = a1 + c1;
        c2 = a0 + b1;
        
        asm volatile("" : : : "memory");
    }
    
    asm volatile("" : : "r"(sum));
}

int main() {
    /* Call high-pressure functions to ensure they're compiled */
    high_pressure_function(42);
    another_high_pressure_function(100);
    return 0;
}

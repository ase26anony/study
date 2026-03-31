/* test_mcf_coverage.c
 * Designed to trigger debug output in GCC's min-cost flow solver
 * Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_coverage.c
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

#include <stdio.h>
#include <stdlib.h>

/* High register pressure function with complex control flow */
__attribute__((noinline, optimize("O3")))
int high_pressure_function(int seed) {
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
    int t0, t1, t2, t3, t4;
    
    /* Memory barrier to force liveness across operations */
    asm volatile("" : : : "memory");
    
    /* Complex nested control flow with loops */
    for (int i = 0; i < 4; i++) {  /* Constant bound encourages unrolling */
        /* Switch creates multiple control flow edges */
        switch (i % 3) {
            case 0:
                /* Chain dependencies to create data flow */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 + v6;
                
                /* Memory barrier between blocks */
                asm volatile("" : : : "memory");
                
                t2 = v7 + v8;
                v9 = t2 * v10;
                v11 = v9 - v12;
                break;
                
            case 1:
                /* Different variable usage pattern */
                t3 = v13 + v14;
                v15 = t3 * v16;
                v0 = v15 + v17;  /* Reuse v0 */
                
                asm volatile("" : : : "memory");
                
                t4 = v1 + v2;
                v3 = t4 * v4;
                v5 = v3 + v6;
                break;
                
            case 2:
                /* Third pattern to increase complexity */
                v7 = v8 + v9;
                v10 = v7 * v11;
                v12 = v10 - v13;
                
                asm volatile("" : : : "memory");
                
                v14 = v15 + v16;
                v17 = v14 * v0;
                v1 = v17 + v2;
                break;
        }
        
        /* Cross-iteration dependencies */
        if (i > 0) {
            v2 = v3 + v4;
            v5 = v6 * v7;
        }
        
        /* Nested conditional */
        if (v8 > 100) {
            v9 = v10 - v11;
        } else {
            v12 = v13 + v14;
            /* Another memory barrier */
            asm volatile("" : : : "memory");
        }
    }
    
    /* Final computation using all variables */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17;
    
    /* Force one more memory clobber */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Secondary function with different pressure pattern */
__attribute__((noinline, optimize("O3")))
int secondary_pressure(int x) {
    volatile int a = x * 2;
    volatile int b = x + 3;
    volatile int c = x - 4;
    volatile int d = x * x;
    volatile int e = x / 2;
    
    /* Loop with if-else chain */
    for (int j = 0; j < 3; j++) {
        if (j == 0) {
            a = b + c;
            asm volatile("" : : : "memory");
        } else if (j == 1) {
            c = d - e;
            asm volatile("" : : : "memory");
        } else {
            e = a * b;
        }
        
        /* Nested loop */
        for (int k = 0; k < 2; k++) {
            d = c + k;
            asm volatile("" : : : "memory");
        }
    }
    
    return a + b + c + d + e;
}

/* Main function to ensure compilation */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Call high-pressure functions multiple times with different seeds */
    for (int i = 0; i < 3; i++) {
        result += high_pressure_function(i * 10);
        result += secondary_pressure(i * 5);
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : "+r"(result) : : "memory");
    
    return result % 256;  /* Return non-zero to be safe */
}

/* test_mcf_coverage.c
 * Designed to trigger debug output in GCC's min-cost flow solver
 * Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_coverage.c
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

#include <stdio.h>
#include <stdlib.h>

/* High register pressure function with complex control flow */
__attribute__((noinline, optimize("O3")))
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
                /* Different operation pattern */
                t3 = v13 + v14;
                v15 = t3 * v16;
                v0 = v15 + v17;  /* Reuse v0 */
                v1 = v0 - v2;    /* Chain across variables */
                
                asm volatile("" : : : "memory");
                
                v3 = v4 + v5;
                v6 = v7 * v8;
                t4 = v9 + v10;
                v11 = t4 - v12;
                break;
                
            case 2:
                /* Third pattern with cross-dependencies */
                v13 = v14 + v15;
                v16 = v13 * v17;
                v0 = v16 - v1;
                v2 = v0 + v3;
                
                asm volatile("" : : : "memory");
                
                v4 = v5 * v6;
                v7 = v8 + v9;
                v10 = v11 - v12;
                v13 = v14 * v15;
                break;
        }
        
        /* Loop-carried dependencies */
        v17 = v0 + i;
        v16 = v1 - i;
        
        /* Conditional inside loop */
        if (i & 1) {
            v15 = v2 * v3;
            v14 = v4 + v5;
        } else {
            v13 = v6 - v7;
            v12 = v8 * v9;
        }
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
    }
    
    /* Final computation using all variables */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17;
    
    /* Force use of result */
    asm volatile("" : "+r"(result) : : "memory");
    
    return result;
}

/* Secondary function with different pressure pattern */
__attribute__((noinline, optimize("O3")))
static int secondary_pressure(int x) {
    volatile int a = x * 2;
    volatile int b = x + 3;
    volatile int c = x - 4;
    volatile int d = x * x;
    volatile int e = x / 2;
    volatile int f = x % 7;
    
    /* Nested loops with different bounds */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            a = b + c;
            b = c - d;
            c = d * e;
            d = e + f;
            e = f - a;
            f = a * b;
            
            asm volatile("" : : : "memory");
        }
        
        /* Conditional with else-if chain */
        if (i == 0) {
            a = b + 1;
        } else if (i == 1) {
            b = c + 2;
        } else {
            c = d + 3;
        }
    }
    
    return a + b + c + d + e + f;
}

int main(void) {
    int result1 = high_pressure_function(42);
    int result2 = secondary_pressure(17);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    return 0;
}

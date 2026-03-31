/* test-mcf-debug.c
 * 
 * This test aims to trigger the debug dump logic in GCC's min-cost flow solver
 * (mcf.cc) for fixup graph node labeling, specifically covering the special
 * node index cases: ENTRY, EXIT, NEW_ENTRY, and NEW_EXIT.
 *
 * Compile with a GCC configured with --enable-checking (defines MCF_DEBUG):
 *   gcc-debug -O3 -funroll-loops -c test-mcf-debug.c -o test.o
 *
 * The high register pressure and complex control flow force IRA to build a
 * fixup graph requiring artificial source/sink nodes, which should be dumped
 * when MCF_DEBUG is active.
 */

#include <stdio.h>
#include <stdlib.h>

/* High-pressure function with many simultaneously live variables */
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
    
    /* Complex control flow: outer switch */
    switch (seed % 5) {
        case 0: {
            /* Chain dependencies to extend live ranges */
            t0 = v0 + v1;
            t1 = v2 * v3;
            t2 = v4 - v5;
            asm volatile("" : : : "memory"); /* Force liveness across barrier */
            v6 = t0 + t1;
            v7 = t1 - t2;
            v8 = v6 * v7;
            break;
        }
        case 1: {
            t0 = v8 + v9;
            t1 = v10 * v11;
            asm volatile("" : : : "memory");
            v12 = t0 - t1;
            v13 = v12 + v14;
            v15 = v13 * 2;
            break;
        }
        case 2: {
            t0 = v16 + v17;
            t1 = v0 * v1;
            t2 = v2 + v3;
            asm volatile("" : : : "memory");
            v4 = t0 + t1;
            v5 = t1 - t2;
            v6 = v4 * v5;
            break;
        }
        case 3: {
            t0 = v7 + v8;
            t1 = v9 * v10;
            t2 = v11 - v12;
            asm volatile("" : : : "memory");
            v13 = t0 + t1;
            v14 = t1 - t2;
            v15 = v13 * v14;
            break;
        }
        default: {
            t0 = v15 + v16;
            t1 = v17 * v0;
            asm volatile("" : : : "memory");
            v1 = t0 - t1;
            v2 = v1 + v3;
            v4 = v2 * 3;
            break;
        }
    }
    
    /* Nested loops to encourage unrolling and extend live ranges */
    for (int i = 0; i < 4; i++) {
        /* Different variable usage in each iteration */
        t0 = v0 + v1 + i;
        t1 = v2 * v3 - i;
        t2 = v4 + v5 + t0;
        t3 = v6 * v7 - t1;
        t4 = v8 + v9 + t2;
        
        asm volatile("" : : : "memory"); /* Barrier within loop */
        
        /* Conditional inside loop */
        if (t0 > t1) {
            v10 = t2 + t3;
            v11 = t3 - t4;
            v12 = v10 * v11;
        } else {
            v13 = t2 - t3;
            v14 = t3 + t4;
            v15 = v13 * v14;
        }
        
        /* More arithmetic chains */
        v16 = v12 + v15;
        v17 = v16 * (i + 1);
        
        /* Another conditional with different variable sets */
        if (i % 2 == 0) {
            v0 = v1 + v17;
            v1 = v2 * v16;
        } else {
            v2 = v3 + v17;
            v3 = v4 * v16;
        }
    }
    
    /* Final complex expression using all variables */
    t0 = v0 + v1 + v2 + v3 + v4 + v5;
    t1 = v6 * v7 * v8 * v9 * v10;
    t2 = v11 + v12 + v13 + v14 + v15;
    t3 = v16 * v17 * seed;
    
    asm volatile("" : : : "memory");
    
    /* Use results to prevent dead code elimination */
    volatile int result = t0 + t1 + t2 + t3;
    (void)result; /* Suppress unused warning */
}

/* Secondary function with different pressure pattern */
__attribute__((noinline, optimize("O3")))
void secondary_pressure_function(int iter) {
    volatile int w0 = iter;
    volatile int w1 = iter * 2;
    volatile int w2 = iter * 3;
    volatile int w3 = iter * 4;
    volatile int w4 = iter * 5;
    volatile int w5 = iter * 6;
    volatile int w6 = iter * 7;
    volatile int w7 = iter * 8;
    volatile int w8 = iter * 9;
    volatile int w9 = iter * 10;
    
    int u0, u1, u2;
    
    /* Loop with switch inside */
    for (int j = 0; j < 3; j++) {
        switch ((iter + j) % 4) {
            case 0:
                u0 = w0 + w1;
                u1 = w2 * w3;
                asm volatile("" : : : "memory");
                w4 = u0 + u1;
                w5 = u0 - u1;
                break;
            case 1:
                u0 = w4 + w5;
                u1 = w6 * w7;
                asm volatile("" : : : "memory");
                w8 = u0 + u1;
                w9 = u0 - u1;
                break;
            case 2:
                u0 = w8 + w9;
                u1 = w0 * w1;
                asm volatile("" : : : "memory");
                w2 = u0 + u1;
                w3 = u0 - u1;
                break;
            default:
                u0 = w2 + w3;
                u1 = w4 * w5;
                asm volatile("" : : : "memory");
                w6 = u0 + u1;
                w7 = u0 - u1;
                break;
        }
        
        /* Cross-iteration dependencies */
        w0 = w0 + w6 + j;
        w1 = w1 * w7 + j;
    }
}

int main() {
    /* Call high-pressure functions multiple times with different seeds */
    for (int s = 0; s < 10; s++) {
        high_pressure_function(s);
        secondary_pressure_function(s);
    }
    
    return 0;
}

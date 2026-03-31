/* Complex CFG generator to trigger MCF fixup graph debug output */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
__attribute__((noinline, optimize("no-goto")))
static unsigned long test_irreducible_goto(int iterations, int seed) {
    /* Many local variables to create register pressure */
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
    volatile float f0 = seed * 1.1f;
    volatile float f1 = seed * 1.2f;
    volatile float f2 = seed * 1.3f;
    volatile float f3 = seed * 1.4f;
    volatile double d0 = seed * 2.1;
    volatile double d1 = seed * 2.2;
    volatile double d2 = seed * 2.3;
    volatile double d3 = seed * 2.4;
    volatile long l0 = seed * 3L;
    volatile long l1 = seed * 4L;
    volatile long l2 = seed * 5L;
    volatile long l3 = seed * 6L;
    volatile long l4 = seed * 7L;
    volatile long l5 = seed * 8L;
    volatile long l6 = seed * 9L;
    volatile long l7 = seed * 10L;
    volatile long l8 = seed * 11L;
    volatile long l9 = seed * 12L;
    
    unsigned long checksum = 0;
    int i;
    
    /* Irreducible loop with goto jumping across boundaries */
    for (i = 0; i < iterations; i++) {
        /* Create arithmetic chains to keep variables live */
        v0 = v1 + v2;
        v1 = v2 + v3;
        v2 = v3 + v4;
        v3 = v4 + v5;
        v4 = v5 + v6;
        v5 = v6 + v7;
        v6 = v7 + v8;
        v7 = v8 + v9;
        v8 = v9 + v0;
        v9 = v0 + v1;
        
        f0 = f1 + f2;
        f1 = f2 + f3;
        f2 = f3 + f0;
        f3 = f0 + f1;
        
        d0 = d1 + d2;
        d1 = d2 + d3;
        d2 = d3 + d0;
        d3 = d0 + d1;
        
        l0 = l1 + l2;
        l1 = l2 + l3;
        l2 = l3 + l4;
        l3 = l4 + l5;
        l4 = l5 + l6;
        l5 = l6 + l7;
        l6 = l7 + l8;
        l7 = l8 + l9;
        l8 = l9 + l0;
        l9 = l0 + l1;
        
        /* Force variable usage to prevent optimization */
        asm volatile("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3));
        asm volatile("" : : "r"(v4), "r"(v5), "r"(v6), "r"(v7));
        asm volatile("" : : "r"(v8), "r"(v9));
        asm volatile("" : : "r"(f0), "r"(f1), "r"(f2), "r"(f3));
        asm volatile("" : : "r"(d0), "r"(d1), "r"(d2), "r"(d3));
        asm volatile("" : : "r"(l0), "r"(l1), "r"(l2), "r"(l3));
        asm volatile("" : : "r"(l4), "r"(l5), "r"(l6), "r"(l7));
        asm volatile("" : : "r"(l8), "r"(l9));
        
        /* Complex irreducible control flow */
        if (i % 7 == 0) {
            goto inner_loop_1;
        } else if (i % 11 == 0) {
            goto inner_loop_2;
        }
        
        continue;
        
    inner_loop_1:
        /* More arithmetic to create register pressure */
        v0 = v0 * 2 - v1;
        v1 = v1 * 2 - v2;
        v2 = v2 * 2 - v3;
        v3 = v3 * 2 - v4;
        
        if (i % 3 == 0) {
            goto outer_loop;
        } else {
            goto inner_loop_2;
        }
        
    inner_loop_2:
        v4 = v4 * 2 - v5;
        v5 = v5 * 2 - v6;
        v6 = v6 * 2 - v7;
        v7 = v7 * 2 - v8;
        
        if (i % 5 == 0) {
            goto inner_loop_1;
        }
        
    outer_loop:
        checksum += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
        checksum += (unsigned long)(f0 + f1 + f2 + f3);
        checksum += (unsigned long)(d0 + d1 + d2 + d3);
        checksum += l0 + l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9;
    }
    
    return checksum;
}

__attribute__((noinline))
static unsigned long test_switch_goto(int iterations, int seed) {
    /* Another set of many variables */
    volatile int a0 = seed, a1 = seed+1, a2 = seed+2, a3 = seed+3, a4 = seed+4;
    volatile int a5 = seed+5, a6 = seed+6, a7 = seed+7, a8 = seed+8, a9 = seed+9;
    volatile int b0 = seed+10, b1 = seed+11, b2 = seed+12, b3 = seed+13, b4 = seed+14;
    volatile int b5 = seed+15, b6 = seed+16, b7 = seed+17, b8 = seed+18, b9 = seed+19;
    volatile float c0 = seed*0.1f, c1 = seed*0.2f, c2 = seed*0.3f, c3 = seed*0.4f;
    volatile double d0 = seed*0.5, d1 = seed*0.6, d2 = seed*0.7, d3 = seed*0.8;
    
    unsigned long checksum = 0;
    int i, state = 0;
    
    for (i = 0; i < iterations; i++) {
        /* Long dependency chains */
        a0 = a1 + a2;
        a1 = a2 + a3;
        a2 = a3 + a4;
        a3 = a4 + a5;
        a4 = a5 + a6;
        a5 = a6 + a7;
        a6 = a7 + a8;
        a7 = a8 + a9;
        a8 = a9 + b0;
        a9 = b0 + b1;
        
        b0 = b1 + b2;
        b1 = b2 + b3;
        b2 = b3 + b4;
        b3 = b4 + b5;
        b4 = b5 + b6;
        b5 = b6 + b7;
        b6 = b7 + b8;
        b7 = b8 + b9;
        b8 = b9 + a0;
        b9 = a0 + a1;
        
        c0 = c1 + c2;
        c1 = c2 + c3;
        c2 = c3 + c0;
        c3 = c0 + c1;
        
        d0 = d1 + d2;
        d1 = d2 + d3;
        d2 = d3 + d0;
        d3 = d0 + d1;
        
        /* Force register usage */
        asm volatile("" : : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4));
        asm volatile("" : : "r"(a5), "r"(a6), "r"(a7), "r"(a8), "r"(a9));
        asm volatile("" : : "r"(b0), "r"(b1), "r"(b2), "r"(b3), "r"(b4));
        asm volatile("" : : "r"(b5), "r"(b6), "r"(b7), "r"(b8), "r"(b9));
        asm volatile("" : : "r"(c0), "r"(c1), "r"(c2), "r"(c3));
        asm volatile("" : : "r"(d0), "r"(d1), "r"(d2), "r"(d3));
        
        /* Switch with goto creating irreducible regions */
        state = (state + i) % 8;
        switch (state) {
            case 0:
                a0 = a0 * 3;
                goto label_x;
            case 1:
                a1 = a1 * 3;
                goto label_y;
            case 2:
                a2 = a2 * 3;
                goto label_z;
            case 3:
                a3 = a3 * 3;
                goto label_x;
            case 4:
                a4 = a4 * 3;
                goto label_y;
            case 5:
                a5 = a5 * 3;
                goto label_z;
            case 6:
                a6 = a6 * 3;
                goto label_x;
            case 7:
                a7 = a7 * 3;
                goto label_y;
        }
        
        /* These labels create multiple entry points to the switch */
        label_x:
            b0 = b0 * 2;
            if (i % 2 == 0) goto continue_loop;
            /* fall through */
        label_y:
            b1 = b1 * 2;
            if (i % 3 == 0) goto label_z;
            /* fall through */
        label_z:
            b2 = b2 * 2;
        
        continue_loop:
            checksum += a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9;
            checksum += b0 + b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9;
            checksum += (unsigned long)(c0 + c1 + c2 + c3);
            checksum += (unsigned long)(d0 + d1 + d2 + d3);
    }
    
    return checksum;
}

__attribute__((noinline))
static unsigned long test_computed_goto(int iterations, int seed) {
    /* Labels for computed goto */
    static void* labels[] = {
        &&state_a, &&state_b, &&state_c, 
        &&state_d, &&state_e, &&state_f
    };
    
    /* Yet another set of variables */
    volatile long x0 = seed, x1 = seed+1, x2 = seed+2, x3 = seed+3, x4 = seed+4;
    volatile long x5 = seed+5, x6 = seed+6, x7 = seed+7, x8 = seed+8, x9 = seed+9;
    volatile long y0 = seed+10, y1 = seed+11, y2 = seed+12, y3 = seed+13, y4 = seed+14;
    volatile long y5 = seed+15, y6 = seed+16, y7 = seed+17, y8 = seed+18, y9 = seed+19;
    volatile double z0 = seed*1.5, z1 = seed*2.5, z2 = seed*3.5, z3 = seed*4.5;
    
    unsigned long checksum = 0;
    int i, state = 0;
    
    for (i = 0; i < iterations; i++) {
        /* Complex arithmetic chains */
        x0 = x1 + x2;
        x1 = x2 + x3;
        x2 = x3 + x4;
        x3 = x4 + x5;
        x4 = x5 + x6;
        x5 = x6 + x7;
        x6 = x7 + x8;
        x7 = x8 + x9;
        x8 = x9 + y0;
        x9 = y0 + y1;
        
        y0 = y1 + y2;
        y1 = y2 + y3;
        y2 = y3 + y4;
        y3 = y4 + y5;
        y4 = y5 + y6;
        y5 = y6 + y7;
        y6 = y7 + y8;
        y7 = y8 + y9;
        y8 = y9 + x0;
        y9 = x0 + x1;
        
        z0 = z1 + z2;
        z1 = z2 + z3;
        z2 = z3 + z0;
        z3 = z0 + z1;
        
        /* Force register usage */
        asm volatile("" : : "r"(x0), "r"(x1), "r"(x2), "r"(x3), "r"(x4));
        asm volatile("" : : "r"(x5), "r"(x6), "r"(x7), "r"(x8), "r"(x9));
        asm volatile("" : : "r"(y0), "r"(y1), "r"(y2), "r"(y3), "r"(y4));
        asm volatile("" : : "r"(y5), "r"(y6), "r"(y7), "r"(y8), "r"(y9));
        asm volatile("" : : "r"(z0), "r"(z1), "r"(z2), "r"(z3));
        
        /* Computed goto state machine */
        state = (state + i) % 6;
        goto *labels[state];
        
    state_a:
        x0 = x0 * 2;
        y0 = y0 * 2;
        goto state_common;
        
    state_b:
        x1 = x1 * 2;
        y1 = y1 * 2;
        goto state_common;
        
    state_c:
        x2 = x2 * 2;
        y2 = y2 * 2;
        goto state_common;
        
    state_d:
        x3 = x3 * 2;
        y3 = y3 * 2;
        goto state_common;
        
    state_e:
        x4 = x4 * 2;
        y4 = y4 * 2;
        goto state_common;
        
    state_f:
        x5 = x5 * 2;
        y5 = y5 * 2;
        /* fall through */
        
    state_common:
        checksum += x0 + x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9;
        checksum += y0 + y1 + y2 + y3 + y4 + y5 + y6 + y7 + y8 + y9;
        checksum += (unsigned long)(z0 + z1 + z2 + z3);
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int iterations = 10000;
    int seed = 42;
    unsigned long total_checksum = 0;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10000;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Running MCF stress test with %d iterations, seed=%d\n", 
           iterations, seed);
    
    /* Run all test functions to increase coverage chance */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    printf("Final checksum: %lu\n", total_checksum);
    
    return 0;
}

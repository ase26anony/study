/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow algorithm fixup graph debug output */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_VARS 30
#define MAX_ITER 10000

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* Complex irreducible control flow with high register pressure */
NOINLINE static unsigned long test_irreducible_goto(int iterations, int seed) {
    /* Declare many variables to create register pressure */
    VOLATILE_VAR int v0 = seed + 1;
    VOLATILE_VAR int v1 = seed + 2;
    VOLATILE_VAR int v2 = seed + 3;
    VOLATILE_VAR int v3 = seed + 4;
    VOLATILE_VAR int v4 = seed + 5;
    VOLATILE_VAR int v5 = seed + 6;
    VOLATILE_VAR int v6 = seed + 7;
    VOLATILE_VAR int v7 = seed + 8;
    VOLATILE_VAR int v8 = seed + 9;
    VOLATILE_VAR int v9 = seed + 10;
    VOLATILE_VAR float f0 = seed * 1.1f;
    VOLATILE_VAR float f1 = seed * 1.2f;
    VOLATILE_VAR float f2 = seed * 1.3f;
    VOLATILE_VAR float f3 = seed * 1.4f;
    VOLATILE_VAR double d0 = seed * 2.1;
    VOLATILE_VAR double d1 = seed * 2.2;
    VOLATILE_VAR double d2 = seed * 2.3;
    VOLATILE_VAR double d3 = seed * 2.4;
    VOLATILE_VAR long l0 = seed * 3L;
    VOLATILE_VAR long l1 = seed * 4L;
    VOLATILE_VAR long l2 = seed * 5L;
    VOLATILE_VAR long l3 = seed * 6L;
    VOLATILE_VAR long l4 = seed * 7L;
    VOLATILE_VAR long l5 = seed * 8L;
    VOLATILE_VAR long l6 = seed * 9L;
    VOLATILE_VAR long l7 = seed * 10L;
    VOLATILE_VAR long l8 = seed * 11L;
    VOLATILE_VAR long l9 = seed * 12L;
    
    unsigned long checksum = 0;
    int i;
    
    /* Create irreducible loop with goto jumping across loop boundaries */
    for (i = 0; i < iterations; i++) {
        /* Label definitions for goto targets */
        loop_start:
        if (i % 3 == 0) {
            /* Jump into middle of another logical loop */
            goto middle_inner;
        }
        
        outer_block:
        /* Complex arithmetic to keep variables live */
        v0 = v1 + v2 * v3 - v4;
        v1 = v5 ^ v6 | v7 & v8;
        v2 = v9 * v0 / (v1 + 1);
        f0 = f1 * f2 + f3;
        f1 = f2 - f3 * f0;
        d0 = d1 + d2 * d3;
        d1 = d2 - d3 / (d0 + 1.0);
        l0 = l1 + l2 - l3;
        l1 = l4 * l5 / (l6 + 1);
        
        if (i % 5 == 0) {
            goto inner_loop;
        }
        
        continue_outer:
        v3 = v4 + v5 - v6;
        v4 = v7 * v8 / (v9 + 1);
        f2 = f3 + f0 * f1;
        f3 = f0 - f1 / (f2 + 1.0f);
        d2 = d3 + d0 * d1;
        d3 = d0 - d1 / (d2 + 1.0);
        l2 = l3 + l4 - l5;
        l3 = l6 * l7 / (l8 + 1);
        
        if (i % 7 == 0) {
            goto loop_end;
        }
        
        middle_inner:
        v5 = v6 + v7 - v8;
        v6 = v9 * v0 / (v1 + 1);
        f0 = f1 + f2 * f3;  /* Reuse variables */
        d0 = d1 + d2 * d3;
        l4 = l5 + l6 - l7;
        
        if (i % 2 == 0) {
            goto outer_block;
        }
        
        inner_loop:
        v7 = v8 + v9 - v0;
        v8 = v1 * v2 / (v3 + 1);
        f1 = f2 + f3 * f0;
        d1 = d2 + d3 * d0;
        l5 = l6 + l7 - l8;
        
        if (i % 11 == 0) {
            goto continue_outer;
        }
        
        loop_end:
        /* More arithmetic operations */
        v9 = v0 + v1 - v2;
        v0 = v3 * v4 / (v5 + 1);
        f2 = f3 + f0 * f1;
        d2 = d3 + d0 * d1;
        l6 = l7 + l8 - l9;
        
        /* Use inline assembly to mark variables as used */
        asm volatile("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4));
        asm volatile("" : : "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9));
        asm volatile("" : : "r"(f0), "r"(f1), "r"(f2), "r"(f3));
        asm volatile("" : : "r"(d0), "r"(d1), "r"(d2), "r"(d3));
        asm volatile("" : : "r"(l0), "r"(l1), "r"(l2), "r"(l3));
        asm volatile("" : : "r"(l4), "r"(l5), "r"(l6), "r"(l7), "r"(l8), "r"(l9));
        
        checksum += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
        checksum += (unsigned long)f0 + (unsigned long)f1 + (unsigned long)f2 + (unsigned long)f3;
        checksum += (unsigned long)d0 + (unsigned long)d1 + (unsigned long)d2 + (unsigned long)d3;
        checksum += l0 + l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9;
    }
    
    return checksum;
}

NOINLINE static unsigned long test_switch_goto(int iterations, int seed) {
    VOLATILE_VAR int a0 = seed, a1 = seed+1, a2 = seed+2, a3 = seed+3, a4 = seed+4;
    VOLATILE_VAR int b0 = seed+5, b1 = seed+6, b2 = seed+7, b3 = seed+8, b4 = seed+9;
    VOLATILE_VAR float fa0 = seed*1.5f, fa1 = seed*1.6f, fa2 = seed*1.7f;
    VOLATILE_VAR double da0 = seed*2.5, da1 = seed*2.6, da2 = seed*2.7;
    VOLATILE_VAR long la0 = seed*100, la1 = seed*101, la2 = seed*102, la3 = seed*103;
    
    unsigned long checksum = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Switch with goto creating irreducible flow */
        switch (i % 8) {
            case 0:
                a0 = a1 + a2;
                a1 = a3 * a4;
                goto label_b;
            case 1:
                a2 = a3 - a4;
                a3 = a0 / (a1 + 1);
                goto label_c;
            case 2:
                a4 = a0 + a1;
                a0 = a2 * a3;
                goto label_d;
            case 3:
                b0 = b1 ^ b2;
                b1 = b3 | b4;
                goto label_a;
            case 4:
                b2 = b3 + b4;
                b3 = b0 - b1;
                goto label_e;
            case 5:
                b4 = b0 * b1;
                b0 = b2 / (b3 + 1);
                goto label_b;
            case 6:
                fa0 = fa1 + fa2;
                fa1 = fa0 * 1.1f;
                goto label_d;
            case 7:
                fa2 = fa0 - fa1;
                fa0 = fa2 / 2.0f;
                goto label_c;
        }
        
        label_a:
        da0 = da1 + da2;
        da1 = da0 * 1.5;
        goto continue_main;
        
        label_b:
        da2 = da0 - da1;
        da0 = da2 / 3.0;
        goto continue_main;
        
        label_c:
        la0 = la1 + la2;
        la1 = la3 * 2;
        goto continue_main;
        
        label_d:
        la2 = la3 - la0;
        la3 = la1 / 2;
        goto continue_main;
        
        label_e:
        /* Complex arithmetic chain */
        a0 = a1 + a2 - a3 * a4 / (b0 + 1);
        a1 = a2 * a3 - a4 + b1;
        a2 = a3 / (a0 + 1) + b2 - b3;
        a3 = a4 ^ b4 | b0 & b1;
        a4 = b2 + b3 - b4 * a0;
        
        continue_main:
        /* More operations to keep variables live */
        b0 = b1 + b2 - b3 * b4;
        b1 = b2 * b3 / (b4 + 1);
        b2 = b3 ^ b4 | a0 & a1;
        b3 = b4 + a2 - a3 * a4;
        b4 = a0 * a1 / (a2 + 1);
        
        fa0 = fa1 + fa2 * 1.1f;
        fa1 = fa2 - fa0 / 2.0f;
        fa2 = fa0 * fa1 + 3.0f;
        
        da0 = da1 + da2 * 1.5;
        da1 = da2 - da0 / 3.0;
        da2 = da0 * da1 + 4.0;
        
        la0 = la1 + la2 - la3;
        la1 = la2 * la3 / (la0 + 1);
        la2 = la3 + la0 - la1;
        la3 = la0 * la1 / (la2 + 1);
        
        /* Force variable usage */
        asm volatile("" : : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4));
        asm volatile("" : : "r"(b0), "r"(b1), "r"(b2), "r"(b3), "r"(b4));
        asm volatile("" : : "r"(fa0), "r"(fa1), "r"(fa2));
        asm volatile("" : : "r"(da0), "r"(da1), "r"(da2));
        asm volatile("" : : "r"(la0), "r"(la1), "r"(la2), "r"(la3));
        
        checksum += a0 + a1 + a2 + a3 + a4 + b0 + b1 + b2 + b3 + b4;
        checksum += (unsigned long)fa0 + (unsigned long)fa1 + (unsigned long)fa2;
        checksum += (unsigned long)da0 + (unsigned long)da1 + (unsigned long)da2;
        checksum += la0 + la1 + la2 + la3;
    }
    
    return checksum;
}

NOINLINE static unsigned long test_computed_goto(int iterations, int seed) {
    /* State machine using computed goto */
    static void* labels[] = {
        &&state0, &&state1, &&state2, &&state3, 
        &&state4, &&state5, &&state6, &&state7
    };
    
    VOLATILE_VAR int x0 = seed, x1 = seed+10, x2 = seed+20, x3 = seed+30;
    VOLATILE_VAR int y0 = seed+40, y1 = seed+50, y2 = seed+60, y3 = seed+70;
    VOLATILE_VAR float fx0 = seed*0.1f, fx1 = seed*0.2f, fx2 = seed*0.3f;
    VOLATILE_VAR double dx0 = seed*0.4, dx1 = seed*0.5, dx2 = seed*0.6;
    VOLATILE_VAR long lx0 = seed*1000, lx1 = seed*1001, lx2 = seed*1002;
    
    unsigned long checksum = 0;
    int state = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Jump to current state */
        goto *labels[state];
        
        state0:
        x0 = x1 + x2 - x3;
        x1 = y0 * y1 / (y2 + 1);
        fx0 = fx1 + fx2;
        state = (i % 3 == 0) ? 1 : 4;
        goto end_state;
        
        state1:
        x2 = x3 + x0 - x1;
        x3 = y2 * y3 / (y0 + 1);
        fx1 = fx2 - fx0;
        state = (i % 5 == 0) ? 2 : 5;
        goto end_state;
        
        state2:
        y0 = y1 + y2 - y3;
        y1 = x0 * x1 / (x2 + 1);
        fx2 = fx0 * fx1;
        state = (i % 7 == 0) ? 3 : 6;
        goto end_state;
        
        state3:
        y2 = y3 + y0 - y1;
        y3 = x2 * x3 / (x0 + 1);
        dx0 = dx1 + dx2;
        state = (i % 2 == 0) ? 4 : 7;
        goto end_state;
        
        state4:
        dx1 = dx2 - dx0;
        lx0 = lx1 + lx2;
        state = (i % 11 == 0) ? 5 : 0;
        goto end_state;
        
        state5:
        dx2 = dx0 * dx1;
        lx1 = lx2 - lx0;
        state = (i % 13 == 0) ? 6 : 1;
        goto end_state;
        
        state6:
        lx2 = lx0 * lx1;
        state = (i % 17 == 0) ? 7 : 2;
        goto end_state;
        
        state7:
        /* Complex chain */
        x0 = x1 * x2 - x3 + y0;
        x1 = x2 / (x3 + 1) + y1 - y2;
        x2 = x3 ^ y3 | y0 & y1;
        x3 = y2 + y3 - x0 * x1;
        y0 = y1 * y2 / (y3 + 1);
        y1 = y2 ^ y3 | x0 & x1;
        y2 = y3 + x2 - x3 * y0;
        y3 = x1 * x2 / (x3 + 1);
        state = (i % 19 == 0) ? 0 : 3;
        /* fall through */
        
        end_state:
        /* Additional arithmetic to increase register pressure */
        fx0 = fx1 + fx2 * 1.5f;
        fx1 = fx2 - fx0 / 2.0f;
        fx2 = fx0 * fx1 + 3.0f;
        
        dx0 = dx1 + dx2 * 2.0;
        dx1 = dx2 - dx0 / 3.0;
        dx2 = dx0 * dx1 + 4.0;
        
        lx0 = lx1 + lx2 - 100;
        lx1 = lx2 * 2 - lx0;
        lx2 = lx0 + lx1 * 3;
        
        /* Force all variables to be considered live */
        asm volatile("" : : "r"(x0), "r"(x1), "r"(x2), "r"(x3));
        asm volatile("" : : "r"(y0), "r"(y1), "r"(y2), "r"(y3));
        asm volatile("" : : "r"(fx0), "r"(fx1), "r"(fx2));
        asm volatile("" : : "r"(dx0), "r"(dx1), "r"(dx2));
        asm volatile("" : : "r"(lx0), "r"(lx1), "r"(lx2));
        
        checksum += x0 + x1 + x2 + x3 + y0 + y1 + y2 + y3;
        checksum += (unsigned long)fx0 + (unsigned long)fx1 + (unsigned long)fx2;
        checksum += (unsigned long)dx0 + (unsigned long)dx1 + (unsigned long)dx2;
        checksum += lx0 + lx1 + lx2;
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int iterations = MAX_ITER;
    int seed = 42;
    unsigned long total_checksum = 0;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = MAX_ITER;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Running MCF coverage test with %d iterations, seed=%d\n", 
           iterations, seed);
    
    /* Call all test functions to increase coverage chance */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    printf("Total checksum: %lu\n", total_checksum);
    
    return 0;
}

/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow algorithm
   with fixup graph creation and debugging output */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_VARS 30
#define MAX_ITER 10000

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* Complex irreducible control flow with goto */
NOINLINE static unsigned long test_irreducible_goto(int iterations, int seed) {
    VOLATILE_VAR int i, j;
    VOLATILE_VAR unsigned long checksum = seed;
    
    /* Many local variables to create register pressure */
    VOLATILE_VAR int v0 = seed, v1 = seed + 1, v2 = seed + 2, v3 = seed + 3;
    VOLATILE_VAR int v4 = seed + 4, v5 = seed + 5, v6 = seed + 6, v7 = seed + 7;
    VOLATILE_VAR int v8 = seed + 8, v9 = seed + 9, v10 = seed + 10, v11 = seed + 11;
    VOLATILE_VAR float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f;
    VOLATILE_VAR float f3 = seed * 0.4f, f4 = seed * 0.5f, f5 = seed * 0.6f;
    VOLATILE_VAR double d0 = seed * 0.01, d1 = seed * 0.02, d2 = seed * 0.03;
    VOLATILE_VAR double d3 = seed * 0.04, d4 = seed * 0.05, d5 = seed * 0.06;
    VOLATILE_VAR long l0 = seed * 100L, l1 = seed * 200L, l2 = seed * 300L;
    VOLATILE_VAR long l3 = seed * 400L, l4 = seed * 500L, l5 = seed * 600L;
    
    /* Labels for irreducible goto pattern */
    loop_start:
    for (i = 0; i < iterations; i++) {
        /* Complex arithmetic chains to keep variables live */
        v0 = v1 + v2 * v3 - v4;
        v1 = v2 + v3 * v4 - v5;
        v2 = v3 + v4 * v5 - v6;
        v3 = v4 + v5 * v6 - v7;
        v4 = v5 + v6 * v7 - v8;
        v5 = v6 + v7 * v8 - v9;
        
        f0 = f1 * 1.1f + f2 - f3;
        f1 = f2 * 1.2f + f3 - f4;
        f2 = f3 * 1.3f + f4 - f5;
        
        d0 = d1 * 1.01 + d2 - d3;
        d1 = d2 * 1.02 + d3 - d4;
        d2 = d3 * 1.03 + d4 - d5;
        
        l0 = l1 + l2 * 3 - l3;
        l1 = l2 + l3 * 4 - l4;
        l2 = l3 + l4 * 5 - l5;
        
        /* Irreducible loop with goto jumping across boundaries */
        if ((i & 3) == 0) {
            goto inner_block_a;
        } else if ((i & 3) == 1) {
            goto inner_block_b;
        } else if ((i & 3) == 2) {
            goto inner_block_c;
        } else {
            goto inner_block_d;
        }
        
        inner_block_a:
        v6 = v7 + v8 * v9 - v10;
        v7 = v8 + v9 * v10 - v11;
        f3 = f4 * 1.4f + f5 - f0;
        d3 = d4 * 1.04 + d5 - d0;
        l3 = l4 + l5 * 6 - l0;
        if ((i & 1) == 0) goto loop_end;
        else goto inner_block_b;
        
        inner_block_b:
        v8 = v9 + v10 * v11 - v0;
        v9 = v10 + v11 * v0 - v1;
        f4 = f5 * 1.5f + f0 - f1;
        d4 = d5 * 1.05 + d0 - d1;
        l4 = l5 + l0 * 7 - l1;
        if ((i & 2) == 0) goto inner_block_c;
        else goto inner_block_d;
        
        inner_block_c:
        v10 = v11 + v0 * v1 - v2;
        v11 = v0 + v1 * v2 - v3;
        f5 = f0 * 1.6f + f1 - f2;
        d5 = d0 * 1.06 + d1 - d2;
        l5 = l0 + l1 * 8 - l2;
        if ((i & 4) == 0) goto inner_block_d;
        else goto inner_block_a;
        
        inner_block_d:
        /* More arithmetic to prevent dead code elimination */
        v0 = v1 ^ v2 | v3 & v4;
        v1 = v2 ^ v3 | v4 & v5;
        f0 = f1 + f2 * f3 / 2.0f;
        d0 = d1 + d2 * d3 / 2.0;
        l0 = l1 | l2 & l3 ^ l4;
        if ((i & 8) == 0) goto loop_end;
        else goto inner_block_a;
        
        loop_end:
        /* Force variables to be considered live */
        asm volatile("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5));
        asm volatile("" : : "r"(v6), "r"(v7), "r"(v8), "r"(v9), "r"(v10), "r"(v11));
        asm volatile("" : : "r"(f0), "r"(f1), "r"(f2), "r"(f3), "r"(f4), "r"(f5));
        asm volatile("" : : "r"(d0), "r"(d1), "r"(d2), "r"(d3), "r"(d4), "r"(d5));
        asm volatile("" : : "r"(l0), "r"(l1), "r"(l2), "r"(l3), "r"(l4), "r"(l5));
        
        checksum += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11;
        checksum += (unsigned long)(f0 + f1 + f2 + f3 + f4 + f5);
        checksum += (unsigned long)(d0 + d1 + d2 + d3 + d4 + d5);
        checksum += l0 + l1 + l2 + l3 + l4 + l5;
    }
    
    return checksum;
}

/* Switch with goto creating irreducible regions */
NOINLINE static unsigned long test_switch_goto(int iterations, int seed) {
    VOLATILE_VAR int i;
    VOLATILE_VAR unsigned long checksum = seed;
    
    /* Another set of many variables */
    VOLATILE_VAR int a0 = seed, a1 = seed * 2, a2 = seed * 3, a3 = seed * 4;
    VOLATILE_VAR int a4 = seed * 5, a5 = seed * 6, a6 = seed * 7, a7 = seed * 8;
    VOLATILE_VAR int a8 = seed * 9, a9 = seed * 10, a10 = seed * 11, a11 = seed * 12;
    VOLATILE_VAR float fa0 = seed * 0.11f, fa1 = seed * 0.22f, fa2 = seed * 0.33f;
    VOLATILE_VAR float fa3 = seed * 0.44f, fa4 = seed * 0.55f, fa5 = seed * 0.66f;
    VOLATILE_VAR double da0 = seed * 0.011, da1 = seed * 0.022, da2 = seed * 0.033;
    VOLATILE_VAR double da3 = seed * 0.044, da4 = seed * 0.055, da5 = seed * 0.066;
    
    /* Labels for switch goto */
    switch_start:
    switch_middle:
    switch_end_label:
    
    for (i = 0; i < iterations; i++) {
        int selector = (i * seed) % 10;
        
        /* Arithmetic before switch */
        a0 = a1 + a2 - a3 * a4;
        a1 = a2 + a3 - a4 * a5;
        a2 = a3 + a4 - a5 * a6;
        fa0 = fa1 * 1.7f + fa2 - fa3;
        fa1 = fa2 * 1.8f + fa3 - fa4;
        da0 = da1 * 1.07 + da2 - da3;
        da1 = da2 * 1.08 + da3 - da4;
        
        switch (selector) {
            case 0:
                a3 = a4 + a5 - a6 * a7;
                a4 = a5 + a6 - a7 * a8;
                fa2 = fa3 * 1.9f + fa4 - fa5;
                da2 = da3 * 1.09 + da4 - da5;
                goto switch_middle;
                
            case 1:
                a5 = a6 + a7 - a8 * a9;
                a6 = a7 + a8 - a9 * a10;
                fa3 = fa4 * 2.0f + fa5 - fa0;
                da3 = da4 * 1.10 + da5 - da0;
                goto switch_end_label;
                
            case 2:
                a7 = a8 + a9 - a10 * a11;
                a8 = a9 + a10 - a11 * a0;
                fa4 = fa5 * 2.1f + fa0 - fa1;
                da4 = da5 * 1.11 + da0 - da1;
                goto switch_start;
                
            case 3:
                a9 = a10 + a11 - a0 * a1;
                a10 = a11 + a0 - a1 * a2;
                fa5 = fa0 * 2.2f + fa1 - fa2;
                da5 = da0 * 1.12 + da1 - da2;
                /* Fall through */
                
            case 4:
                a11 = a0 + a1 - a2 * a3;
                a0 = a1 + a2 - a3 * a4;
                fa0 = fa1 * 2.3f + fa2 - fa3;
                da0 = da1 * 1.13 + da2 - da3;
                goto switch_middle;
                
            default:
                a1 = a2 + a3 - a4 * a5;
                a2 = a3 + a4 - a5 * a6;
                fa1 = fa2 * 2.4f + fa3 - fa4;
                da1 = da2 * 1.14 + da3 - da4;
                break;
        }
        
        switch_end_label:
        /* More operations after switch */
        a3 = a4 | a5 & a6 ^ a7;
        a4 = a5 | a6 & a7 ^ a8;
        fa2 = fa3 + fa4 * fa5 / 3.0f;
        da2 = da3 + da4 * da5 / 3.0;
        
        /* Force liveness */
        asm volatile("" : : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5));
        asm volatile("" : : "r"(a6), "r"(a7), "r"(a8), "r"(a9), "r"(a10), "r"(a11));
        asm volatile("" : : "r"(fa0), "r"(fa1), "r"(fa2), "r"(fa3), "r"(fa4), "r"(fa5));
        asm volatile("" : : "r"(da0), "r"(da1), "r"(da2), "r"(da3), "r"(da4), "r"(da5));
        
        checksum += a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11;
        checksum += (unsigned long)(fa0 + fa1 + fa2 + fa3 + fa4 + fa5);
        checksum += (unsigned long)(da0 + da1 + da2 + da3 + da4 + da5);
    }
    
    return checksum;
}

/* Computed goto for state machine simulation */
NOINLINE static unsigned long test_computed_goto(int iterations, int seed) {
    VOLATILE_VAR int i, state;
    VOLATILE_VAR unsigned long checksum = seed;
    
    /* Yet another large set of variables */
    VOLATILE_VAR int x0 = seed, x1 = seed + 100, x2 = seed + 200, x3 = seed + 300;
    VOLATILE_VAR int x4 = seed + 400, x5 = seed + 500, x6 = seed + 600, x7 = seed + 700;
    VOLATILE_VAR int x8 = seed + 800, x9 = seed + 900, x10 = seed + 1000;
    VOLATILE_VAR float fx0 = seed * 0.07f, fx1 = seed * 0.14f, fx2 = seed * 0.21f;
    VOLATILE_VAR float fx3 = seed * 0.28f, fx4 = seed * 0.35f, fx5 = seed * 0.42f;
    VOLATILE_VAR double dx0 = seed * 0.007, dx1 = seed * 0.014, dx2 = seed * 0.021;
    VOLATILE_VAR double dx3 = seed * 0.028, dx4 = seed * 0.035, dx5 = seed * 0.042;
    
    /* Labels for computed goto */
    static void* states[] = {
        &&state_0, &&state_1, &&state_2, 
        &&state_3, &&state_4, &&state_5,
        &&state_6, &&state_7, &&state_8
    };
    
    state = seed % 9;
    
    for (i = 0; i < iterations; i++) {
        /* Jump to current state */
        goto *states[state];
        
        state_0:
        x0 = x1 + x2 * x3 - x4;
        x1 = x2 + x3 * x4 - x5;
        fx0 = fx1 * 2.5f + fx2 - fx3;
        dx0 = dx1 * 1.15 + dx2 - dx3;
        state = (state + 1) % 9;
        goto state_end;
        
        state_1:
        x2 = x3 + x4 * x5 - x6;
        x3 = x4 + x5 * x6 - x7;
        fx1 = fx2 * 2.6f + fx3 - fx4;
        dx1 = dx2 * 1.16 + dx3 - dx4;
        state = (state + 2) % 9;
        goto state_0;
        
        state_2:
        x4 = x5 + x6 * x7 - x8;
        x5 = x6 + x7 * x8 - x9;
        fx2 = fx3 * 2.7f + fx4 - fx5;
        dx2 = dx3 * 1.17 + dx4 - dx5;
        state = (state + 3) % 9;
        goto state_1;
        
        state_3:
        x6 = x7 + x8 * x9 - x10;
        x7 = x8 + x9 * x10 - x0;
        fx3 = fx4 * 2.8f + fx5 - fx0;
        dx3 = dx4 * 1.18 + dx5 - dx0;
        state = (state + 4) % 9;
        goto state_2;
        
        state_4:
        x8 = x9 + x10 * x0 - x1;
        x9 = x10 + x0 * x1 - x2;
        fx4 = fx5 * 2.9f + fx0 - fx1;
        dx4 = dx5 * 1.19 + dx0 - dx1;
        state = (state + 5) % 9;
        goto state_3;
        
        state_5:
        x10 = x0 + x1 * x2 - x3;
        x0 = x1 + x2 * x3 - x4;
        fx5 = fx0 * 3.0f + fx1 - fx2;
        dx5 = dx0 * 1.20 + dx1 - dx2;
        state = (state + 6) % 9;
        goto state_4;
        
        state_6:
        x1 = x2 + x3 * x4 - x5;
        x2 = x3 + x4 * x5 - x6;
        fx0 = fx1 * 3.1f + fx2 - fx3;
        dx0 = dx1 * 1.21 + dx2 - dx3;
        state = (state + 7) % 9;
        goto state_5;
        
        state_7:
        x3 = x4 + x5 * x6 - x7;
        x4 = x5 + x6 * x7 - x8;
        fx1 = fx2 * 3.2f + fx3 - fx4;
        dx1 = dx2 * 1.22 + dx3 - dx4;
        state = (state + 8) % 9;
        goto state_6;
        
        state_8:
        x5 = x6 + x7 * x8 - x9;
        x6 = x7 + x8 * x9 - x10;
        fx2 = fx3 * 3.3f + fx4 - fx5;
        dx2 = dx3 * 1.23 + dx4 - dx5;
        state = (state * 7 + 1) % 9;
        goto state_7;
        
        state_end:
        /* Cross-type operations */
        x7 = x8 ^ x9 | x10 & x0;
        x8 = x9 ^ x10 | x0 & x1;
        fx3 = fx4 + fx5 * fx0 / 4.0f;
        dx3 = dx4 + dx5 * dx0 / 4.0;
        
        /* Force all variables to be live */
        asm volatile("" : : "r"(x0), "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5));
        asm volatile("" : : "r"(x6), "r"(x7), "r"(x8), "r"(x9), "r"(x10));
        asm volatile("" : : "r"(fx0), "r"(fx1), "r"(fx2), "r"(fx3), "r"(fx4), "r"(fx5));
        asm volatile("" : : "r"(dx0), "r"(dx1), "r"(dx2), "r"(dx3), "r"(dx4), "r"(dx5));
        
        checksum += x0 + x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10;
        checksum += (unsigned long)(fx0 + fx1 + fx2 + fx3 + fx4 + fx5);
        checksum += (unsigned long)(dx0 + dx1 + dx2 + dx3 + dx4 + dx5);
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int iterations = MAX_ITER;
    unsigned long total_checksum = 0;
    int seed = time(NULL);
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = MAX_ITER;
    }
    
    printf("Running MCF coverage test with %d iterations, seed=%d\n", 
           iterations, seed);
    
    /* Run all test patterns to maximize coverage chance */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    printf("Final checksum: %lu\n", total_checksum);
    
    return 0;
}

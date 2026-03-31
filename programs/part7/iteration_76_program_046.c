/* mcf_coverage.c - Program to trigger MCF fixup graph debug output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_VARS 32
#define ITERATIONS 10000

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* Complex irreducible control flow with goto */
NOINLINE static uint64_t test_irreducible_goto(int iterations, int seed) {
    VOLATILE_VAR int i, j;
    uint64_t checksum = 0;
    
    /* Many local variables to create register pressure */
    VOLATILE_VAR int v0 = seed + 1, v1 = seed + 2, v2 = seed + 3, v3 = seed + 4;
    VOLATILE_VAR int v4 = seed + 5, v5 = seed + 6, v6 = seed + 7, v7 = seed + 8;
    VOLATILE_VAR int v8 = seed + 9, v9 = seed + 10, v10 = seed + 11, v11 = seed + 12;
    VOLATILE_VAR int v12 = seed + 13, v13 = seed + 14, v14 = seed + 15, v15 = seed + 16;
    VOLATILE_VAR float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f, f3 = seed * 0.4f;
    VOLATILE_VAR double d0 = seed * 0.01, d1 = seed * 0.02, d2 = seed * 0.03, d3 = seed * 0.04;
    VOLATILE_VAR long l0 = seed * 100L, l1 = seed * 200L, l2 = seed * 300L, l3 = seed * 400L;
    VOLATILE_VAR int v16 = seed + 17, v17 = seed + 18, v18 = seed + 19, v19 = seed + 20;
    VOLATILE_VAR int v20 = seed + 21, v21 = seed + 22, v22 = seed + 23, v23 = seed + 24;
    
    /* Labels for irreducible loops */
    loop_start:
    for (i = 0; i < iterations; i++) {
        /* Complex arithmetic to keep variables live */
        v0 = v1 + v2 * v3 - v4 / (v5 + 1);
        v1 = v2 + v3 * v4 - v5 / (v6 + 1);
        v2 = v3 + v4 * v5 - v6 / (v7 + 1);
        v3 = v4 + v5 * v6 - v7 / (v8 + 1);
        v4 = v5 + v6 * v7 - v8 / (v9 + 1);
        v5 = v6 + v7 * v8 - v9 / (v10 + 1);
        v6 = v7 + v8 * v9 - v10 / (v11 + 1);
        v7 = v8 + v9 * v10 - v11 / (v12 + 1);
        
        f0 = f1 * 1.1f + f2 * 0.9f - f3 * 0.5f;
        f1 = f2 * 1.2f + f3 * 0.8f - f0 * 0.6f;
        f2 = f3 * 1.3f + f0 * 0.7f - f1 * 0.7f;
        f3 = f0 * 1.4f + f1 * 0.6f - f2 * 0.8f;
        
        d0 = d1 * 1.01 + d2 * 0.99 - d3 * 0.95;
        d1 = d2 * 1.02 + d3 * 0.98 - d0 * 0.96;
        d2 = d3 * 1.03 + d0 * 0.97 - d1 * 0.97;
        d3 = d0 * 1.04 + d1 * 0.96 - d2 * 0.98;
        
        l0 = l1 + l2 - l3 * (i % 7 + 1);
        l1 = l2 + l3 - l0 * (i % 5 + 1);
        l2 = l3 + l0 - l1 * (i % 3 + 1);
        l3 = l0 + l1 - l2 * (i % 2 + 1);
        
        /* Irreducible loop with goto jumping across boundaries */
        if (i % 13 == 0) {
            goto inner_loop;
        } else if (i % 17 == 0) {
            goto loop_end;
        } else if (i % 19 == 0) {
            goto loop_start;
        }
        
        continue;
        
        inner_loop:
        for (j = 0; j < 5; j++) {
            v8 = v9 + v10 * v11 - v12 / (v13 + 1);
            v9 = v10 + v11 * v12 - v13 / (v14 + 1);
            v10 = v11 + v12 * v13 - v14 / (v15 + 1);
            
            if (j % 2 == 0 && i % 3 == 0) {
                goto loop_start;  /* Jump to outer loop */
            }
        }
        
        loop_end:
        v11 = v12 + v13 * v14 - v15 / (v16 + 1);
    }
    
    /* Aggregate checksum */
    checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11;
    checksum += (uint64_t)(f0 * 100) + (uint64_t)(f1 * 100) + (uint64_t)(f2 * 100) + (uint64_t)(f3 * 100);
    checksum += (uint64_t)d0 + (uint64_t)d1 + (uint64_t)d2 + (uint64_t)d3;
    checksum += l0 + l1 + l2 + l3;
    
    return checksum;
}

/* Switch with goto creating complex CFG */
NOINLINE static uint64_t test_switch_goto(int iterations, int seed) {
    VOLATILE_VAR int i;
    uint64_t checksum = 0;
    
    /* Many local variables */
    VOLATILE_VAR int a0 = seed, a1 = seed * 2, a2 = seed * 3, a3 = seed * 4;
    VOLATILE_VAR int a4 = seed * 5, a5 = seed * 6, a6 = seed * 7, a7 = seed * 8;
    VOLATILE_VAR int a8 = seed * 9, a9 = seed * 10, a10 = seed * 11, a11 = seed * 12;
    VOLATILE_VAR float fa0 = seed * 0.5f, fa1 = seed * 1.5f, fa2 = seed * 2.5f, fa3 = seed * 3.5f;
    VOLATILE_VAR double da0 = seed * 0.05, da1 = seed * 0.15, da2 = seed * 0.25, da3 = seed * 0.35;
    VOLATILE_VAR long la0 = seed * 50, la1 = seed * 150, la2 = seed * 250, la3 = seed * 350;
    VOLATILE_VAR int a12 = seed * 13, a13 = seed * 14, a14 = seed * 15, a15 = seed * 16;
    VOLATILE_VAR int a16 = seed * 17, a17 = seed * 18, a18 = seed * 19, a19 = seed * 20;
    
    /* Labels for goto targets */
    label_case1:
    label_case2:
    label_case3:
    label_default:
    
    for (i = 0; i < iterations; i++) {
        /* Arithmetic chains */
        a0 = a1 + a2 - a3 * a4 / (a5 + 1);
        a1 = a2 + a3 - a4 * a5 / (a6 + 1);
        a2 = a3 + a4 - a5 * a6 / (a7 + 1);
        a3 = a4 + a5 - a6 * a7 / (a8 + 1);
        a4 = a5 + a6 - a7 * a8 / (a9 + 1);
        a5 = a6 + a7 - a8 * a9 / (a10 + 1);
        
        fa0 = fa1 * 2.0f - fa2 * 1.5f + fa3 * 0.5f;
        fa1 = fa2 * 2.1f - fa3 * 1.6f + fa0 * 0.6f;
        fa2 = fa3 * 2.2f - fa0 * 1.7f + fa1 * 0.7f;
        fa3 = fa0 * 2.3f - fa1 * 1.8f + fa2 * 0.8f;
        
        da0 = da1 * 1.05 - da2 * 0.95 + da3 * 0.45;
        da1 = da2 * 1.06 - da3 * 0.96 + da0 * 0.46;
        da2 = da3 * 1.07 - da0 * 0.97 + da1 * 0.47;
        da3 = da0 * 1.08 - da1 * 0.98 + da2 * 0.48;
        
        la0 = la1 + la2 * (i % 4 + 1) - la3 / (i % 3 + 1);
        la1 = la2 + la3 * (i % 5 + 1) - la0 / (i % 2 + 1);
        la2 = la3 + la0 * (i % 6 + 1) - la1 / (i % 7 + 1);
        la3 = la0 + la1 * (i % 8 + 1) - la2 / (i % 9 + 1);
        
        /* Complex switch with goto jumping to different labels */
        switch (i % 7) {
            case 0:
                a6 = a7 + a8 - a9 * a10;
                if (i % 3 == 0) goto label_case2;
                break;
            case 1:
                a7 = a8 + a9 - a10 * a11;
                if (i % 5 == 0) goto label_case3;
                break;
            case 2:
                a8 = a9 + a10 - a11 * a12;
                if (i % 7 == 0) goto label_default;
                break;
            case 3:
                a9 = a10 + a11 - a12 * a13;
                if (i % 11 == 0) goto label_case1;
                break;
            case 4:
                a10 = a11 + a12 - a13 * a14;
                if (i % 13 == 0) goto label_case2;
                break;
            case 5:
                a11 = a12 + a13 - a14 * a15;
                if (i % 17 == 0) goto label_case3;
                break;
            default:
                a12 = a13 + a14 - a15 * a16;
                if (i % 19 == 0) goto label_case1;
                break;
        }
        
        /* More arithmetic to extend live ranges */
        a13 = a14 + a15 - a16 * a17 / (a18 + 1);
        a14 = a15 + a16 - a17 * a18 / (a19 + 1);
        a15 = a16 + a17 - a18 * a19 / (a0 + 1);
        
        /* Use inline assembly to mark variables as used */
        asm volatile("" : : "r"(a0), "r"(a1), "r"(a2), "r"(a3));
        asm volatile("" : : "r"(fa0), "r"(fa1), "r"(fa2), "r"(fa3));
        asm volatile("" : : "r"(da0), "r"(da1), "r"(da2), "r"(da3));
    }
    
    checksum = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11 + a12 + a13 + a14 + a15;
    checksum += (uint64_t)(fa0 * 1000) + (uint64_t)(fa1 * 1000) + (uint64_t)(fa2 * 1000) + (uint64_t)(fa3 * 1000);
    checksum += (uint64_t)da0 + (uint64_t)da1 + (uint64_t)da2 + (uint64_t)da3;
    checksum += la0 + la1 + la2 + la3;
    
    return checksum;
}

/* Computed goto state machine */
NOINLINE static uint64_t test_computed_goto(int iterations, int seed) {
    VOLATILE_VAR int i, state;
    uint64_t checksum = 0;
    
    /* Many local variables */
    VOLATILE_VAR int x0 = seed, x1 = seed * 3, x2 = seed * 5, x3 = seed * 7;
    VOLATILE_VAR int x4 = seed * 11, x5 = seed * 13, x6 = seed * 17, x7 = seed * 19;
    VOLATILE_VAR int x8 = seed * 23, x9 = seed * 29, x10 = seed * 31, x11 = seed * 37;
    VOLATILE_VAR float fx0 = seed * 0.11f, fx1 = seed * 0.22f, fx2 = seed * 0.33f, fx3 = seed * 0.44f;
    VOLATILE_VAR double dx0 = seed * 0.011, dx1 = seed * 0.022, dx2 = seed * 0.033, dx3 = seed * 0.044;
    VOLATILE_VAR long lx0 = seed * 111, lx1 = seed * 222, lx2 = seed * 333, lx3 = seed * 444;
    VOLATILE_VAR int x12 = seed * 41, x13 = seed * 43, x14 = seed * 47, x15 = seed * 53;
    VOLATILE_VAR int x16 = seed * 59, x17 = seed * 61, x18 = seed * 67, x19 = seed * 71;
    
    /* Labels for computed goto */
    static void* labels[] = {
        &&state0, &&state1, &&state2, &&state3, 
        &&state4, &&state5, &&state6, &&state7
    };
    
    state = seed % 8;
    
    for (i = 0; i < iterations; i++) {
        /* Arithmetic operations */
        x0 = x1 + x2 * x3 - x4 / (x5 + 1);
        x1 = x2 + x3 * x4 - x5 / (x6 + 1);
        x2 = x3 + x4 * x5 - x6 / (x7 + 1);
        x3 = x4 + x5 * x6 - x7 / (x8 + 1);
        x4 = x5 + x6 * x7 - x8 / (x9 + 1);
        x5 = x6 + x7 * x8 - x9 / (x10 + 1);
        
        fx0 = fx1 * 1.5f + fx2 * 0.5f - fx3 * 0.25f;
        fx1 = fx2 * 1.6f + fx3 * 0.6f - fx0 * 0.35f;
        fx2 = fx3 * 1.7f + fx0 * 0.7f - fx1 * 0.45f;
        fx3 = fx0 * 1.8f + fx1 * 0.8f - fx2 * 0.55f;
        
        dx0 = dx1 * 1.005 + dx2 * 0.005 - dx3 * 0.0025;
        dx1 = dx2 * 1.006 + dx3 * 0.006 - dx0 * 0.0035;
        dx2 = dx3 * 1.007 + dx0 * 0.007 - dx1 * 0.0045;
        dx3 = dx0 * 1.008 + dx1 * 0.008 - dx2 * 0.0055;
        
        lx0 = lx1 + lx2 * (i % 3 + 1) - lx3 / (i % 4 + 1);
        lx1 = lx2 + lx3 * (i % 5 + 1) - lx0 / (i % 6 + 1);
        lx2 = lx3 + lx0 * (i % 7 + 1) - lx1 / (i % 8 + 1);
        lx3 = lx0 + lx1 * (i % 9 + 1) - lx2 / (i % 10 + 1);
        
        /* Computed goto - creates irreducible CFG */
        goto *labels[state];
        
        state0:
            x6 = x7 + x8 - x9 * x10;
            state = (state + 1) % 8;
            if (i % 11 == 0) goto *labels[3];
            continue;
            
        state1:
            x7 = x8 + x9 - x10 * x11;
            state = (state + 3) % 8;
            if (i % 13 == 0) goto *labels[5];
            continue;
            
        state2:
            x8 = x9 + x10 - x11 * x12;
            state = (state + 5) % 8;
            if (i % 17 == 0) goto *labels[7];
            continue;
            
        state3:
            x9 = x10 + x11 - x12 * x13;
            state = (state + 7) % 8;
            if (i % 19 == 0) goto *labels[1];
            continue;
            
        state4:
            x10 = x11 + x12 - x13 * x14;
            state = (state + 2) % 8;
            if (i % 23 == 0) goto *labels[0];
            continue;
            
        state5:
            x11 = x12 + x13 - x14 * x15;
            state = (state + 4) % 8;
            if (i % 29 == 0) goto *labels[2];
            continue;
            
        state6:
            x12 = x13 + x14 - x15 * x16;
            state = (state + 6) % 8;
            if (i % 31 == 0) goto *labels[4];
            continue;
            
        state7:
            x13 = x14 + x15 - x16 * x17;
            state = (state + 1) % 8;
            if (i % 37 == 0) goto *labels[6];
            continue;
    }
    
    checksum = x0 + x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10 + x11 + x12 + x13;
    checksum += (uint64_t)(fx0 * 10000) + (uint64_t)(fx1 * 10000) + (uint64_t)(fx2 * 10000) + (uint64_t)(fx3 * 10000);
    checksum += (uint64_t)dx0 + (uint64_t)dx1 + (uint64_t)dx2 + (uint64_t)dx3;
    checksum += lx0 + lx1 + lx2 + lx3;
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int iterations = ITERATIONS;
    int seed = 42;
    uint64_t total_checksum = 0;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = ITERATIONS;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Running MCF coverage test with %d iterations, seed=%d\n", iterations, seed);
    
    /* Run all test functions to increase coverage chances */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    printf("Total checksum: %lu\n", total_checksum);
    
    return 0;
}

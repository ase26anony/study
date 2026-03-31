/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow algorithm
   with complex control flow and high register pressure */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define KEEP_ALIVE(var) asm volatile("" : : "r"(var))

/* Complex irreducible control flow with goto jumping across loops */
NOINLINE unsigned long test_irreducible_goto(int iterations, int seed) {
    /* Declare many variables to create register pressure */
    volatile int a0 = seed + 1, a1 = seed + 2, a2 = seed + 3, a3 = seed + 4;
    volatile int a4 = seed + 5, a5 = seed + 6, a6 = seed + 7, a7 = seed + 8;
    volatile int a8 = seed + 9, a9 = seed + 10, a10 = seed + 11, a11 = seed + 12;
    volatile float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f;
    volatile float f3 = seed * 0.4f, f4 = seed * 0.5f, f5 = seed * 0.6f;
    volatile double d0 = seed * 0.01, d1 = seed * 0.02, d2 = seed * 0.03;
    volatile double d3 = seed * 0.04, d4 = seed * 0.05, d5 = seed * 0.06;
    volatile long l0 = seed * 100L, l1 = seed * 200L, l2 = seed * 300L;
    volatile long l3 = seed * 400L, l4 = seed * 500L, l5 = seed * 600L;
    
    unsigned long checksum = 0;
    int i = 0;
    
    /* Outer loop with label for goto jumps */
    outer_loop:
    while (i < iterations) {
        /* Create arithmetic dependency chains */
        a0 = a1 + a2 * a3 - a4 / (a5 + 1);
        a1 = a2 + a3 * a4 - a5 / (a6 + 1);
        a2 = a3 + a4 * a5 - a6 / (a7 + 1);
        f0 = f1 * 1.1f + f2 * 0.9f - f3 / (f4 + 0.5f);
        f1 = f2 * 1.2f + f3 * 0.8f - f4 / (f5 + 0.5f);
        d0 = d1 * 1.01 + d2 * 0.99 - d3 / (d4 + 0.1);
        d1 = d2 * 1.02 + d3 * 0.98 - d4 / (d5 + 0.1);
        l0 = l1 * 2 + l2 / 3 - l3 * 4 + l4;
        l1 = l2 * 3 + l3 / 4 - l4 * 5 + l5;
        
        /* Inner loop with multiple entry points */
        inner_loop:
        for (int j = 0; j < 10; j++) {
            /* More arithmetic operations */
            a3 = a4 + a5 * a6 - a7 / (a8 + 1);
            a4 = a5 + a6 * a7 - a8 / (a9 + 1);
            f2 = f3 * 1.3f + f4 * 0.7f - f5 / (f0 + 0.5f);
            d2 = d3 * 1.03 + d4 * 0.97 - d5 / (d0 + 0.1);
            l2 = l3 * 4 + l4 / 5 - l5 * 6 + l0;
            
            /* Irreducible control flow: goto to outer loop from inner loop */
            if ((i + j) % 37 == 0) {
                i++;
                goto outer_loop;  /* Jump to outer loop, creating irreducible region */
            }
            
            /* Another goto creating cross-loop jump */
            if ((i * j) % 41 == 0) {
                goto skip_point;  /* Jump forward, skipping code */
            }
            
            a5 = a6 + a7 * a8 - a9 / (a10 + 1);
            a6 = a7 + a8 * a9 - a10 / (a11 + 1);
            
            skip_point:
            f3 = f4 * 1.4f + f5 * 0.6f - f0 / (f1 + 0.5f);
            d3 = d4 * 1.04 + d5 * 0.96 - d0 / (d1 + 0.1);
            l3 = l4 * 5 + l5 / 6 - l0 * 7 + l1;
            
            /* Jump back to inner loop start */
            if (j % 7 == 3) {
                goto inner_loop;
            }
        }
        
        /* Jump to different part of outer loop */
        if (i % 13 == 0) {
            goto middle_outer;
        }
        
        a7 = a8 + a9 * a10 - a11 / (a0 + 1);
        a8 = a9 + a10 * a11 - a0 / (a1 + 1);
        
        middle_outer:
        f4 = f5 * 1.5f + f0 * 0.5f - f1 / (f2 + 0.5f);
        d4 = d5 * 1.05 + d0 * 0.95 - d1 / (d2 + 0.1);
        l4 = l5 * 6 + l0 / 7 - l1 * 8 + l2;
        
        i++;
    }
    
    /* Aggregate checksum from all variables */
    checksum = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11;
    checksum += (unsigned long)(f0 * 100) + (unsigned long)(f1 * 100);
    checksum += (unsigned long)(f2 * 100) + (unsigned long)(f3 * 100);
    checksum += (unsigned long)(f4 * 100) + (unsigned long)(f5 * 100);
    checksum += (unsigned long)d0 + (unsigned long)d1 + (unsigned long)d2;
    checksum += (unsigned long)d3 + (unsigned long)d4 + (unsigned long)d5;
    checksum += l0 + l1 + l2 + l3 + l4 + l5;
    
    KEEP_ALIVE(checksum);
    return checksum;
}

/* Complex switch with goto creating irreducible flow */
NOINLINE unsigned long test_switch_goto(int iterations, int seed) {
    /* Another set of many variables */
    volatile int b0 = seed, b1 = seed * 2, b2 = seed * 3, b3 = seed * 4;
    volatile int b4 = seed * 5, b5 = seed * 6, b6 = seed * 7, b7 = seed * 8;
    volatile int b8 = seed * 9, b9 = seed * 10, b10 = seed * 11, b11 = seed * 12;
    volatile float g0 = seed * 0.15f, g1 = seed * 0.25f, g2 = seed * 0.35f;
    volatile float g3 = seed * 0.45f, g4 = seed * 0.55f, g5 = seed * 0.65f;
    volatile double e0 = seed * 0.015, e1 = seed * 0.025, e2 = seed * 0.035;
    volatile double e3 = seed * 0.045, e4 = seed * 0.055, e5 = seed * 0.065;
    
    unsigned long checksum = 0;
    int state = 0;
    
    /* Labels for goto targets */
    state0_label:
    state1_label:
    state2_label:
    state3_label:
    exit_label:
    
    for (int i = 0; i < iterations; i++) {
        /* Long arithmetic chains */
        b0 = b1 * b2 - b3 / (b4 + 1) + b5 % (b6 + 1);
        b1 = b2 * b3 - b4 / (b5 + 1) + b6 % (b7 + 1);
        b2 = b3 * b4 - b5 / (b6 + 1) + b7 % (b8 + 1);
        g0 = g1 * 1.7f + g2 / 2.3f - g3 * 0.7f + g4;
        g1 = g2 * 1.8f + g3 / 2.4f - g4 * 0.8f + g5;
        e0 = e1 * 1.07 + e2 / 2.03 - e3 * 0.07 + e4;
        e1 = e2 * 1.08 + e3 / 2.04 - e4 * 0.08 + e5;
        
        /* Complex switch with goto to labels outside */
        switch (state) {
            case 0:
                b3 = b4 * b5 - b6 / (b7 + 1) + b8 % (b9 + 1);
                if (i % 11 == 0) goto state2_label;
                state = 1;
                break;
            case 1:
                b4 = b5 * b6 - b7 / (b8 + 1) + b9 % (b10 + 1);
                if (i % 13 == 0) goto state3_label;
                state = 2;
                break;
            case 2:
                b5 = b6 * b7 - b8 / (b9 + 1) + b10 % (b11 + 1);
                if (i % 17 == 0) goto state0_label;
                state = 3;
                break;
            case 3:
                b6 = b7 * b8 - b9 / (b10 + 1) + b11 % (b0 + 1);
                if (i % 19 == 0) goto state1_label;
                state = 0;
                break;
        }
        
        /* More operations after switch */
        g2 = g3 * 1.9f + g4 / 2.5f - g5 * 0.9f + g0;
        g3 = g4 * 2.0f + g5 / 2.6f - g0 * 1.0f + g1;
        e2 = e3 * 1.09 + e4 / 2.05 - e5 * 0.09 + e0;
        e3 = e4 * 1.10 + e5 / 2.06 - e0 * 0.10 + e1;
        
        /* Conditional goto to exit label */
        if (i % 23 == 0 && state == 0) {
            goto exit_label;
        }
        
        b7 = b8 * b9 - b10 / (b11 + 1) + b0 % (b1 + 1);
        b8 = b9 * b10 - b11 / (b0 + 1) + b1 % (b2 + 1);
    }
    
    exit_label:
    checksum = b0 + b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 + b11;
    checksum += (unsigned long)(g0 * 100) + (unsigned long)(g1 * 100);
    checksum += (unsigned long)(g2 * 100) + (unsigned long)(g3 * 100);
    checksum += (unsigned long)(g4 * 100) + (unsigned long)(g5 * 100);
    checksum += (unsigned long)e0 + (unsigned long)e1 + (unsigned long)e2;
    checksum += (unsigned long)e3 + (unsigned long)e4 + (unsigned long)e5;
    
    KEEP_ALIVE(checksum);
    return checksum;
}

/* Computed goto for state machine simulation */
NOINLINE unsigned long test_computed_goto(int iterations, int seed) {
    /* Yet another set of variables */
    volatile int c0 = seed + 100, c1 = seed + 200, c2 = seed + 300;
    volatile int c3 = seed + 400, c4 = seed + 500, c5 = seed + 600;
    volatile int c6 = seed + 700, c7 = seed + 800, c8 = seed + 900;
    volatile float h0 = seed * 1.1f, h1 = seed * 1.2f, h2 = seed * 1.3f;
    volatile float h3 = seed * 1.4f, h4 = seed * 1.5f, h5 = seed * 1.6f;
    volatile double k0 = seed * 1.01, k1 = seed * 1.02, k2 = seed * 1.03;
    volatile double k3 = seed * 1.04, k4 = seed * 1.05, k5 = seed * 1.06;
    
    /* Labels for computed goto */
    static void* labels[] = {
        &&label_a, &&label_b, &&label_c, 
        &&label_d, &&label_e, &&label_f
    };
    
    unsigned long checksum = 0;
    int label_index = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Arithmetic operations before goto */
        c0 = c1 + c2 - c3 * c4 / (c5 + 1);
        c1 = c2 + c3 - c4 * c5 / (c6 + 1);
        h0 = h1 * 2.1f - h2 / 1.5f + h3 * 0.3f;
        h1 = h2 * 2.2f - h3 / 1.6f + h4 * 0.4f;
        k0 = k1 * 2.01 - k2 / 1.05 + k3 * 0.03;
        k1 = k2 * 2.02 - k3 / 1.06 + k4 * 0.04;
        
        /* Computed goto - creates complex CFG */
        goto *labels[label_index % 6];
        
        label_a:
        c2 = c3 + c4 - c5 * c6 / (c7 + 1);
        h2 = h3 * 2.3f - h4 / 1.7f + h5 * 0.5f;
        k2 = k3 * 2.03 - k4 / 1.07 + k5 * 0.05;
        label_index = (label_index + 1) % 6;
        continue;
        
        label_b:
        c3 = c4 + c5 - c6 * c7 / (c8 + 1);
        h3 = h4 * 2.4f - h5 / 1.8f + h0 * 0.6f;
        k3 = k4 * 2.04 - k5 / 1.08 + k0 * 0.06;
        label_index = (label_index + 3) % 6;
        continue;
        
        label_c:
        c4 = c5 + c6 - c7 * c8 / (c0 + 1);
        h4 = h5 * 2.5f - h0 / 1.9f + h1 * 0.7f;
        k4 = k5 * 2.05 - k0 / 1.09 + k1 * 0.07;
        label_index = (label_index + 2) % 6;
        continue;
        
        label_d:
        c5 = c6 + c7 - c8 * c0 / (c1 + 1);
        h5 = h0 * 2.6f - h1 / 2.0f + h2 * 0.8f;
        k5 = k0 * 2.06 - k1 / 1.10 + k2 * 0.08;
        label_index = (label_index + 5) % 6;
        continue;
        
        label_e:
        c6 = c7 + c8 - c0 * c1 / (c2 + 1);
        h0 = h1 * 2.7f - h2 / 2.1f + h3 * 0.9f;
        k0 = k1 * 2.07 - k2 / 1.11 + k3 * 0.09;
        label_index = (label_index + 4) % 6;
        continue;
        
        label_f:
        c7 = c8 + c0 - c1 * c2 / (c3 + 1);
        h1 = h2 * 2.8f - h3 / 2.2f + h4 * 1.0f;
        k1 = k2 * 2.08 - k3 / 1.12 + k4 * 0.10;
        label_index = (label_index + 1) % 6;
        continue;
    }
    
    checksum = c0 + c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8;
    checksum += (unsigned long)(h0 * 100) + (unsigned long)(h1 * 100);
    checksum += (unsigned long)(h2 * 100) + (unsigned long)(h3 * 100);
    checksum += (unsigned long)(h4 * 100) + (unsigned long)(h5 * 100);
    checksum += (unsigned long)k0 + (unsigned long)k1 + (unsigned long)k2;
    checksum += (unsigned long)k3 + (unsigned long)k4 + (unsigned long)k5;
    
    KEEP_ALIVE(checksum);
    return checksum;
}

int main(int argc, char *argv[]) {
    int iterations = 10000;
    int seed = time(NULL);
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10000;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Running MCF coverage test with iterations=%d, seed=%d\n", 
           iterations, seed);
    
    /* Run all test functions to trigger different CFG patterns */
    unsigned long sum1 = test_irreducible_goto(iterations, seed);
    unsigned long sum2 = test_switch_goto(iterations, seed + 1);
    unsigned long sum3 = test_computed_goto(iterations, seed + 2);
    
    /* Final checksum to prevent dead code elimination */
    unsigned long total = sum1 + sum2 + sum3;
    printf("Checksum: %lu\n", total);
    
    return 0;
}

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
NOINLINE static unsigned long test_irreducible_goto(int iterations, int seed) {
    /* Declare many variables to increase register pressure */
    volatile int v0 = seed + 1;
    int v1 = seed + 2, v2 = seed + 3, v3 = seed + 4, v4 = seed + 5;
    int v5 = seed + 6, v6 = seed + 7, v7 = seed + 8, v8 = seed + 9, v9 = seed + 10;
    float f0 = seed * 0.1f, f1 = f0 + 1.1f, f2 = f1 + 2.2f, f3 = f2 + 3.3f;
    double d0 = seed * 0.01, d1 = d0 + 1.11, d2 = d1 + 2.22, d3 = d2 + 3.33;
    long l0 = seed * 100L, l1 = l0 + 111L, l2 = l1 + 222L, l3 = l2 + 333L;
    int v10 = v9 + 11, v11 = v10 + 12, v12 = v11 + 13, v13 = v12 + 14;
    int v14 = v13 + 15, v15 = v14 + 16, v16 = v15 + 17, v17 = v16 + 18;
    float f4 = f3 + 4.4f, f5 = f4 + 5.5f, f6 = f5 + 6.6f, f7 = f6 + 7.7f;
    double d4 = d3 + 4.44, d5 = d4 + 5.55, d6 = d5 + 6.66, d7 = d6 + 7.77;
    long l4 = l3 + 444L, l5 = l4 + 555L, l6 = l5 + 666L, l7 = l6 + 777L;
    
    unsigned long checksum = 0;
    int i = 0;
    
    /* Outer loop with label that can be jumped into from inner loop */
outer_loop_start:
    if (i >= iterations) goto end;
    
    /* Create long arithmetic chains to keep variables live */
    v1 = v0 + v1 * 3 - v2 / 2;
    v2 = v1 + v3 * 2 - v4 / 3;
    v3 = v2 + v5 * 5 - v6 / 4;
    v4 = v3 + v7 * 7 - v8 / 5;
    v5 = v4 + v9 * 11 - v10 / 6;
    f0 = f1 + f2 * 1.5f - f3 / 2.0f;
    f1 = f0 + f4 * 2.5f - f5 / 3.0f;
    d0 = d1 + d2 * 1.7 - d3 / 2.3;
    d1 = d0 + d4 * 2.7 - d5 / 3.3;
    l0 = l1 + l2 * 3 - l3 / 4;
    l1 = l0 + l4 * 5 - l5 / 6;
    
    /* Inner loop with multiple entry points */
    int j = 0;
inner_loop_start:
    if (j >= 10) goto inner_loop_end;
    
    /* More arithmetic to increase register pressure */
    v6 = v5 + v11 * j - v12 / (j + 1);
    v7 = v6 + v13 * (j + 1) - v14 / (j + 2);
    v8 = v7 + v15 * (j + 2) - v16 / (j + 3);
    v9 = v8 + v17 * (j + 3) - v0 / (j + 4);
    f2 = f1 + f6 * (j * 0.1f) - f7 / (j + 1.0f);
    f3 = f2 + f5 * ((j + 1) * 0.2f) - f4 / (j + 2.0f);
    d2 = d1 + d6 * (j * 0.3) - d7 / (j + 1.3);
    d3 = d2 + d5 * ((j + 1) * 0.4) - d4 / (j + 2.3);
    l2 = l1 + l6 * j - l7 / (j + 1);
    l3 = l2 + l5 * (j + 1) - l4 / (j + 2);
    
    /* Irreducible control flow: jump to outer loop from inner loop */
    if ((i * j + seed) % 37 == 0) {
        i++;
        goto outer_loop_start;  /* Jump from inner to outer loop */
    }
    
    /* Another irreducible jump target */
    if ((i * j + seed) % 41 == 0) {
        j += 2;
        goto inner_middle;  /* Jump to middle of inner loop */
    }
    
    j++;
    goto inner_loop_start;

inner_middle:
    /* Middle of inner loop - another entry point */
    v10 = v9 + v0 * j - v1 / (j + 5);
    v11 = v10 + v2 * (j + 1) - v3 / (j + 6);
    f4 = f3 + f0 * (j * 0.5f) - f1 / (j + 3.0f);
    d4 = d3 + d0 * (j * 0.6) - d1 / (j + 3.3);
    l4 = l3 + l0 * j - l1 / (j + 3);
    
    if (j < 10) {
        j++;
        goto inner_loop_start;
    }

inner_loop_end:
    i++;
    goto outer_loop_start;

end:
    /* Aggregate checksum from all variables */
    checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
               v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 +
               (unsigned long)f0 + (unsigned long)f1 + (unsigned long)f2 + 
               (unsigned long)f3 + (unsigned long)f4 + (unsigned long)f5 +
               (unsigned long)f6 + (unsigned long)f7 +
               (unsigned long)d0 + (unsigned long)d1 + (unsigned long)d2 +
               (unsigned long)d3 + (unsigned long)d4 + (unsigned long)d5 +
               (unsigned long)d6 + (unsigned long)d7 +
               l0 + l1 + l2 + l3 + l4 + l5 + l6 + l7;
    
    /* Keep all variables alive */
    KEEP_ALIVE(v0); KEEP_ALIVE(v1); KEEP_ALIVE(v2); KEEP_ALIVE(v3);
    KEEP_ALIVE(v4); KEEP_ALIVE(v5); KEEP_ALIVE(v6); KEEP_ALIVE(v7);
    KEEP_ALIVE(v8); KEEP_ALIVE(v9); KEEP_ALIVE(v10); KEEP_ALIVE(v11);
    KEEP_ALIVE(v12); KEEP_ALIVE(v13); KEEP_ALIVE(v14); KEEP_ALIVE(v15);
    KEEP_ALIVE(v16); KEEP_ALIVE(v17);
    
    return checksum;
}

/* Complex switch with goto creating irreducible regions */
NOINLINE static unsigned long test_switch_goto(int iterations, int seed) {
    volatile int state = seed % 5;
    int a0 = seed, a1 = seed + 1, a2 = seed + 2, a3 = seed + 3, a4 = seed + 4;
    int a5 = seed + 5, a6 = seed + 6, a7 = seed + 7, a8 = seed + 8, a9 = seed + 9;
    float b0 = seed * 0.2f, b1 = b0 + 1.2f, b2 = b1 + 2.2f, b3 = b2 + 3.2f;
    double c0 = seed * 0.02, c1 = c0 + 1.12, c2 = c1 + 2.22, c3 = c2 + 3.32;
    int a10 = a9 + 10, a11 = a10 + 11, a12 = a11 + 12, a13 = a12 + 13;
    int a14 = a13 + 14, a15 = a14 + 15, a16 = a15 + 16, a17 = a16 + 17;
    float b4 = b3 + 4.2f, b5 = b4 + 5.2f, b6 = b5 + 6.2f, b7 = b6 + 7.2f;
    double c4 = c3 + 4.42, c5 = c4 + 5.52, c6 = c5 + 6.62, c7 = c6 + 7.72;
    
    unsigned long checksum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Long arithmetic chains */
        a1 = a0 + a1 * 2 - a2 / 2;
        a2 = a1 + a3 * 3 - a4 / 3;
        a3 = a2 + a5 * 4 - a6 / 4;
        a4 = a3 + a7 * 5 - a8 / 5;
        a5 = a4 + a9 * 6 - a10 / 6;
        b0 = b1 + b2 * 1.3f - b3 / 2.1f;
        b1 = b0 + b4 * 2.3f - b5 / 3.1f;
        c0 = c1 + c2 * 1.4 - c3 / 2.2;
        c1 = c0 + c4 * 2.4 - c5 / 3.2;
        
        /* Complex switch with goto jumping to labels outside */
        switch (state) {
            case 0:
                a6 = a5 + a11 * i - a12 / (i + 1);
                b2 = b1 + b6 * (i * 0.1f) - b7 / (i + 1.1f);
                if ((i + seed) % 7 == 0) goto label_outside_switch;
                state = 1;
                break;
                
            case 1:
                a7 = a6 + a13 * (i + 1) - a14 / (i + 2);
                c2 = c1 + c6 * (i * 0.2) - c7 / (i + 1.2);
                if ((i + seed) % 11 == 0) goto another_label;
                state = 2;
                break;
                
            case 2:
                a8 = a7 + a15 * (i + 2) - a16 / (i + 3);
                b3 = b2 + b5 * ((i + 1) * 0.3f) - b4 / (i + 2.1f);
                if ((i + seed) % 13 == 0) goto label_outside_switch;
                state = 3;
                break;
                
            case 3:
                a9 = a8 + a17 * (i + 3) - a0 / (i + 4);
                c3 = c2 + c5 * ((i + 1) * 0.4) - c4 / (i + 2.2);
                if ((i + seed) % 17 == 0) goto another_label;
                state = 4;
                break;
                
            case 4:
                a10 = a9 + a1 * (i + 4) - a2 / (i + 5);
                b4 = b3 + b0 * ((i + 2) * 0.5f) - b1 / (i + 3.1f);
                state = 0;
                break;
        }
        
        /* Continue normal flow */
        a11 = a10 + a3 * (i + 5) - a4 / (i + 6);
        c4 = c3 + c0 * ((i + 2) * 0.6) - c1 / (i + 3.2);
        continue;
        
    label_outside_switch:
        /* Label outside switch - creates irreducible region */
        a12 = a11 + a5 * (i + 6) - a6 / (i + 7);
        b5 = b4 + b1 * ((i + 3) * 0.7f) - b2 / (i + 4.1f);
        state = (state + 1) % 5;
        continue;
        
    another_label:
        /* Another label outside switch */
        a13 = a12 + a7 * (i + 7) - a8 / (i + 8);
        c5 = c4 + c1 * ((i + 3) * 0.8) - c2 / (i + 4.2);
        state = (state + 2) % 5;
    }
    
    checksum = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 +
               a10 + a11 + a12 + a13 + a14 + a15 + a16 + a17 +
               (unsigned long)b0 + (unsigned long)b1 + (unsigned long)b2 + 
               (unsigned long)b3 + (unsigned long)b4 + (unsigned long)b5 +
               (unsigned long)b6 + (unsigned long)b7 +
               (unsigned long)c0 + (unsigned long)c1 + (unsigned long)c2 +
               (unsigned long)c3 + (unsigned long)c4 + (unsigned long)c5 +
               (unsigned long)c6 + (unsigned long)c7;
    
    KEEP_ALIVE(state);
    return checksum;
}

/* Computed goto state machine for complex CFG */
NOINLINE static unsigned long test_computed_goto(int iterations, int seed) {
    static void* labels[] = { &&state0, &&state1, &&state2, &&state3, &&state4 };
    
    int x0 = seed, x1 = seed + 100, x2 = seed + 200, x3 = seed + 300;
    int x4 = seed + 400, x5 = seed + 500, x6 = seed + 600, x7 = seed + 700;
    float y0 = seed * 0.3f, y1 = y0 + 10.3f, y2 = y1 + 20.3f, y3 = y2 + 30.3f;
    double z0 = seed * 0.03, z1 = z0 + 10.33, z2 = z1 + 20.33, z3 = z2 + 30.33;
    int x8 = x7 + 800, x9 = x8 + 900, x10 = x9 + 1000, x11 = x10 + 1100;
    int x12 = x11 + 1200, x13 = x12 + 1300, x14 = x13 + 1400, x15 = x14 + 1500;
    float y4 = y3 + 40.3f, y5 = y4 + 50.3f, y6 = y5 + 60.3f, y7 = y6 + 70.3f;
    double z4 = z3 + 40.33, z5 = z4 + 50.33, z6 = z5 + 60.33, z7 = z6 + 70.33;
    
    unsigned long checksum = 0;
    int state = seed % 5;
    
    for (int i = 0; i < iterations; i++) {
        /* Arithmetic operations to increase register pressure */
        x1 = x0 + x1 * ((i % 3) + 1) - x2 / ((i % 5) + 1);
        x2 = x1 + x3 * ((i % 4) + 1) - x4 / ((i % 6) + 1);
        x3 = x2 + x5 * ((i % 5) + 1) - x6 / ((i % 7) + 1);
        x4 = x3 + x7 * ((i % 6) + 1) - x8 / ((i % 8) + 1);
        y0 = y1 + y2 * (i * 0.05f + 1.0f) - y3 / (i * 0.1f + 2.0f);
        y1 = y0 + y4 * (i * 0.06f + 1.1f) - y5 / (i * 0.11f + 2.1f);
        z0 = z1 + z2 * (i * 0.07 + 1.2) - z3 / (i * 0.12 + 2.2);
        z1 = z0 + z4 * (i * 0.08 + 1.3) - z5 / (i * 0.13 + 2.3);
        
        /* Computed goto - creates complex CFG */
        goto *labels[state];
        
    state0:
        x5 = x4 + x9 * (i + 1) - x10 / (i + 2);
        y2 = y1 + y6 * (i * 0.2f) - y7 / (i + 1.2f);
        state = (state + (i % 3)) % 5;
        if ((i + seed) % 19 == 0) goto state3;
        continue;
        
    state1:
        x6 = x5 + x11 * (i + 2) - x12 / (i + 3);
        z2 = z1 + z6 * (i * 0.3) - z7 / (i + 1.3);
        state = (state + (i % 4)) % 5;
        if ((i + seed) % 23 == 0) goto state0;
        continue;
        
    state2:
        x7 = x6 + x13 * (i + 3) - x14 / (i + 4);
        y3 = y2 + y5 * ((i + 1) * 0.4f) - y4 / (i + 2.2f);
        state = (state + (i % 5)) % 5;
        if ((i + seed) % 29 == 0) goto state4;
        continue;
        
    state3:
        x8 = x7 + x15 * (i + 4) - x0 / (i + 5);
        z3 = z2 + z5 * ((i + 1) * 0.5) - z4 / (i + 2.3);
        state = (state + (i % 2)) % 5;
        if ((i + seed) % 31 == 0) goto state1;
        continue;
        
    state4:
        x9 = x8 + x1 * (i + 5) - x2 / (i + 6);
        y4 = y3 + y0 * ((i + 2) * 0.6f) - y1 / (i + 3.2f);
        state = (state + (i % 3)) % 5;
        if ((i + seed) % 37 == 0) goto state2;
        continue;
    }
    
    checksum = x0 + x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 +
               x10 + x11 + x12 + x13 + x14 + x15 +
               (unsigned long)y0 + (unsigned long)y1 + (unsigned long)y2 + 
               (unsigned long)y3 + (unsigned long)y4 + (unsigned long)y5 +
               (unsigned long)y6 + (unsigned long)y7 +
               (unsigned long)z0 + (unsigned long)z1 + (unsigned long)z2 +
               (unsigned long)z3 + (unsigned long)z4 + (unsigned long)z5 +
               (unsigned long)z6 + (unsigned long)z7;
    
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
    
    unsigned long total_checksum = 0;
    
    /* Run all test functions to increase coverage chances */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    printf("Total checksum: %lu\n", total_checksum);
    
    return 0;
}

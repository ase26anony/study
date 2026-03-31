/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow algorithm
   with fixup graph creation for coverage testing */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define KEEP_ALIVE(var) asm volatile("" : : "r"(var))

/* Function 1: Irreducible loop with goto jumping across loop boundaries */
NOINLINE static unsigned long test_irreducible_goto(int iterations, int seed) {
    /* Declare many variables to increase register pressure */
    volatile int v0 = seed + 1, v1 = seed + 2, v2 = seed + 3, v3 = seed + 4;
    volatile int v4 = seed + 5, v5 = seed + 6, v6 = seed + 7, v7 = seed + 8;
    volatile int v8 = seed + 9, v9 = seed + 10, v10 = seed + 11, v11 = seed + 12;
    volatile float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f;
    volatile float f3 = seed * 0.4f, f4 = seed * 0.5f, f5 = seed * 0.6f;
    volatile double d0 = seed * 0.01, d1 = seed * 0.02, d2 = seed * 0.03;
    volatile long l0 = seed * 100, l1 = seed * 200, l2 = seed * 300;
    volatile long l3 = seed * 400, l4 = seed * 500, l5 = seed * 600;
    volatile long l6 = seed * 700, l7 = seed * 800, l8 = seed * 900;
    
    unsigned long checksum = 0;
    int i;
    
    /* Outer loop */
    for (i = 0; i < iterations; i++) {
        /* Create irreducible region with goto */
        if (i % 3 == 0) {
            goto label_a;
        } else if (i % 3 == 1) {
            goto label_b;
        } else {
            goto label_c;
        }
        
    label_a:
        /* Complex arithmetic chain using many variables */
        v0 = v1 + v2 * v3 - v4 / (v5 + 1);
        v1 = v2 * v3 + v4 - v5 / (v6 + 1);
        v2 = v3 + v4 * v5 - v6 / (v7 + 1);
        f0 = f1 * 2.0f + f2 / 3.0f - f3 * 4.0f;
        d0 = d1 * 1.5 + d2 / 2.5 - seed * 0.001;
        l0 = l1 + l2 * l3 - l4 / (l5 + 1);
        checksum += v0 + (int)f0 + (int)d0 + l0;
        
        if (i % 5 == 0) {
            goto label_d;  /* Jump to inner loop */
        } else {
            goto label_c;
        }
        
    label_b:
        /* Different arithmetic pattern */
        v3 = v4 + v5 * v6 - v7 / (v8 + 1);
        v4 = v5 * v6 + v7 - v8 / (v9 + 1);
        f1 = f2 * 3.0f + f3 / 4.0f - f4 * 5.0f;
        d1 = d2 * 2.5 + seed * 0.002 - d0 / 2.0;
        l1 = l2 + l3 * l4 - l5 / (l6 + 1);
        checksum += v3 + (int)f1 + (int)d1 + l1;
        
        if (i % 7 == 0) {
            goto label_a;  /* Jump back */
        } else {
            goto label_d;
        }
        
    label_c:
        /* More arithmetic */
        v5 = v6 + v7 * v8 - v9 / (v10 + 1);
        v6 = v7 * v8 + v9 - v10 / (v11 + 1);
        f2 = f3 * 4.0f + f4 / 5.0f - f5 * 6.0f;
        d2 = seed * 0.003 + d0 * 1.1 - d1 / 1.5;
        l2 = l3 + l4 * l5 - l6 / (l7 + 1);
        checksum += v5 + (int)f2 + (int)d2 + l2;
        
        if (i % 11 == 0) {
            goto label_b;
        }
        /* Fall through to label_d */
        
    label_d:
        /* Inner loop simulation */
        v7 = v8 + v9 * v10 - v11 / (v0 + 1);
        v8 = v9 * v10 + v11 - v0 / (v1 + 1);
        f3 = f4 * 5.0f + f5 / 6.0f - f0 * 7.0f;
        l3 = l4 + l5 * l6 - l7 / (l8 + 1);
        checksum += v7 + (int)f3 + l3;
        
        /* Force all variables to stay live */
        KEEP_ALIVE(v0); KEEP_ALIVE(v1); KEEP_ALIVE(v2); KEEP_ALIVE(v3);
        KEEP_ALIVE(v4); KEEP_ALIVE(v5); KEEP_ALIVE(v6); KEEP_ALIVE(v7);
        KEEP_ALIVE(v8); KEEP_ALIVE(v9); KEEP_ALIVE(v10); KEEP_ALIVE(v11);
        KEEP_ALIVE(f0); KEEP_ALIVE(f1); KEEP_ALIVE(f2); KEEP_ALIVE(f3);
        KEEP_ALIVE(f4); KEEP_ALIVE(f5); KEEP_ALIVE(d0); KEEP_ALIVE(d1);
        KEEP_ALIVE(d2); KEEP_ALIVE(l0); KEEP_ALIVE(l1); KEEP_ALIVE(l2);
        KEEP_ALIVE(l3); KEEP_ALIVE(l4); KEEP_ALIVE(l5); KEEP_ALIVE(l6);
        KEEP_ALIVE(l7); KEEP_ALIVE(l8);
    }
    
    return checksum;
}

/* Function 2: Switch with goto creating complex CFG */
NOINLINE static unsigned long test_switch_goto(int iterations, int seed) {
    volatile int a0 = seed, a1 = seed * 2, a2 = seed * 3, a3 = seed * 4;
    volatile int a4 = seed * 5, a5 = seed * 6, a6 = seed * 7, a7 = seed * 8;
    volatile int a8 = seed * 9, a9 = seed * 10, a10 = seed * 11, a11 = seed * 12;
    volatile float b0 = seed * 0.15f, b1 = seed * 0.25f, b2 = seed * 0.35f;
    volatile float b3 = seed * 0.45f, b4 = seed * 0.55f, b5 = seed * 0.65f;
    volatile double c0 = seed * 0.015, c1 = seed * 0.025, c2 = seed * 0.035;
    volatile long e0 = seed * 150, e1 = seed * 250, e2 = seed * 350;
    volatile long e3 = seed * 450, e4 = seed * 550, e5 = seed * 650;
    
    unsigned long checksum = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        int mod = i % 12;
        
        switch (mod) {
            case 0:
                a0 = a1 + a2;
                goto out_of_switch_1;
            case 1:
                a1 = a2 * a3;
                goto out_of_switch_2;
            case 2:
                a2 = a3 - a4;
                goto out_of_switch_3;
            case 3:
                a3 = a4 / (a5 + 1);
                goto out_of_switch_1;
            case 4:
                a4 = a5 + a6 * a7;
                goto out_of_switch_2;
            case 5:
                a5 = a6 - a7 / (a8 + 1);
                goto out_of_switch_3;
            case 6:
                a6 = a7 * a8 + a9;
                b0 = b1 * 1.5f + b2;
                goto out_of_switch_1;
            case 7:
                a7 = a8 - a9 * a10;
                b1 = b2 / 2.0f - b3;
                goto out_of_switch_2;
            case 8:
                a8 = a9 + a10 / (a11 + 1);
                b2 = b3 * 3.0f + b4;
                c0 = c1 * 1.7 + c2;
                goto out_of_switch_3;
            case 9:
                a9 = a10 * a11 - a0;
                b3 = b4 / 1.3f - b5;
                c1 = c2 * 2.1 - c0;
                goto out_of_switch_1;
            case 10:
                a10 = a11 + a0 * a1;
                b4 = b5 * 2.5f + b0;
                c2 = c0 / 1.4 + c1;
                e0 = e1 + e2 * e3;
                goto out_of_switch_2;
            case 11:
                a11 = a0 - a1 / (a2 + 1);
                b5 = b0 / 1.8f - b1;
                e1 = e2 - e3 * e4;
                goto out_of_switch_3;
        }
        
    out_of_switch_1:
        /* Cross-block arithmetic */
        a0 = a1 + a2 * a3 - a4;
        b0 = b1 + b2 - b3 * b4;
        checksum += a0 + (int)b0 + (int)c0 + e0;
        if (i % 13 == 0) goto switch_end;
        
    out_of_switch_2:
        a1 = a2 * a3 + a4 - a5;
        b1 = b2 * b3 - b4 / b5;
        c0 = c1 + c2 * 1.1;
        checksum += a1 + (int)b1 + (int)c0 + e1;
        if (i % 17 == 0) goto out_of_switch_1;
        
    out_of_switch_3:
        a2 = a3 - a4 * a5 + a6;
        b2 = b3 / b4 + b5 * b0;
        c1 = c2 - c0 / 1.3;
        e0 = e1 * e2 + e3 - e4;
        checksum += a2 + (int)b2 + (int)c1 + e0;
        
    switch_end:
        /* Keep variables alive */
        KEEP_ALIVE(a0); KEEP_ALIVE(a1); KEEP_ALIVE(a2); KEEP_ALIVE(a3);
        KEEP_ALIVE(a4); KEEP_ALIVE(a5); KEEP_ALIVE(a6); KEEP_ALIVE(a7);
        KEEP_ALIVE(a8); KEEP_ALIVE(a9); KEEP_ALIVE(a10); KEEP_ALIVE(a11);
        KEEP_ALIVE(b0); KEEP_ALIVE(b1); KEEP_ALIVE(b2); KEEP_ALIVE(b3);
        KEEP_ALIVE(b4); KEEP_ALIVE(b5); KEEP_ALIVE(c0); KEEP_ALIVE(c1);
        KEEP_ALIVE(c2); KEEP_ALIVE(e0); KEEP_ALIVE(e1); KEEP_ALIVE(e2);
        KEEP_ALIVE(e3); KEEP_ALIVE(e4); KEEP_ALIVE(e5);
    }
    
    return checksum;
}

/* Function 3: Computed goto simulating state machine */
NOINLINE static unsigned long test_computed_goto(int iterations, int seed) {
    volatile int x0 = seed, x1 = seed + 1, x2 = seed + 2, x3 = seed + 3;
    volatile int x4 = seed + 4, x5 = seed + 5, x6 = seed + 6, x7 = seed + 7;
    volatile int x8 = seed + 8, x9 = seed + 9, x10 = seed + 10, x11 = seed + 11;
    volatile float y0 = seed * 0.2f, y1 = seed * 0.3f, y2 = seed * 0.4f;
    volatile float y3 = seed * 0.5f, y4 = seed * 0.6f, y5 = seed * 0.7f;
    volatile double z0 = seed * 0.02, z1 = seed * 0.03, z2 = seed * 0.04;
    volatile long w0 = seed * 120, w1 = seed * 130, w2 = seed * 140;
    volatile long w3 = seed * 150, w4 = seed * 160, w5 = seed * 170;
    
    /* Labels for computed goto */
    static void* labels[] = {
        &&state0, &&state1, &&state2, &&state3, 
        &&state4, &&state5, &&state6, &&state7
    };
    
    unsigned long checksum = 0;
    int i, state = 0;
    
    for (i = 0; i < iterations; i++) {
        /* Update state based on iteration */
        state = (state + (i % 5)) % 8;
        
        /* Jump to current state */
        goto *labels[state];
        
    state0:
        x0 = x1 + x2 * x3 - x4;
        y0 = y1 * 1.1f + y2;
        z0 = z1 + z2 * 0.5;
        w0 = w1 + w2 - w3;
        checksum += x0 + (int)y0 + (int)z0 + w0;
        if (i % 19 == 0) state = 4;
        continue;
        
    state1:
        x1 = x2 * x3 + x4 - x5;
        y1 = y2 / 1.3f - y3;
        z1 = z2 * 1.2 - z0;
        w1 = w2 * w3 + w4;
        checksum += x1 + (int)y1 + (int)z1 + w1;
        if (i % 23 == 0) state = 0;
        continue;
        
    state2:
        x2 = x3 - x4 * x5 + x6;
        y2 = y3 * 1.4f + y4 - y5;
        z2 = z0 / 1.1 + z1;
        w2 = w3 - w4 * w5;
        checksum += x2 + (int)y2 + (int)z2 + w2;
        if (i % 29 == 0) state = 6;
        continue;
        
    state3:
        x3 = x4 + x5 / (x6 + 1) - x7;
        y3 = y4 / 1.6f - y5 * y0;
        w3 = w4 + w5 * w0 - w1;
        checksum += x3 + (int)y3 + w3;
        if (i % 31 == 0) state = 1;
        continue;
        
    state4:
        x4 = x5 * x6 + x7 - x8;
        y4 = y5 * 1.8f + y0 - y1;
        z0 = z1 * 1.3 + z2 / 1.2;
        w4 = w5 - w0 * w1 + w2;
        checksum += x4 + (int)y4 + (int)z0 + w4;
        if (i % 37 == 0) state = 2;
        continue;
        
    state5:
        x5 = x6 - x7 / (x8 + 1) * x9;
        y5 = y0 / 1.9f - y1 * y2;
        z1 = z2 + z0 * 1.4 - seed * 0.001;
        w5 = w0 * w1 + w2 - w3;
        checksum += x5 + (int)y5 + (int)z1 + w5;
        if (i % 41 == 0) state = 3;
        continue;
        
    state6:
        x6 = x7 + x8 * x9 - x10;
        y0 = y1 * 2.1f + y2 - y3;
        z2 = z0 / 1.5 + z1 * 1.6;
        w0 = w1 + w2 * w3 - w4;
        checksum += x6 + (int)y0 + (int)z2 + w0;
        if (i % 43 == 0) state = 5;
        continue;
        
    state7:
        x7 = x8 - x9 * x10 + x11;
        y1 = y2 / 2.2f - y3 * y4;
        w1 = w2 + w3 - w4 * w5;
        checksum += x7 + (int)y1 + w1;
        if (i % 47 == 0) state = 7;
        continue;
    }
    
    /* Keep all variables alive */
    KEEP_ALIVE(x0); KEEP_ALIVE(x1); KEEP_ALIVE(x2); KEEP_ALIVE(x3);
    KEEP_ALIVE(x4); KEEP_ALIVE(x5); KEEP_ALIVE(x6); KEEP_ALIVE(x7);
    KEEP_ALIVE(x8); KEEP_ALIVE(x9); KEEP_ALIVE(x10); KEEP_ALIVE(x11);
    KEEP_ALIVE(y0); KEEP_ALIVE(y1); KEEP_ALIVE(y2); KEEP_ALIVE(y3);
    KEEP_ALIVE(y4); KEEP_ALIVE(y5); KEEP_ALIVE(z0); KEEP_ALIVE(z1);
    KEEP_ALIVE(z2); KEEP_ALIVE(w0); KEEP_ALIVE(w1); KEEP_ALIVE(w2);
    KEEP_ALIVE(w3); KEEP_ALIVE(w4); KEEP_ALIVE(w5);
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int iterations = 10000;
    int seed = time(NULL);
    unsigned long total_checksum = 0;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10000;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Running MCF coverage test with %d iterations, seed=%d\n", 
           iterations, seed);
    
    /* Call all test functions to trigger different CFG patterns */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    printf("Final checksum: %lu\n", total_checksum);
    
    return 0;
}

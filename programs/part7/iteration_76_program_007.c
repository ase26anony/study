/* mcf_coverage.c - Program to trigger MCF fixup graph debug output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
__attribute__((noinline,noipa))
static uint64_t test_irreducible_goto(int iterations, int seed) {
    /* Declare many variables to create register pressure */
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
    volatile float f0 = seed * 0.1f;
    volatile float f1 = seed * 0.2f;
    volatile float f2 = seed * 0.3f;
    volatile float f3 = seed * 0.4f;
    volatile double d0 = seed * 0.01;
    volatile double d1 = seed * 0.02;
    volatile double d2 = seed * 0.03;
    volatile double d3 = seed * 0.04;
    volatile long l0 = seed * 100L;
    volatile long l1 = seed * 200L;
    volatile long l2 = seed * 300L;
    volatile long l3 = seed * 400L;
    volatile int v10 = seed + 11;
    volatile int v11 = seed + 12;
    volatile int v12 = seed + 13;
    volatile int v13 = seed + 14;
    volatile int v14 = seed + 15;
    volatile int v15 = seed + 16;
    volatile int v16 = seed + 17;
    volatile int v17 = seed + 18;
    volatile int v18 = seed + 19;
    volatile int v19 = seed + 20;
    
    uint64_t checksum = 0;
    int i = 0;
    
    /* Create irreducible loop with goto jumping across loop boundaries */
    outer_loop:
    for (; i < iterations; i++) {
        /* Complex arithmetic to keep variables live */
        v0 = v1 + v2;
        v1 = v3 * v4;
        v2 = v5 - v6;
        v3 = v7 ^ v8;
        v4 = v9 | v10;
        f0 = f1 * f2 + f3;
        f1 = f2 - f3 * f0;
        d0 = d1 / d2 + d3;
        d1 = d2 * d3 - d0;
        l0 = l1 + l2 * l3;
        l1 = l2 - l3 / (l0 + 1);
        
        if (i % 7 == 0) {
            goto inner_label;  /* Jump into inner loop */
        }
        
        if (i % 13 == 0) {
            goto outer_loop;   /* Jump back to loop start */
        }
        
        /* More arithmetic */
        v5 = v6 + v7 * v8;
        v6 = v9 - v10 ^ v11;
        v7 = v12 | v13 & v14;
        v8 = v15 << v16;
        v9 = v17 >> v18;
        f2 = f3 * 2.0f - f1;
        f3 = f0 / 3.0f + f2;
        d2 = d3 * 1.5 - d1;
        d3 = d0 / 2.5 + d2;
        l2 = l3 * 3 + l1;
        l3 = l0 / 4 - l2;
        
        continue;
        
        inner_label:
        /* Inner loop label that can be jumped into */
        v10 = v11 + v12 - v13;
        v11 = v14 * v15 / (v16 + 1);
        v12 = v17 ^ v18 | v19;
        v13 = v0 & v1 | v2;
        v14 = v3 << v4;
        v15 = v5 >> v6;
        
        if (i % 11 == 0) {
            goto exit_label;   /* Jump out of loop */
        }
        
        if (i % 17 == 0) {
            goto outer_loop;   /* Jump back to outer loop */
        }
    }
    
    exit_label:
    /* Aggregate checksum from all variables */
    checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
               (uint64_t)f0 + (uint64_t)f1 + (uint64_t)f2 + (uint64_t)f3 +
               (uint64_t)d0 + (uint64_t)d1 + (uint64_t)d2 + (uint64_t)d3 +
               l0 + l1 + l2 + l3 +
               v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
    
    /* Use inline assembly to mark variables as used */
    asm volatile("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4));
    asm volatile("" : : "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9));
    asm volatile("" : : "r"(f0), "r"(f1), "r"(f2), "r"(f3));
    asm volatile("" : : "r"(d0), "r"(d1), "r"(d2), "r"(d3));
    asm volatile("" : : "r"(l0), "r"(l1), "r"(l2), "r"(l3));
    
    return checksum;
}

__attribute__((noinline,noipa))
static uint64_t test_switch_goto(int iterations, int seed) {
    /* Another set of many variables */
    volatile int a0 = seed * 2;
    volatile int a1 = seed * 3;
    volatile int a2 = seed * 4;
    volatile int a3 = seed * 5;
    volatile int a4 = seed * 6;
    volatile int a5 = seed * 7;
    volatile int a6 = seed * 8;
    volatile int a7 = seed * 9;
    volatile int a8 = seed * 10;
    volatile int a9 = seed * 11;
    volatile float b0 = seed * 0.5f;
    volatile float b1 = seed * 0.6f;
    volatile float b2 = seed * 0.7f;
    volatile float b3 = seed * 0.8f;
    volatile double c0 = seed * 0.15;
    volatile double c1 = seed * 0.25;
    volatile double c2 = seed * 0.35;
    volatile double c3 = seed * 0.45;
    volatile long e0 = seed * 500L;
    volatile long e1 = seed * 600L;
    volatile long e2 = seed * 700L;
    volatile long e3 = seed * 800L;
    volatile int a10 = seed * 12;
    volatile int a11 = seed * 13;
    volatile int a12 = seed * 14;
    volatile int a13 = seed * 15;
    volatile int a14 = seed * 16;
    volatile int a15 = seed * 17;
    volatile int a16 = seed * 18;
    volatile int a17 = seed * 19;
    volatile int a18 = seed * 20;
    volatile int a19 = seed * 21;
    
    uint64_t checksum = 0;
    
    /* Switch with goto creating irreducible flow */
    for (int i = 0; i < iterations; i++) {
        int mod = i % 19;
        
        /* Complex arithmetic before switch */
        a0 = a1 + a2 * a3;
        a1 = a4 - a5 ^ a6;
        a2 = a7 | a8 & a9;
        a3 = a10 << a11;
        a4 = a12 >> a13;
        b0 = b1 * b2 - b3;
        b1 = b2 / b3 + b0;
        c0 = c1 * 1.1 - c2;
        c1 = c3 / 1.2 + c0;
        e0 = e1 + e2 * e3;
        e1 = e2 - e3 / (e0 + 1);
        
        switch (mod) {
            case 0:
                a5 = a6 + a7 - a8;
                a6 = a9 * a10 / (a11 + 1);
                goto label_outside_switch;
                
            case 1:
                a7 = a8 ^ a9 | a10;
                a8 = a11 & a12 | a13;
                goto label_inside_loop;
                
            case 2:
                a9 = a10 << a11;
                a10 = a12 >> a13;
                /* Fall through */
                
            case 3:
                a11 = a12 + a13 * a14;
                a12 = a15 - a16 ^ a17;
                break;
                
            case 4:
                a13 = a14 | a15 & a16;
                a14 = a17 << a18;
                goto label_outside_switch;
                
            case 5:
                a15 = a16 >> a17;
                a16 = a18 + a19 - a0;
                goto label_inside_loop;
                
            default:
                a17 = a18 * a19 / (a0 + 1);
                a18 = a1 ^ a2 | a3;
                break;
        }
        
        /* More arithmetic after switch */
        a19 = a0 & a1 | a2;
        a0 = a3 << a4;
        a1 = a5 >> a6;
        b2 = b3 * 3.0f - b1;
        b3 = b0 / 4.0f + b2;
        c2 = c3 * 2.5 - c1;
        c3 = c0 / 3.5 + c2;
        e2 = e3 * 5 + e1;
        e3 = e0 / 6 - e2;
        
        continue;
        
        label_inside_loop:
        /* Label inside the loop but outside switch */
        a2 = a3 + a4 - a5;
        a3 = a6 * a7 / (a8 + 1);
        a4 = a9 ^ a10 | a11;
        continue;
    }
    
    label_outside_switch:
    /* Label outside the loop entirely */
    a5 = a6 + a7 * a8;
    a6 = a9 - a10 ^ a11;
    
    /* Aggregate checksum */
    checksum = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 +
               (uint64_t)b0 + (uint64_t)b1 + (uint64_t)b2 + (uint64_t)b3 +
               (uint64_t)c0 + (uint64_t)c1 + (uint64_t)c2 + (uint64_t)c3 +
               e0 + e1 + e2 + e3 +
               a10 + a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19;
    
    /* Mark variables as used */
    asm volatile("" : : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4));
    asm volatile("" : : "r"(a5), "r"(a6), "r"(a7), "r"(a8), "r"(a9));
    asm volatile("" : : "r"(b0), "r"(b1), "r"(b2), "r"(b3));
    asm volatile("" : : "r"(c0), "r"(c1), "r"(c2), "r"(c3));
    asm volatile("" : : "r"(e0), "r"(e1), "r"(e2), "r"(e3));
    
    return checksum;
}

__attribute__((noinline,noipa))
static uint64_t test_computed_goto(int iterations, int seed) {
    /* Yet another set of variables */
    volatile int x0 = seed * 3;
    volatile int x1 = seed * 4;
    volatile int x2 = seed * 5;
    volatile int x3 = seed * 6;
    volatile int x4 = seed * 7;
    volatile int x5 = seed * 8;
    volatile int x6 = seed * 9;
    volatile int x7 = seed * 10;
    volatile int x8 = seed * 11;
    volatile int x9 = seed * 12;
    volatile float y0 = seed * 0.9f;
    volatile float y1 = seed * 1.0f;
    volatile float y2 = seed * 1.1f;
    volatile float y3 = seed * 1.2f;
    volatile double z0 = seed * 0.55;
    volatile double z1 = seed * 0.65;
    volatile double z2 = seed * 0.75;
    volatile double z3 = seed * 0.85;
    volatile long w0 = seed * 900L;
    volatile long w1 = seed * 1000L;
    volatile long w2 = seed * 1100L;
    volatile long w3 = seed * 1200L;
    volatile int x10 = seed * 13;
    volatile int x11 = seed * 14;
    volatile int x12 = seed * 15;
    volatile int x13 = seed * 16;
    volatile int x14 = seed * 17;
    volatile int x15 = seed * 18;
    volatile int x16 = seed * 19;
    volatile int x17 = seed * 20;
    volatile int x18 = seed * 21;
    volatile int x19 = seed * 22;
    
    /* Labels for computed goto */
    static void* labels[] = {
        &&state0, &&state1, &&state2, &&state3, 
        &&state4, &&state5, &&state6, &&state7
    };
    
    uint64_t checksum = 0;
    int state = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex arithmetic in each iteration */
        x0 = x1 + x2 * x3;
        x1 = x4 - x5 ^ x6;
        x2 = x7 | x8 & x9;
        x3 = x10 << x11;
        x4 = x12 >> x13;
        y0 = y1 * y2 - y3;
        y1 = y2 / y3 + y0;
        z0 = z1 * 1.3 - z2;
        z1 = z3 / 1.4 + z0;
        w0 = w1 + w2 * w3;
        w1 = w2 - w3 / (w0 + 1);
        
        /* Update state based on complex condition */
        state = (state + (i % 7) * (x0 & 0x7)) % 8;
        
        /* Computed goto */
        goto *labels[state];
        
        state0:
            x5 = x6 + x7 - x8;
            x6 = x9 * x10 / (x11 + 1);
            state = (state + 1) % 8;
            continue;
            
        state1:
            x7 = x8 ^ x9 | x10;
            x8 = x11 & x12 | x13;
            state = (state + 3) % 8;
            continue;
            
        state2:
            x9 = x10 << x11;
            x10 = x12 >> x13;
            state = (state + 5) % 8;
            continue;
            
        state3:
            x11 = x12 + x13 * x14;
            x12 = x15 - x16 ^ x17;
            state = (state + 2) % 8;
            continue;
            
        state4:
            x13 = x14 | x15 & x16;
            x14 = x17 << x18;
            state = (state + 4) % 8;
            continue;
            
        state5:
            x15 = x16 >> x17;
            x16 = x18 + x19 - x0;
            state = (state + 6) % 8;
            continue;
            
        state6:
            x17 = x18 * x19 / (x0 + 1);
            x18 = x1 ^ x2 | x3;
            state = (state + 7) % 8;
            continue;
            
        state7:
            x19 = x0 & x1 | x2;
            x0 = x3 << x4;
            state = (state + 1) % 8;
            continue;
    }
    
    /* Aggregate checksum */
    checksum = x0 + x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 +
               (uint64_t)y0 + (uint64_t)y1 + (uint64_t)y2 + (uint64_t)y3 +
               (uint64_t)z0 + (uint64_t)z1 + (uint64_t)z2 + (uint64_t)z3 +
               w0 + w1 + w2 + w3 +
               x10 + x11 + x12 + x13 + x14 + x15 + x16 + x17 + x18 + x19;
    
    /* Mark variables as used */
    asm volatile("" : : "r"(x0), "r"(x1), "r"(x2), "r"(x3), "r"(x4));
    asm volatile("" : : "r"(x5), "r"(x6), "r"(x7), "r"(x8), "r"(x9));
    asm volatile("" : : "r"(y0), "r"(y1), "r"(y2), "r"(y3));
    asm volatile("" : : "r"(z0), "r"(z1), "r"(z2), "r"(z3));
    asm volatile("" : : "r"(w0), "r"(w1), "r"(w2), "r"(w3));
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int iterations = 10000;
    int seed = 42;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10000;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Running MCF coverage test with %d iterations, seed=%d\n", 
           iterations, seed);
    
    uint64_t total_checksum = 0;
    
    /* Run all test functions */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    printf("Total checksum: %lu\n", total_checksum);
    
    return 0;
}

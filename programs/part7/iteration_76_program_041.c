/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow algorithm fixup graph debug output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MANY_VARS 30

/* Prevent inlining and optimization */
__attribute__((noinline, noipa))
unsigned long test_irreducible_goto(int iterations, unsigned seed) {
    /* Many local variables to create register pressure */
    volatile int a0 = seed + 1, a1 = seed + 2, a2 = seed + 3, a3 = seed + 4;
    volatile int a4 = seed + 5, a5 = seed + 6, a6 = seed + 7, a7 = seed + 8;
    volatile int a8 = seed + 9, a9 = seed + 10, a10 = seed + 11, a11 = seed + 12;
    volatile float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f, f3 = seed * 0.4f;
    volatile double d0 = seed * 0.01, d1 = seed * 0.02, d2 = seed * 0.03, d3 = seed * 0.04;
    volatile long l0 = seed * 100, l1 = seed * 200, l2 = seed * 300, l3 = seed * 400;
    volatile int b0 = 0, b1 = 0, b2 = 0, b3 = 0, b4 = 0, b5 = 0, b6 = 0, b7 = 0;
    
    unsigned long checksum = 0;
    
    /* Create irreducible loop with goto jumping across boundaries */
    int i = 0;
    outer_loop_start:
    while (i < iterations) {
        /* Complex arithmetic to keep variables live */
        a0 = a1 + a2 * a3 - a4;
        a1 = a5 ^ a6 | a7 & a8;
        a2 = a9 * a10 / (a11 + 1);
        f0 = f1 * f2 + f3;
        d0 = d1 - d2 * d3;
        l0 = l1 << 2 | l2 >> 3;
        
        /* Force register usage with inline asm */
        asm volatile("" : "+r"(a0), "+r"(a1), "+r"(a2));
        asm volatile("" : "+r"(f0), "+r"(f1));
        asm volatile("" : "+r"(d0), "+r"(d1));
        
        if (i % 7 == 0) {
            goto inner_label_1;
        } else if (i % 7 == 1) {
            goto inner_label_2;
        } else if (i % 7 == 2) {
            goto outer_label;
        }
        
        continue_after_goto:
        a3 = a4 + a5 - a6;
        a4 = a7 * a8 / a9;
        f2 = f3 * 2.0f - f0;
        d2 = d3 + 1.0 / d0;
        l1 = l2 + l3 * 2;
        
        i++;
        if (i % 13 == 0) {
            goto outer_loop_start;  /* Jump back to while start */
        }
        continue;
        
        inner_label_1:
        a5 = a6 ^ a7 | a8;
        a6 = a9 + a10 * a11;
        f3 = f0 / f1 + f2;
        d3 = d0 - d1 * d2;
        l2 = l3 << 1 | l0;
        goto continue_after_goto;
        
        inner_label_2:
        a7 = a8 * a9 - a10;
        a8 = a11 & a0 | a1;
        f0 = f1 + f2 * f3;
        d0 = d1 / (d2 + 0.5);
        l3 = l0 + l1 - l2;
        goto continue_after_goto;
        
        outer_label:
        a9 = a10 + a11 * a0;
        a10 = a1 ^ a2 | a3;
        f1 = f2 - f3 * f0;
        d1 = d2 + d3 / 2.0;
        l0 = l1 | l2 & l3;
        goto outer_loop_start;  /* Jump to while start, skipping increment */
    }
    
    /* Aggregate checksum */
    checksum = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11;
    checksum += (unsigned long)f0 + (unsigned long)f1 + (unsigned long)f2 + (unsigned long)f3;
    checksum += (unsigned long)d0 + (unsigned long)d1 + (unsigned long)d2 + (unsigned long)d3;
    checksum += l0 + l1 + l2 + l3;
    
    return checksum;
}

__attribute__((noinline, noipa))
unsigned long test_switch_goto(int iterations, unsigned seed) {
    volatile int x0 = seed, x1 = seed * 2, x2 = seed * 3, x3 = seed * 4;
    volatile int x4 = seed * 5, x5 = seed * 6, x6 = seed * 7, x7 = seed * 8;
    volatile float y0 = seed * 0.11f, y1 = seed * 0.22f, y2 = seed * 0.33f;
    volatile double z0 = seed * 0.011, z1 = seed * 0.022, z2 = seed * 0.033;
    volatile long w0 = seed * 111, w1 = seed * 222, w2 = seed * 333;
    
    unsigned long checksum = 0;
    int state = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex arithmetic chains */
        x0 = x1 * x2 - x3 + x4;
        x1 = x5 ^ x6 | x7 & x0;
        x2 = x3 * x4 / (x5 + 1);
        y0 = y1 * 1.5f + y2;
        z0 = z1 - z2 * 0.5;
        w0 = w1 << 1 | w2 >> 2;
        
        /* Force register usage */
        asm volatile("" : "+r"(x0), "+r"(x1), "+r"(x2), "+r"(x3));
        asm volatile("" : "+r"(y0), "+r"(y1));
        asm volatile("" : "+r"(z0), "+r"(z1));
        
        /* Switch with goto creating irreducible flow */
        switch (state % 5) {
            case 0:
                x3 = x4 + x5 * x6;
                if (i % 3 == 0) goto label_a;
                else goto label_b;
            case 1:
                x4 = x5 ^ x6 | x7;
                goto label_c;
            case 2:
                x5 = x6 * x7 - x0;
                if (i % 2 == 0) goto label_d;
                break;
            case 3:
                x6 = x7 + x0 * x1;
                goto label_a;
            case 4:
                x7 = x0 & x1 | x2;
                goto label_b;
        }
        
        continue_switch:
        y1 = y2 * 2.0f - y0;
        z1 = z2 + 1.0 / z0;
        w1 = w2 + w0 * 3;
        state = (state * 7 + i) % 100;
        continue;
        
        label_a:
        x0 = x1 + x2 - x3;
        y2 = y0 / y1 + 0.5f;
        goto continue_switch;
        
        label_b:
        x1 = x2 * x3 / x4;
        z2 = z0 - z1 * 0.25;
        goto continue_switch;
        
        label_c:
        x2 = x3 ^ x4 | x5;
        w2 = w0 << 2 | w1;
        goto continue_switch;
        
        label_d:
        x3 = x4 + x5 * x6;
        y0 = y1 + y2 * 0.75f;
        goto continue_switch;
    }
    
    checksum = x0 + x1 + x2 + x3 + x4 + x5 + x6 + x7;
    checksum += (unsigned long)y0 + (unsigned long)y1 + (unsigned long)y2;
    checksum += (unsigned long)z0 + (unsigned long)z1 + (unsigned long)z2;
    checksum += w0 + w1 + w2;
    
    return checksum;
}

__attribute__((noinline, noipa))
unsigned long test_computed_goto(int iterations, unsigned seed) {
    volatile int v0 = seed, v1 = seed + 1, v2 = seed + 2, v3 = seed + 3;
    volatile int v4 = seed + 4, v5 = seed + 5, v6 = seed + 6, v7 = seed + 7;
    volatile int v8 = seed + 8, v9 = seed + 9, v10 = seed + 10, v11 = seed + 11;
    volatile float r0 = seed * 0.05f, r1 = seed * 0.15f, r2 = seed * 0.25f;
    volatile double s0 = seed * 0.005, s1 = seed * 0.015, s2 = seed * 0.025;
    
    /* Labels for computed goto */
    static void* labels[] = {
        &&state0, &&state1, &&state2, &&state3, 
        &&state4, &&state5, &&state6, &&state7
    };
    
    unsigned long checksum = 0;
    int state = seed % 8;
    
    for (int i = 0; i < iterations; i++) {
        /* Long dependency chains */
        v0 = v1 * v2 - v3 + v4;
        v1 = v5 ^ v6 | v7 & v8;
        v2 = v9 * v10 / (v11 + 1);
        v3 = v4 + v5 - v6 * v7;
        v4 = v8 & v9 | v10 ^ v11;
        r0 = r1 * 1.1f + r2;
        r1 = r2 - r0 * 0.5f;
        s0 = s1 / (s2 + 0.001);
        s1 = s2 * 2.0 - s0;
        
        /* Force all variables to be register-allocated */
        asm volatile("" : "+r"(v0), "+r"(v1), "+r"(v2), "+r"(v3));
        asm volatile("" : "+r"(v4), "+r"(v5), "+r"(v6), "+r"(v7));
        asm volatile("" : "+r"(r0), "+r"(r1), "+r"(r2));
        asm volatile("" : "+r"(s0), "+r"(s1), "+r"(s2));
        
        /* Computed goto - creates complex CFG */
        goto *labels[state];
        
        state0:
        v5 = v6 + v7 * v8;
        r2 = r0 / r1 + 0.3f;
        state = (state + 1) % 8;
        continue;
        
        state1:
        v6 = v7 ^ v8 | v9;
        s2 = s0 - s1 * 0.1;
        state = (state + 3) % 8;
        continue;
        
        state2:
        v7 = v8 * v9 - v10;
        v8 = v11 & v0 | v1;
        state = (state + 5) % 8;
        continue;
        
        state3:
        v9 = v10 + v11 * v0;
        r0 = r1 * 2.0f - r2;
        state = (state + 2) % 8;
        continue;
        
        state4:
        v10 = v11 ^ v0 | v1;
        s0 = s1 + s2 / 3.0;
        state = (state + 7) % 8;
        continue;
        
        state5:
        v11 = v0 * v1 - v2;
        r1 = r2 + r0 * 0.25f;
        state = (state + 4) % 8;
        continue;
        
        state6:
        v0 = v1 + v2 - v3;
        s1 = s2 * 0.5 - s0;
        state = (state + 6) % 8;
        continue;
        
        state7:
        v1 = v2 * v3 / v4;
        r2 = r0 - r1 * 0.75f;
        state = (state + 1) % 8;
        continue;
    }
    
    checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11;
    checksum += (unsigned long)r0 + (unsigned long)r1 + (unsigned long)r2;
    checksum += (unsigned long)s0 + (unsigned long)s1 + (unsigned long)s2;
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int iterations = 10000;
    unsigned seed = 42;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10000;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Running MCF coverage test with %d iterations, seed=%u\n", 
           iterations, seed);
    
    unsigned long total_checksum = 0;
    
    /* Run all test functions to increase coverage chance */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    printf("Total checksum: %lu\n", total_checksum);
    
    return 0;
}

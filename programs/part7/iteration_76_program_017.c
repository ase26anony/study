/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow algorithm
   with fixup graph creation and debug output */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* Function 1: Irreducible loops with goto jumping across boundaries */
NOINLINE static unsigned long test_irreducible_goto(int iterations, int seed) {
    /* Declare many variables to increase register pressure */
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
    VOLATILE_VAR float f0 = seed * 0.1f;
    VOLATILE_VAR float f1 = seed * 0.2f;
    VOLATILE_VAR float f2 = seed * 0.3f;
    VOLATILE_VAR float f3 = seed * 0.4f;
    VOLATILE_VAR double d0 = seed * 0.01;
    VOLATILE_VAR double d1 = seed * 0.02;
    VOLATILE_VAR double d2 = seed * 0.03;
    VOLATILE_VAR long l0 = seed * 100L;
    VOLATILE_VAR long l1 = seed * 200L;
    VOLATILE_VAR long l2 = seed * 300L;
    VOLATILE_VAR long l3 = seed * 400L;
    VOLATILE_VAR long l4 = seed * 500L;
    VOLATILE_VAR long l5 = seed * 600L;
    VOLATILE_VAR long l6 = seed * 700L;
    VOLATILE_VAR long l7 = seed * 800L;
    VOLATILE_VAR long l8 = seed * 900L;
    VOLATILE_VAR long l9 = seed * 1000L;
    
    unsigned long checksum = 0;
    int i = 0;
    
    /* Outer loop */
    while (i < iterations) {
        /* Complex arithmetic to keep variables live */
        v0 = v1 + v2;
        v1 = v2 * v3;
        v2 = v3 - v4;
        v3 = v4 ^ v5;
        v4 = v5 | v6;
        v5 = v6 & v7;
        v6 = v7 << 2;
        v7 = v8 >> 1;
        v8 = v9 + i;
        v9 = v0 * i;
        
        f0 = f1 * 1.1f;
        f1 = f2 + 2.2f;
        f2 = f3 - 3.3f;
        f3 = f0 * 0.5f;
        
        d0 = d1 * 1.01;
        d1 = d2 + 2.02;
        d2 = d0 - 3.03;
        
        l0 = l1 + l2;
        l1 = l2 * l3;
        l2 = l3 - l4;
        l3 = l4 ^ l5;
        l4 = l5 | l6;
        l5 = l6 & l7;
        l6 = l7 << 3;
        l7 = l8 >> 2;
        l8 = l9 + i;
        l9 = l0 * i;
        
        /* Use inline assembly to mark variables as used */
        asm volatile("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4));
        asm volatile("" : : "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9));
        asm volatile("" : : "r"(f0), "r"(f1), "r"(f2), "r"(f3));
        asm volatile("" : : "r"(d0), "r"(d1), "r"(d2));
        asm volatile("" : : "r"(l0), "r"(l1), "r"(l2), "r"(l3), "r"(l4));
        asm volatile("" : : "r"(l5), "r"(l6), "r"(l7), "r"(l8), "r"(l9));
        
        /* Irreducible control flow with goto */
        if (i % 7 == 0) {
            goto inner_loop_start;
        } else if (i % 11 == 0) {
            goto middle_label;
        }
        
        i++;
        continue;
        
    inner_loop_start:
        /* Inner loop that can be entered from multiple points */
        for (int j = 0; j < 5; j++) {
            v0 += j;
            if (j % 2 == 0) {
                goto middle_label;
            }
        }
        
        i++;
        continue;
        
    middle_label:
        /* Middle block that can be reached from multiple places */
        v1 += v2;
        if (i % 13 == 0) {
            goto outer_loop_end;
        }
        
        i++;
        continue;
        
    outer_loop_end:
        v2 += v3;
        i++;
    }
    
    /* Aggregate checksum */
    checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
               (unsigned long)f0 + (unsigned long)f1 + (unsigned long)f2 + (unsigned long)f3 +
               (unsigned long)d0 + (unsigned long)d1 + (unsigned long)d2 +
               l0 + l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9;
    
    return checksum;
}

/* Function 2: Switch with goto creating complex CFG */
NOINLINE static unsigned long test_switch_goto(int iterations, int seed) {
    VOLATILE_VAR int a0 = seed, a1 = seed + 1, a2 = seed + 2, a3 = seed + 3;
    VOLATILE_VAR int a4 = seed + 4, a5 = seed + 5, a6 = seed + 6, a7 = seed + 7;
    VOLATILE_VAR int a8 = seed + 8, a9 = seed + 9, a10 = seed + 10, a11 = seed + 11;
    VOLATILE_VAR float b0 = seed * 0.5f, b1 = seed * 1.5f, b2 = seed * 2.5f;
    VOLATILE_VAR double c0 = seed * 0.25, c1 = seed * 0.75, c2 = seed * 1.25;
    VOLATILE_VAR long d0 = seed * 1000L, d1 = seed * 2000L, d2 = seed * 3000L;
    VOLATILE_VAR long d3 = seed * 4000L, d4 = seed * 5000L, d5 = seed * 6000L;
    
    unsigned long checksum = 0;
    int state = seed % 5;
    
    for (int i = 0; i < iterations; i++) {
        /* Long dependency chain */
        a0 = a1 + a2;
        a1 = a2 * a3;
        a2 = a3 - a4;
        a3 = a4 ^ a5;
        a4 = a5 | a6;
        a5 = a6 & a7;
        a6 = a7 << (i % 4);
        a7 = a8 >> 1;
        a8 = a9 + a10;
        a9 = a10 * a11;
        a10 = a11 - a0;
        a11 = a0 ^ a1;
        
        b0 = b1 * 1.1f + i;
        b1 = b2 + 2.2f - i;
        b2 = b0 * 0.9f + i * 0.1f;
        
        c0 = c1 * 1.01 + i * 0.01;
        c1 = c2 + 2.02 - i * 0.01;
        c2 = c0 * 0.99 + i * 0.02;
        
        d0 = d1 + d2;
        d1 = d2 * d3;
        d2 = d3 - d4;
        d3 = d4 ^ d5;
        d4 = d5 | d0;
        d5 = d0 & d1;
        
        /* Mark variables as used */
        asm volatile("" : : 
            "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
            "r"(a6), "r"(a7), "r"(a8), "r"(a9), "r"(a10), "r"(a11),
            "r"(b0), "r"(b1), "r"(b2),
            "r"(c0), "r"(c1), "r"(c2),
            "r"(d0), "r"(d1), "r"(d2), "r"(d3), "r"(d4), "r"(d5)
        );
        
        /* Switch with goto creating irreducible regions */
        switch (state) {
            case 0:
                a0 += i;
                if (i % 3 == 0) goto label_x;
                state = 1;
                break;
            case 1:
                a1 -= i;
                if (i % 5 == 0) goto label_y;
                state = 2;
                break;
            case 2:
                a2 *= i;
                if (i % 7 == 0) goto label_z;
                state = 3;
                break;
            case 3:
                a3 ^= i;
                if (i % 11 == 0) goto label_x;
                state = 4;
                break;
            case 4:
                a4 |= i;
                if (i % 13 == 0) goto label_y;
                state = 0;
                break;
        }
        continue;
        
    label_x:
        a5 += i * 2;
        state = (state + 1) % 5;
        continue;
        
    label_y:
        a6 -= i * 3;
        state = (state + 2) % 5;
        continue;
        
    label_z:
        a7 *= i * 4;
        state = (state + 3) % 5;
        continue;
    }
    
    checksum = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11 +
               (unsigned long)b0 + (unsigned long)b1 + (unsigned long)b2 +
               (unsigned long)c0 + (unsigned long)c1 + (unsigned long)c2 +
               d0 + d1 + d2 + d3 + d4 + d5;
    
    return checksum;
}

/* Function 3: Computed goto simulating state machine */
NOINLINE static unsigned long test_computed_goto(int iterations, int seed) {
    VOLATILE_VAR int x0 = seed, x1 = seed + 100, x2 = seed + 200, x3 = seed + 300;
    VOLATILE_VAR int x4 = seed + 400, x5 = seed + 500, x6 = seed + 600, x7 = seed + 700;
    VOLATILE_VAR int x8 = seed + 800, x9 = seed + 900, x10 = seed + 1000;
    VOLATILE_VAR float y0 = seed * 0.33f, y1 = seed * 0.66f, y2 = seed * 0.99f;
    VOLATILE_VAR double z0 = seed * 0.111, z1 = seed * 0.222, z2 = seed * 0.333;
    VOLATILE_VAR long w0 = seed * 1111L, w1 = seed * 2222L, w2 = seed * 3333L;
    VOLATILE_VAR long w3 = seed * 4444L, w4 = seed * 5555L, w5 = seed * 6666L;
    
    /* Labels for computed goto */
    static void* labels[] = {
        &&state_a, &&state_b, &&state_c, &&state_d, &&state_e,
        &&state_f, &&state_g, &&state_h
    };
    
    unsigned long checksum = 0;
    int state = seed % 8;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex arithmetic operations */
        x0 = x1 + x2;
        x1 = x2 * x3;
        x2 = x3 - x4;
        x3 = x4 ^ x5;
        x4 = x5 | x6;
        x5 = x6 & x7;
        x6 = x7 << (i % 5);
        x7 = x8 >> (i % 3);
        x8 = x9 + x10;
        x9 = x10 * x0;
        x10 = x0 - x1;
        
        y0 = y1 * 1.5f + i * 0.1f;
        y1 = y2 + 2.5f - i * 0.2f;
        y2 = y0 * 0.8f + i * 0.3f;
        
        z0 = z1 * 1.11 + i * 0.01;
        z1 = z2 + 2.22 - i * 0.02;
        z2 = z0 * 0.89 + i * 0.03;
        
        w0 = w1 + w2;
        w1 = w2 * w3;
        w2 = w3 - w4;
        w3 = w4 ^ w5;
        w4 = w5 | w0;
        w5 = w0 & w1;
        
        /* Mark all variables as used */
        asm volatile("" : :
            "r"(x0), "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5),
            "r"(x6), "r"(x7), "r"(x8), "r"(x9), "r"(x10),
            "r"(y0), "r"(y1), "r"(y2),
            "r"(z0), "r"(z1), "r"(z2),
            "r"(w0), "r"(w1), "r"(w2), "r"(w3), "r"(w4), "r"(w5)
        );
        
        /* Computed goto - creates very complex CFG */
        goto *labels[state];
        
    state_a:
        x0 += i * 11;
        state = (state + (i % 3)) % 8;
        continue;
        
    state_b:
        x1 -= i * 13;
        state = (state + (i % 5)) % 8;
        continue;
        
    state_c:
        x2 *= i * 17;
        state = (state + (i % 7)) % 8;
        continue;
        
    state_d:
        x3 ^= i * 19;
        state = (state + (i % 11)) % 8;
        continue;
        
    state_e:
        x4 |= i * 23;
        state = (state + (i % 13)) % 8;
        continue;
        
    state_f:
        x5 &= i * 29;
        state = (state + (i % 17)) % 8;
        continue;
        
    state_g:
        x6 <<= (i % 4);
        state = (state + (i % 19)) % 8;
        continue;
        
    state_h:
        x7 >>= (i % 3);
        state = (state + (i % 23)) % 8;
        continue;
    }
    
    checksum = x0 + x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10 +
               (unsigned long)y0 + (unsigned long)y1 + (unsigned long)y2 +
               (unsigned long)z0 + (unsigned long)z1 + (unsigned long)z2 +
               w0 + w1 + w2 + w3 + w4 + w5;
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int iterations = 10000;
    int seed = time(NULL);
    unsigned long total_checksum = 0;
    
    /* Parse command line for iteration count */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) {
            iterations = 10000;
        }
    }
    
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Running MCF coverage test with %d iterations, seed=%d\n", 
           iterations, seed);
    
    /* Run all test functions with different patterns */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    printf("Total checksum: %lu\n", total_checksum);
    
    return 0;
}

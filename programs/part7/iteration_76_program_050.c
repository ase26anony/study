/* mcf_coverage.c - Program to trigger GCC's MCF fixup graph debug output */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_VARS 30
#define DEFAULT_ITERATIONS 10000

/* Prevent optimization of control flow and variables */
#define NOOPT __attribute__((noinline,optimize("no-goto")))
#define VOLATILE_VAR volatile

/* Function 1: Irreducible loops with goto jumping across loop boundaries */
NOOPT unsigned long long test_irreducible_goto(int iterations, int seed) {
    /* Declare many variables to increase register pressure */
    VOLATILE_VAR int a0 = seed + 1, a1 = seed + 2, a2 = seed + 3, a3 = seed + 4;
    VOLATILE_VAR int a4 = seed + 5, a5 = seed + 6, a6 = seed + 7, a7 = seed + 8;
    VOLATILE_VAR int a8 = seed + 9, a9 = seed + 10, a10 = seed + 11, a11 = seed + 12;
    VOLATILE_VAR float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f;
    VOLATILE_VAR double d0 = seed * 0.01, d1 = seed * 0.02, d2 = seed * 0.03;
    VOLATILE_VAR long l0 = seed * 100L, l1 = seed * 200L, l2 = seed * 300L;
    VOLATILE_VAR unsigned long long checksum = 0;
    
    /* Labels for irreducible control flow */
    outer_loop_start:
    for (int i = 0; i < iterations; i++) {
        /* Complex arithmetic creating register pressure */
        a0 = a1 + a2 * a3 - a4;
        a1 = a5 ^ a6 | a7 & a8;
        a2 = a9 * a10 / (a11 + 1);
        f0 = f1 * f2 + (float)a0;
        d0 = d1 - d2 * (double)a1;
        l0 = l1 << (a2 & 3);
        
        /* Irreducible loop pattern using goto */
        if ((i + seed) % 7 == 0) {
            goto inner_block_a;
        } else if ((i + seed) % 11 == 0) {
            goto inner_block_b;
        }
        
        continue_outer:
        /* More arithmetic to keep variables live */
        a3 = a4 + a5 - a6;
        a4 = a7 * a8 % (a9 + 1);
        f1 = f0 * 2.0f - f2;
        d1 = d0 / 2.0 + d2;
        l1 = l0 >> 1 | l2;
        
        /* Use inline assembly to mark variables as used */
        asm volatile("" : : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4));
        asm volatile("" : : "r"(f0), "r"(f1), "r"(f2));
        asm volatile("" : : "r"(d0), "r"(d1), "r"(d2));
        asm volatile("" : : "r"(l0), "r"(l1), "r"(l2));
        
        checksum += a0 + a1 + a2 + a3 + a4 + (unsigned long long)f0 + 
                   (unsigned long long)d0 + l0 + l1 + l2;
        
        /* Jump back to outer loop start occasionally */
        if (i % 13 == 0) {
            goto outer_loop_start;
        }
        continue;
        
        inner_block_a:
        a5 = a6 + a7 * a8;
        a6 = a9 ^ a10 & a11;
        f2 = f0 + f1 * 3.0f;
        d2 = d0 - d1 / 4.0;
        l2 = l0 & l1 | 0xFF;
        
        /* Jump to different block creating irreducible region */
        if (i % 3 == 0) {
            goto inner_block_b;
        } else {
            goto continue_outer;
        }
        
        inner_block_b:
        a7 = a8 - a9 + a10;
        a8 = a11 * a0 % (a1 + 1);
        a9 = a2 + a3 - a4;
        f0 = f1 / f2 + 1.0f;
        d0 = d1 * 2.0 - d2;
        l0 = l1 ^ l2;
        
        /* Jump back to block A or continue */
        if (i % 5 == 0) {
            goto inner_block_a;
        } else {
            goto continue_outer;
        }
    }
    
    return checksum;
}

/* Function 2: Switch with goto creating complex CFG */
NOOPT unsigned long long test_switch_goto(int iterations, int seed) {
    VOLATILE_VAR int b0 = seed, b1 = seed * 2, b2 = seed * 3, b3 = seed * 4;
    VOLATILE_VAR int b4 = seed * 5, b5 = seed * 6, b6 = seed * 7, b7 = seed * 8;
    VOLATILE_VAR int b8 = seed * 9, b9 = seed * 10, b10 = seed * 11, b11 = seed * 12;
    VOLATILE_VAR float g0 = seed * 0.15f, g1 = seed * 0.25f, g2 = seed * 0.35f;
    VOLATILE_VAR double e0 = seed * 0.05, e1 = seed * 0.15, e2 = seed * 0.25;
    VOLATILE_VAR long m0 = seed * 150L, m1 = seed * 250L, m2 = seed * 350L;
    VOLATILE_VAR unsigned long long checksum = 0;
    
    /* Labels for switch goto targets */
    switch_label_a:
    switch_label_b:
    switch_label_c:
    switch_label_d:
    
    for (int i = 0; i < iterations; i++) {
        int mod = (i + seed) % 20;
        
        /* Switch with goto to different labels */
        switch (mod) {
            case 0:
            case 1:
            case 2:
                b0 = b1 + b2;
                b1 = b3 * b4;
                g0 = g1 + g2;
                goto switch_label_a;
                
            case 3:
            case 4:
            case 5:
                b2 = b5 - b6;
                b3 = b7 / (b8 + 1);
                g1 = g0 * g2;
                goto switch_label_b;
                
            case 6:
            case 7:
            case 8:
                b4 = b9 ^ b10;
                b5 = b11 & b0;
                g2 = g1 - g0;
                goto switch_label_c;
                
            case 9:
            case 10:
            case 11:
                b6 = b1 | b2;
                b7 = b3 % (b4 + 1);
                e0 = e1 + e2;
                goto switch_label_d;
                
            default:
                b8 = b5 + b6;
                b9 = b7 * b8;
                e1 = e0 * e2;
                /* Fall through to label A */
                goto switch_label_a;
        }
        
        /* Code at each label with arithmetic */
        switch_label_a:
        b10 = b0 * b1 + b2;
        b11 = b3 - b4 * b5;
        e2 = e0 / (e1 + 1.0);
        m0 = m1 << (b0 & 3);
        checksum += b0 + b10 + (unsigned long long)g0 + (unsigned long long)e0 + m0;
        asm volatile("" : : "r"(b0), "r"(b10), "r"(g0), "r"(e0), "r"(m0));
        
        switch_label_b:
        b0 = b6 + b7 - b8;
        b1 = b9 ^ b10 & b11;
        e0 = e1 - e2 * 2.0;
        m1 = m0 >> 2 | m2;
        checksum += b1 + b6 + (unsigned long long)g1 + (unsigned long long)e1 + m1;
        asm volatile("" : : "r"(b1), "r"(b6), "r"(g1), "r"(e1), "r"(m1));
        
        switch_label_c:
        b2 = b8 * b9 % (b10 + 1);
        b3 = b11 + b0 - b1;
        e1 = e0 + e2 / 3.0;
        m2 = m1 & m0 ^ 0xAA;
        checksum += b2 + b7 + (unsigned long long)g2 + (unsigned long long)e2 + m2;
        asm volatile("" : : "r"(b2), "r"(b7), "r"(g2), "r"(e2), "r"(m2));
        
        switch_label_d:
        b4 = b2 / (b3 + 1) + b4;
        b5 = b5 | b6 & b7;
        e2 = e0 * e1 - e2;
        m0 = m2 + m1 * 2;
        checksum += b3 + b8 + b9 + b11 + m0;
        asm volatile("" : : "r"(b3), "r"(b8), "r"(b9), "r"(b11), "r"(m0));
        
        /* More arithmetic to increase register pressure */
        g0 = g1 * 1.5f + g2;
        g1 = g0 - g2 * 0.5f;
        g2 = g1 / (g0 + 0.1f);
        
        e0 = e1 + e2 * 0.25;
        e1 = e0 - e2 / 0.75;
        e2 = e1 * e0 + 1.0;
    }
    
    return checksum;
}

/* Function 3: Computed goto simulating state machine */
NOOPT unsigned long long test_computed_goto(int iterations, int seed) {
    VOLATILE_VAR int c0 = seed, c1 = seed + 100, c2 = seed + 200, c3 = seed + 300;
    VOLATILE_VAR int c4 = seed + 400, c5 = seed + 500, c6 = seed + 600, c7 = seed + 700;
    VOLATILE_VAR int c8 = seed + 800, c9 = seed + 900, c10 = seed + 1000, c11 = seed + 1100;
    VOLATILE_VAR float h0 = seed * 0.05f, h1 = seed * 0.15f, h2 = seed * 0.25f;
    VOLATILE_VAR double k0 = seed * 0.005, k1 = seed * 0.015, k2 = seed * 0.025;
    VOLATILE_VAR long n0 = seed * 50L, n1 = seed * 150L, n2 = seed * 250L;
    VOLATILE_VAR unsigned long long checksum = 0;
    
    /* Labels for computed goto */
    static void* labels[] = {
        &&state_a, &&state_b, &&state_c, 
        &&state_d, &&state_e, &&state_f
    };
    
    int state = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Update state based on complex condition */
        state = (state + (i * seed) % 6) % 6;
        
        /* Computed goto */
        goto *labels[state];
        
        state_a:
        c0 = c1 + c2 * c3 - c4;
        c1 = c5 ^ c6 | c7 & c8;
        h0 = h1 * h2 + (float)c0;
        k0 = k1 - k2 * (double)c1;
        n0 = n1 << (c2 & 3);
        checksum += c0 + c1 + (unsigned long long)h0 + (unsigned long long)k0 + n0;
        asm volatile("" : : "r"(c0), "r"(c1), "r"(h0), "r"(k0), "r"(n0));
        state = (state + 1) % 6;
        continue;
        
        state_b:
        c2 = c3 + c4 - c5;
        c3 = c6 * c7 % (c8 + 1);
        h1 = h0 * 2.0f - h2;
        k1 = k0 / 2.0 + k2;
        n1 = n0 >> 1 | n2;
        checksum += c2 + c3 + (unsigned long long)h1 + (unsigned long long)k1 + n1;
        asm volatile("" : : "r"(c2), "r"(c3), "r"(h1), "r"(k1), "r"(n1));
        state = (state + 2) % 6;
        continue;
        
        state_c:
        c4 = c5 + c6 * c7;
        c5 = c8 ^ c9 & c10;
        h2 = h0 + h1 * 3.0f;
        k2 = k0 - k1 / 4.0;
        n2 = n0 & n1 | 0xFF;
        checksum += c4 + c5 + (unsigned long long)h2 + (unsigned long long)k2 + n2;
        asm volatile("" : : "r"(c4), "r"(c5), "r"(h2), "r"(k2), "r"(n2));
        state = (state + 3) % 6;
        continue;
        
        state_d:
        c6 = c7 - c8 + c9;
        c7 = c10 * c0 % (c1 + 1);
        h0 = h1 / h2 + 1.0f;
        k0 = k1 * 2.0 - k2;
        n0 = n1 ^ n2;
        checksum += c6 + c7 + (unsigned long long)h0 + (unsigned long long)k0 + n0;
        asm volatile("" : : "r"(c6), "r"(c7), "r"(h0), "r"(k0), "r"(n0));
        state = (state + 4) % 6;
        continue;
        
        state_e:
        c8 = c9 + c10 - c11;
        c9 = c0 * c1 / (c2 + 1);
        h1 = h2 * 1.5f - h0;
        k1 = k2 + k0 * 0.5;
        n1 = n2 << 2 & n0;
        checksum += c8 + c9 + (unsigned long long)h1 + (unsigned long long)k1 + n1;
        asm volatile("" : : "r"(c8), "r"(c9), "r"(h1), "r"(k1), "r"(n1));
        state = (state + 5) % 6;
        continue;
        
        state_f:
        c10 = c11 * c2 + c3;
        c11 = c4 ^ c5 | c6;
        h2 = h0 - h1 / 2.0f;
        k2 = k1 * k0 + 1.0;
        n2 = n0 >> 3 | n1;
        checksum += c10 + c11 + (unsigned long long)h2 + (unsigned long long)k2 + n2;
        asm volatile("" : : "r"(c10), "r"(c11), "r"(h2), "r"(k2), "r"(n2));
        state = 0;
        continue;
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int iterations = DEFAULT_ITERATIONS;
    int seed = 42;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) {
            iterations = DEFAULT_ITERATIONS;
        }
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Running MCF coverage test with %d iterations, seed=%d\n", 
           iterations, seed);
    
    /* Call all test functions to trigger different CFG patterns */
    unsigned long long checksum1 = test_irreducible_goto(iterations, seed);
    unsigned long long checksum2 = test_switch_goto(iterations, seed + 1);
    unsigned long long checksum3 = test_computed_goto(iterations, seed + 2);
    
    unsigned long long total = checksum1 + checksum2 + checksum3;
    
    printf("Checksum1: %llu\n", checksum1);
    printf("Checksum2: %llu\n", checksum2);
    printf("Checksum3: %llu\n", checksum3);
    printf("Total checksum: %llu\n", total);
    
    return (total > 0) ? 0 : 1;
}

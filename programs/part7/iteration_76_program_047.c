/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow algorithm
   with fixup graph creation and debug output */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* Function 1: Irreducible loops with goto jumping across loop boundaries */
NOINLINE static unsigned long test_irreducible_goto(int iterations, int seed) {
    /* Declare many local variables to increase register pressure */
    VOLATILE_VAR int a0 = seed + 1, a1 = seed + 2, a2 = seed + 3, a3 = seed + 4;
    VOLATILE_VAR int a4 = seed + 5, a5 = seed + 6, a6 = seed + 7, a7 = seed + 8;
    VOLATILE_VAR int a8 = seed + 9, a9 = seed + 10, a10 = seed + 11, a11 = seed + 12;
    VOLATILE_VAR float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f;
    VOLATILE_VAR float f3 = seed * 0.4f, f4 = seed * 0.5f, f5 = seed * 0.6f;
    VOLATILE_VAR double d0 = seed * 0.01, d1 = seed * 0.02, d2 = seed * 0.03;
    VOLATILE_VAR double d3 = seed * 0.04, d4 = seed * 0.05;
    VOLATILE_VAR long l0 = seed * 100L, l1 = seed * 200L, l2 = seed * 300L;
    VOLATILE_VAR long l3 = seed * 400L, l4 = seed * 500L, l5 = seed * 600L;
    VOLATILE_VAR long l6 = seed * 700L, l7 = seed * 800L, l8 = seed * 900L;
    
    unsigned long checksum = 0;
    int i = 0;
    
    /* Outer loop */
    outer_loop_start:
    if (i >= iterations) goto outer_loop_end;
    
    /* Complex arithmetic to keep variables live */
    a0 = a1 + a2; a1 = a3 * a4; a2 = a5 - a6; a3 = a7 / (a8 ? a8 : 1);
    a4 = a9 ^ a10; a5 = a11 | a0; a6 = a1 & a2; a7 = a3 % (a4 ? a4 : 1);
    f0 = f1 + f2; f1 = f3 * f4; f2 = f5 - f0; f3 = f1 / (f2 + 1.0f);
    d0 = d1 + d2; d1 = d3 * d4; d2 = d0 - d1; d3 = d2 / (d4 + 1.0);
    l0 = l1 + l2; l1 = l3 * l4; l2 = l5 - l6; l3 = l7 / (l8 ? l8 : 1);
    l4 = l0 ^ l1; l5 = l2 | l3; l6 = l4 & l5; l7 = l6 % (l0 ? l0 : 1);
    
    /* Create irreducible region with goto jumping into inner loop */
    if ((i & 3) == 0) {
        goto inner_loop_middle;  /* Jump into middle of inner loop */
    }
    
    /* Inner loop */
    int j = 0;
    inner_loop_start:
    if (j >= 10) goto inner_loop_end;
    
    /* More arithmetic operations */
    a8 = a9 + a10; a9 = a11 * a0; a10 = a1 - a2; a11 = a3 / (a4 ? a4 : 1);
    f4 = f5 + f0; f5 = f1 * f2;
    d4 = d0 + d1;
    l8 = l0 + l1;
    
    inner_loop_middle:  /* Label jumped to from outside */
    if (j < 5) {
        /* Cross-loop variable usage */
        a0 = a8 + j; a1 = a9 * j; a2 = a10 - j;
        f0 = f4 + j; f1 = f5 * j;
        d0 = d4 + j; d1 = d0 * j;
        l0 = l8 + j; l1 = l0 * j;
    }
    
    j++;
    if ((j & 1) == 0) {
        goto inner_loop_start;
    } else {
        goto inner_loop_middle;  /* Another jump creating complexity */
    }
    
    inner_loop_end:
    
    /* Jump back to outer loop start from within */
    if ((i & 7) == 0) {
        i++;
        goto outer_loop_start;  /* Jump creating loop with multiple entries */
    }
    
    i++;
    if ((i & 5) == 0) {
        goto outer_loop_start;
    } else {
        goto skip_label;
    }
    
    skip_label:
    /* Force all variables to be used to prevent elimination */
    asm volatile("" : : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5));
    asm volatile("" : : "r"(a6), "r"(a7), "r"(a8), "r"(a9), "r"(a10), "r"(a11));
    asm volatile("" : : "r"(f0), "r"(f1), "r"(f2), "r"(f3), "r"(f4), "r"(f5));
    asm volatile("" : : "r"(d0), "r"(d1), "r"(d2), "r"(d3), "r"(d4));
    asm volatile("" : : "r"(l0), "r"(l1), "r"(l2), "r"(l3), "r"(l4));
    asm volatile("" : : "r"(l5), "r"(l6), "r"(l7), "r"(l8));
    
    checksum += a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11;
    checksum += (unsigned long)(f0 + f1 + f2 + f3 + f4 + f5);
    checksum += (unsigned long)(d0 + d1 + d2 + d3 + d4);
    checksum += l0 + l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8;
    
    goto outer_loop_start;
    
    outer_loop_end:
    return checksum;
}

/* Function 2: Switch with goto creating non-trivial control flow */
NOINLINE static unsigned long test_switch_goto(int iterations, int seed) {
    VOLATILE_VAR int b0 = seed, b1 = seed * 2, b2 = seed * 3, b3 = seed * 4;
    VOLATILE_VAR int b4 = seed * 5, b5 = seed * 6, b6 = seed * 7, b7 = seed * 8;
    VOLATILE_VAR int b8 = seed * 9, b9 = seed * 10, b10 = seed * 11, b11 = seed * 12;
    VOLATILE_VAR float g0 = seed * 0.15f, g1 = seed * 0.25f, g2 = seed * 0.35f;
    VOLATILE_VAR float g3 = seed * 0.45f, g4 = seed * 0.55f, g5 = seed * 0.65f;
    VOLATILE_VAR double e0 = seed * 0.015, e1 = seed * 0.025, e2 = seed * 0.035;
    VOLATILE_VAR double e3 = seed * 0.045, e4 = seed * 0.055;
    VOLATILE_VAR long m0 = seed * 150L, m1 = seed * 250L, m2 = seed * 350L;
    VOLATILE_VAR long m3 = seed * 450L, m4 = seed * 550L, m5 = seed * 650L;
    VOLATILE_VAR long m6 = seed * 750L, m7 = seed * 850L, m8 = seed * 950L;
    
    unsigned long checksum = 0;
    
    /* Labels for goto targets */
    switch_label_1:
    switch_label_2:
    switch_label_3:
    after_switch:
    
    for (int i = 0; i < iterations; i++) {
        /* Long dependency chain */
        b0 = b1 + b2; b1 = b3 * b4; b2 = b5 - b6; b3 = b7 / (b8 ? b8 : 1);
        b4 = b9 ^ b10; b5 = b11 | b0; b6 = b1 & b2; b7 = b3 % (b4 ? b4 : 1);
        b8 = b9 + b10; b9 = b11 * b0; b10 = b1 - b2; b11 = b3 / (b4 ? b4 : 1);
        
        g0 = g1 + g2; g1 = g3 * g4; g2 = g5 - g0; g3 = g1 / (g2 + 1.0f);
        g4 = g5 + g0; g5 = g1 * g2;
        
        e0 = e1 + e2; e1 = e3 * e4; e2 = e0 - e1; e3 = e2 / (e4 + 1.0);
        e4 = e0 + e1;
        
        m0 = m1 + m2; m1 = m3 * m4; m2 = m5 - m6; m3 = m7 / (m8 ? m8 : 1);
        m4 = m0 ^ m1; m5 = m2 | m3; m6 = m4 & m5; m7 = m6 % (m0 ? m0 : 1);
        m8 = m0 + m1;
        
        /* Switch with goto jumping to labels outside */
        switch (i & 7) {
            case 0:
                b0 += i;
                goto switch_label_1;  /* Jump to label outside switch */
            case 1:
                b1 += i;
                goto switch_label_2;
            case 2:
                b2 += i;
                /* Fall through */
            case 3:
                b3 += i;
                goto switch_label_3;
            case 4:
                b4 += i;
                /* Direct jump to after_switch */
                goto after_switch;
            case 5:
                b5 += i;
                /* Jump back to case 0 label */
                if ((i & 3) == 0) goto case_0_label;
                break;
            case 6:
                b6 += i;
                /* Jump to another case */
                if ((i & 1) == 0) goto case_2_label;
                break;
            case 7:
                b7 += i;
                break;
        }
        
        /* Continue normal flow */
        b8 += i;
        goto switch_continue;
        
        /* Labels for goto targets */
        case_0_label:
        b0 *= 2;
        goto switch_continue;
        
        case_2_label:
        b2 *= 2;
        goto switch_continue;
        
        switch_continue:
        
        /* Force variable usage */
        asm volatile("" : : "r"(b0), "r"(b1), "r"(b2), "r"(b3), "r"(b4), "r"(b5));
        asm volatile("" : : "r"(b6), "r"(b7), "r"(b8), "r"(b9), "r"(b10), "r"(b11));
        asm volatile("" : : "r"(g0), "r"(g1), "r"(g2), "r"(g3), "r"(g4), "r"(g5));
        asm volatile("" : : "r"(e0), "r"(e1), "r"(e2), "r"(e3), "r"(e4));
        asm volatile("" : : "r"(m0), "r"(m1), "r"(m2), "r"(m3), "r"(m4));
        asm volatile("" : : "r"(m5), "r"(m6), "r"(m7), "r"(m8));
        
        checksum += b0 + b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 + b11;
        checksum += (unsigned long)(g0 + g1 + g2 + g3 + g4 + g5);
        checksum += (unsigned long)(e0 + e1 + e2 + e3 + e4);
        checksum += m0 + m1 + m2 + m3 + m4 + m5 + m6 + m7 + m8;
        
        continue;
        
        after_switch:
        b9 += i;
        /* Jump back into the loop body */
        if ((i & 1) == 0) {
            goto switch_continue;
        }
    }
    
    return checksum;
}

/* Function 3: Computed goto simulating state machine */
NOINLINE static unsigned long test_computed_goto(int iterations, int seed) {
    VOLATILE_VAR int c0 = seed, c1 = seed * 3, c2 = seed * 5, c3 = seed * 7;
    VOLATILE_VAR int c4 = seed * 11, c5 = seed * 13, c6 = seed * 17, c7 = seed * 19;
    VOLATILE_VAR int c8 = seed * 23, c9 = seed * 29, c10 = seed * 31, c11 = seed * 37;
    VOLATILE_VAR float h0 = seed * 0.07f, h1 = seed * 0.11f, h2 = seed * 0.13f;
    VOLATILE_VAR float h3 = seed * 0.17f, h4 = seed * 0.19f, h5 = seed * 0.23f;
    VOLATILE_VAR double k0 = seed * 0.007, k1 = seed * 0.011, k2 = seed * 0.013;
    VOLATILE_VAR double k3 = seed * 0.017, k4 = seed * 0.019;
    VOLATILE_VAR long n0 = seed * 70L, n1 = seed * 110L, n2 = seed * 130L;
    VOLATILE_VAR long n3 = seed * 170L, n4 = seed * 190L, n5 = seed * 230L;
    VOLATILE_VAR long n6 = seed * 270L, n7 = seed * 310L, n8 = seed * 370L;
    
    unsigned long checksum = 0;
    
    /* Labels for computed goto */
    static void* labels[] = {
        &&state0, &&state1, &&state2, &&state3,
        &&state4, &&state5, &&state6, &&state7
    };
    
    int state = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex arithmetic operations */
        c0 = c1 + c2; c1 = c3 * c4; c2 = c5 - c6; c3 = c7 / (c8 ? c8 : 1);
        c4 = c9 ^ c10; c5 = c11 | c0; c6 = c1 & c2; c7 = c3 % (c4 ? c4 : 1);
        c8 = c9 + c10; c9 = c11 * c0; c10 = c1 - c2; c11 = c3 / (c4 ? c4 : 1);
        
        h0 = h1 + h2; h1 = h3 * h4; h2 = h5 - h0; h3 = h1 / (h2 + 1.0f);
        h4 = h5 + h0; h5 = h1 * h2;
        
        k0 = k1 + k2; k1 = k3 * k4; k2 = k0 - k1; k3 = k2 / (k4 + 1.0);
        k4 = k0 + k1;
        
        n0 = n1 + n2; n1 = n3 * n4; n2 = n5 - n6; n3 = n7 / (n8 ? n8 : 1);
        n4 = n0 ^ n1; n5 = n2 | n3; n6 = n4 & n5; n7 = n6 % (n0 ? n0 : 1);
        n8 = n0 + n1;
        
        /* Update state based on complex condition */
        state = (state + (i & 3) + (c0 & 1) + (c1 & 2)) & 7;
        
        /* Computed goto */
        goto *labels[state];
        
        state0:
        c0 += i * 2;
        h0 += i * 0.5f;
        k0 += i * 0.05;
        n0 += i * 20L;
        state = (state + 1) & 7;
        goto state_end;
        
        state1:
        c1 += i * 3;
        h1 += i * 0.7f;
        k1 += i * 0.07;
        n1 += i * 30L;
        if ((i & 5) == 0) goto state3;  /* Jump to another state */
        state = (state + 2) & 7;
        goto state_end;
        
        state2:
        c2 += i * 5;
        h2 += i * 1.1f;
        k2 += i * 0.11;
        n2 += i * 50L;
        state = (state + 3) & 7;
        goto state_end;
        
        state3:
        c3 += i * 7;
        h3 += i * 1.3f;
        k3 += i * 0.13;
        n3 += i * 70L;
        /* Jump back to state0 sometimes */
        if ((i & 7) == 0) goto state0;
        state = (state + 1) & 7;
        goto state_end;
        
        state4:
        c4 += i * 11;
        h4 += i * 1.7f;
        k4 += i * 0.17;
        n4 += i * 110L;
        state = (state + 2) & 7;
        goto state_end;
        
        state5:
        c5 += i * 13;
        h5 += i * 1.9f;
        n5 += i * 130L;
        /* Jump to state_end directly */
        goto state_end;
        
        state6:
        c6 += i * 17;
        n6 += i * 170L;
        state = (state + 1) & 7;
        goto state_end;
        
        state7:
        c7 += i * 19;
        n7 += i * 190L;
        state = (state + 3) & 7;
        /* Fall through to state_end */
        
        state_end:
        
        /* Force all variables to be live */
        asm volatile("" : : "r"(c0), "r"(c1), "r"(c2), "r"(c3), "r"(c4), "r"(c5));
        asm volatile("" : : "r"(c6), "r"(c7), "r"(c8), "r"(c9), "r"(c10), "r"(c11));
        asm volatile("" : : "r"(h0), "r"(h1), "r"(h2), "r"(h3), "r"(h4), "r"(h5));
        asm volatile("" : : "r"(k0), "r"(k1), "r"(k2), "r"(k3), "r"(k4));
        asm volatile("" : : "r"(n0), "r"(n1), "r"(n2), "r"(n3), "r"(n4));
        asm volatile("" : : "r"(n5), "r"(n6), "r"(n7), "r"(n8));
        
        checksum += c0 + c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8 + c9 + c10 + c11;
        checksum += (unsigned long)(h0 + h1 + h2 + h3 + h4 + h5);
        checksum += (unsigned long)(k0 + k1 + k2 + k3 + k4);
        checksum += n0 + n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8;
    }
    
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
    
    /* Call all test functions to create complex CFGs */
    unsigned long checksum1 = test_irreducible_goto(iterations, seed);
    printf("test_irreducible_goto checksum: %lu\n", checksum1);
    
    unsigned long checksum2 = test_switch_goto(iterations, seed + 1);
    printf("test_switch_goto checksum: %lu\n", checksum2);
    
    unsigned long checksum3 = test_computed_goto(iterations, seed + 2);
    printf("test_computed_goto checksum: %lu\n", checksum3);
    
    unsigned long total = checksum1 + checksum2 + checksum3;
    printf("Total checksum: %lu\n", total);
    
    return 0;
}

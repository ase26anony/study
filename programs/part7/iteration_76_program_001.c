/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow algorithm fixup graph debug output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_VARS 32
#define MAX_ITER 10000

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* Complex control flow test 1: Irreducible loops with goto */
NOINLINE static uint64_t test_irreducible_goto(int iterations, int seed) {
    VOLATILE_VAR int i, j;
    VOLATILE_VAR uint64_t checksum = seed;
    
    /* Many local variables to create register pressure */
    VOLATILE_VAR int a0 = 1, a1 = 2, a2 = 3, a3 = 4, a4 = 5;
    VOLATILE_VAR int b0 = 6, b1 = 7, b2 = 8, b3 = 9, b4 = 10;
    VOLATILE_VAR float f0 = 1.1f, f1 = 2.2f, f2 = 3.3f, f3 = 4.4f, f4 = 5.5f;
    VOLATILE_VAR double d0 = 1.01, d1 = 2.02, d2 = 3.03, d3 = 4.04, d4 = 5.05;
    VOLATILE_VAR long l0 = 100, l1 = 200, l2 = 300, l3 = 400, l4 = 500;
    VOLATILE_VAR int c0 = 11, c1 = 12, c2 = 13, c3 = 14, c4 = 15;
    VOLATILE_VAR int e0 = 16, e1 = 17, e2 = 18, e3 = 19, e4 = 20;
    
    /* Labels for irreducible control flow */
    loop_start:
    inner_loop:
    middle_block:
    exit_path:
    
    for (i = 0; i < iterations; i++) {
        /* Complex arithmetic chains to keep variables live */
        a0 = a1 + a2 * a3 - a4;
        a1 = a2 + a3 * a4 - a0;
        a2 = a3 + a4 * a0 - a1;
        a3 = a4 + a0 * a1 - a2;
        a4 = a0 + a1 * a2 - a3;
        
        b0 = b1 ^ b2 | b3 & b4;
        b1 = b2 ^ b3 | b4 & b0;
        b2 = b3 ^ b4 | b0 & b1;
        b3 = b4 ^ b0 | b1 & b2;
        b4 = b0 ^ b1 | b2 & b3;
        
        f0 = f1 * 1.1f + f2 - f3 / f4;
        f1 = f2 * 1.2f + f3 - f4 / f0;
        f2 = f3 * 1.3f + f4 - f0 / f1;
        f3 = f4 * 1.4f + f0 - f1 / f2;
        f4 = f0 * 1.5f + f1 - f2 / f3;
        
        d0 = d1 * 1.01 + d2 - d3 / d4;
        d1 = d2 * 1.02 + d3 - d4 / d0;
        d2 = d3 * 1.03 + d4 - d0 / d1;
        d3 = d4 * 1.04 + d0 - d1 / d2;
        d4 = d0 * 1.05 + d1 - d2 / d3;
        
        /* Irreducible control flow using goto */
        if ((i & 1) == 0) {
            goto inner_loop;
        }
        
        if ((i & 3) == 1) {
            l0 = l1 + l2 * l3 - l4;
            l1 = l2 + l3 * l4 - l0;
            l2 = l3 + l4 * l0 - l1;
            l3 = l4 + l0 * l1 - l2;
            l4 = l0 + l1 * l2 - l3;
            goto middle_block;
        }
        
        if ((i & 7) == 3) {
            c0 = c1 + c2 * c3 - c4;
            c1 = c2 + c3 * c4 - c0;
            c2 = c3 + c4 * c0 - c1;
            c3 = c4 + c0 * c1 - c2;
            c4 = c0 + c1 * c2 - c3;
            goto exit_path;
        }
        
        if ((i & 15) == 7) {
            e0 = e1 ^ e2 | e3 & e4;
            e1 = e2 ^ e3 | e4 & e0;
            e2 = e3 ^ e4 | e0 & e1;
            e3 = e4 ^ e0 | e1 & e2;
            e4 = e0 ^ e1 | e2 & e3;
            goto loop_start;
        }
        
        middle_block:
        /* More arithmetic to prevent dead code elimination */
        a0 = a0 ^ i;
        b0 = b0 + i;
        f0 = f0 + i;
        d0 = d0 + i;
        l0 = l0 ^ i;
        
        inner_loop:
        /* Cross-loop variable usage */
        a1 = a1 ^ (i >> 1);
        b1 = b1 + (i >> 1);
        
        exit_path:
        /* Final computation for checksum */
        checksum += a0 + b0 + (int)f0 + (int)d0 + l0 + c0 + e0;
        checksum = checksum * 1103515245 + 12345;
    }
    
    /* Use all variables in final computation to keep them live */
    asm volatile("" : : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4));
    asm volatile("" : : "r"(b0), "r"(b1), "r"(b2), "r"(b3), "r"(b4));
    asm volatile("" : : "r"(f0), "r"(f1), "r"(f2), "r"(f3), "r"(f4));
    asm volatile("" : : "r"(d0), "r"(d1), "r"(d2), "r"(d3), "r"(d4));
    
    return checksum;
}

/* Complex control flow test 2: Switch with goto */
NOINLINE static uint64_t test_switch_goto(int iterations, int seed) {
    VOLATILE_VAR int i;
    VOLATILE_VAR uint64_t checksum = seed;
    
    /* Another set of many local variables */
    VOLATILE_VAR int x0 = 1, x1 = 2, x2 = 3, x3 = 4, x4 = 5;
    VOLATILE_VAR int y0 = 6, y1 = 7, y2 = 8, y3 = 9, y4 = 10;
    VOLATILE_VAR float g0 = 1.5f, g1 = 2.5f, g2 = 3.5f, g3 = 4.5f, g4 = 5.5f;
    VOLATILE_VAR double h0 = 1.15, h1 = 2.25, h2 = 3.35, h3 = 4.45, h4 = 5.55;
    VOLATILE_VAR long m0 = 150, m1 = 250, m2 = 350, m3 = 450, m4 = 550;
    
    /* Labels for switch goto */
    case_a:
    case_b:
    case_c:
    case_d:
    switch_end:
    
    for (i = 0; i < iterations; i++) {
        /* Arithmetic chains */
        x0 = x1 + x2 * x3 - x4;
        x1 = x2 + x3 * x4 - x0;
        x2 = x3 + x4 * x0 - x1;
        x3 = x4 + x0 * x1 - x2;
        x4 = x0 + x1 * x2 - x3;
        
        y0 = y1 ^ y2 | y3 & y4;
        y1 = y2 ^ y3 | y4 & y0;
        y2 = y3 ^ y4 | y0 & y1;
        y3 = y4 ^ y0 | y1 & y2;
        y4 = y0 ^ y1 | y2 & y3;
        
        /* Switch with goto creating irreducible regions */
        switch (i & 7) {
            case 0:
                g0 = g1 * 1.1f + g2 - g3 / g4;
                g1 = g2 * 1.2f + g3 - g4 / g0;
                goto case_a;
            case 1:
                g2 = g3 * 1.3f + g4 - g0 / g1;
                g3 = g4 * 1.4f + g0 - g1 / g2;
                goto case_b;
            case 2:
                g4 = g0 * 1.5f + g1 - g2 / g3;
                h0 = h1 * 1.01 + h2 - h3 / h4;
                goto case_c;
            case 3:
                h1 = h2 * 1.02 + h3 - h4 / h0;
                h2 = h3 * 1.03 + h4 - h0 / h1;
                goto case_d;
            case 4:
                h3 = h4 * 1.04 + h0 - h1 / h2;
                h4 = h0 * 1.05 + h1 - h2 / h3;
                goto case_a;
            case 5:
                m0 = m1 + m2 * m3 - m4;
                m1 = m2 + m3 * m4 - m0;
                goto case_b;
            case 6:
                m2 = m3 + m4 * m0 - m1;
                m3 = m4 + m0 * m1 - m2;
                goto case_c;
            case 7:
                m4 = m0 + m1 * m2 - m3;
                goto case_d;
        }
        
        case_a:
        x0 = x0 ^ (i * 2);
        case_b:
        y0 = y0 + (i * 3);
        case_c:
        g0 = g0 + (i * 4);
        case_d:
        h0 = h0 + (i * 5);
        m0 = m0 ^ (i * 6);
        
        switch_end:
        /* Update checksum */
        checksum += x0 + y0 + (int)g0 + (int)h0 + m0;
        checksum = (checksum << 5) | (checksum >> 59); /* Rotate right */
    }
    
    /* Mark variables as used */
    asm volatile("" : : "r"(x0), "r"(x1), "r"(x2), "r"(x3), "r"(x4));
    asm volatile("" : : "r"(y0), "r"(y1), "r"(y2), "r"(y3), "r"(y4));
    asm volatile("" : : "r"(g0), "r"(g1), "r"(g2), "r"(g3), "r"(g4));
    
    return checksum;
}

/* Complex control flow test 3: Computed goto (labels as values) */
NOINLINE static uint64_t test_computed_goto(int iterations, int seed) {
    VOLATILE_VAR int i;
    VOLATILE_VAR uint64_t checksum = seed;
    
    /* Yet another set of variables */
    VOLATILE_VAR int p0 = 21, p1 = 22, p2 = 23, p3 = 24, p4 = 25;
    VOLATILE_VAR int q0 = 26, q1 = 27, q2 = 28, q3 = 29, q4 = 30;
    VOLATILE_VAR float r0 = 6.1f, r1 = 7.2f, r2 = 8.3f, r3 = 9.4f, r4 = 10.5f;
    VOLATILE_VAR double s0 = 6.01, s1 = 7.02, s2 = 8.03, s3 = 9.04, s4 = 10.05;
    VOLATILE_VAR long t0 = 600, t1 = 700, t2 = 800, t3 = 900, t4 = 1000;
    
    /* Labels for computed goto */
    static void* labels[] = {
        &&state0, &&state1, &&state2, &&state3, 
        &&state4, &&state5, &&state6, &&state7
    };
    
    VOLATILE_VAR int state = 0;
    
    for (i = 0; i < iterations; i++) {
        /* Long arithmetic dependency chains */
        p0 = p1 + p2 * p3 - p4;
        p1 = p2 + p3 * p4 - p0;
        p2 = p3 + p4 * p0 - p1;
        p3 = p4 + p0 * p1 - p2;
        p4 = p0 + p1 * p2 - p3;
        
        q0 = q1 ^ q2 | q3 & q4;
        q1 = q2 ^ q3 | q4 & q0;
        q2 = q3 ^ q4 | q0 & q1;
        q3 = q4 ^ q0 | q1 & q2;
        q4 = q0 ^ q1 | q2 & q3;
        
        r0 = r1 * 1.6f + r2 - r3 / r4;
        r1 = r2 * 1.7f + r3 - r4 / r0;
        r2 = r3 * 1.8f + r4 - r0 / r1;
        r3 = r4 * 1.9f + r0 - r1 / r2;
        r4 = r0 * 2.0f + r1 - r2 / r3;
        
        /* Computed goto - creates very complex CFG */
        goto *labels[state];
        
        state0:
        s0 = s1 * 1.06 + s2 - s3 / s4;
        state = (i & 1) ? 1 : 4;
        continue;
        
        state1:
        s1 = s2 * 1.07 + s3 - s4 / s0;
        state = (i & 2) ? 2 : 5;
        continue;
        
        state2:
        s2 = s3 * 1.08 + s4 - s0 / s1;
        state = (i & 3) ? 3 : 6;
        continue;
        
        state3:
        s3 = s4 * 1.09 + s0 - s1 / s2;
        state = (i & 4) ? 0 : 7;
        continue;
        
        state4:
        s4 = s0 * 1.10 + s1 - s2 / s3;
        t0 = t1 + t2 * t3 - t4;
        state = (i & 5) ? 1 : 0;
        continue;
        
        state5:
        t1 = t2 + t3 * t4 - t0;
        state = (i & 6) ? 2 : 1;
        continue;
        
        state6:
        t2 = t3 + t4 * t0 - t1;
        state = (i & 7) ? 3 : 2;
        continue;
        
        state7:
        t3 = t4 + t0 * t1 - t2;
        t4 = t0 + t1 * t2 - t3;
        state = 0;
        
        /* Update checksum */
        checksum += p0 + q0 + (int)r0 + (int)s0 + t0;
        checksum = checksum * 6364136223846793005ULL + 1442695040888963407ULL;
    }
    
    /* Ensure variables are marked as used */
    asm volatile("" : : "r"(p0), "r"(p1), "r"(p2), "r"(p3), "r"(p4));
    asm volatile("" : : "r"(q0), "r"(q1), "r"(q2), "r"(q3), "r"(q4));
    asm volatile("" : : "r"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r4));
    asm volatile("" : : "r"(s0), "r"(s1), "r"(s2), "r"(s3), "r"(s4));
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int iterations = MAX_ITER;
    uint64_t final_checksum = 0;
    int seed = 42;
    
    /* Parse command line for iteration count */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) {
            iterations = MAX_ITER;
        }
    }
    
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Running MCF coverage tests with %d iterations, seed=%d\n", 
           iterations, seed);
    
    /* Run all test functions to increase coverage chance */
    final_checksum ^= test_irreducible_goto(iterations, seed);
    printf("Test 1 complete, checksum so far: %llu\n", 
           (unsigned long long)final_checksum);
    
    final_checksum ^= test_switch_goto(iterations, seed + 1);
    printf("Test 2 complete, checksum so far: %llu\n", 
           (unsigned long long)final_checksum);
    
    final_checksum ^= test_computed_goto(iterations, seed + 2);
    printf("Test 3 complete, final checksum: %llu\n", 
           (unsigned long long)final_checksum);
    
    return 0;
}

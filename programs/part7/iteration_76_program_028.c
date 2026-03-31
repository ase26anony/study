/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow algorithm
   with complex CFG transformations for coverage testing */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_VARS 30
#define DEFAULT_ITERATIONS 10000

/* Prevent optimization of critical functions */
__attribute__((noinline, noipa))
static unsigned long test_irreducible_goto(int iterations, int seed) {
    /* Declare many local variables to create register pressure */
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
    volatile float f4 = seed * 0.5f;
    
    volatile double d0 = seed * 0.01;
    volatile double d1 = seed * 0.02;
    volatile double d2 = seed * 0.03;
    volatile double d3 = seed * 0.04;
    volatile double d4 = seed * 0.05;
    
    volatile long l0 = seed * 100;
    volatile long l1 = seed * 200;
    volatile long l2 = seed * 300;
    volatile long l3 = seed * 400;
    volatile long l4 = seed * 500;
    
    int temp[NUM_VARS];
    unsigned long checksum = 0;
    
    /* Create irreducible loop structure using goto */
    int i = 0;
    int state = 0;
    
    LOOP_START:
    if (i >= iterations) goto LOOP_END;
    
    /* Complex arithmetic to keep variables live */
    v0 = v1 + v2;
    v1 = v3 * v4;
    v2 = v5 - v6;
    v3 = v7 / (v8 + 1);
    v4 = v9 ^ v0;
    
    f0 = f1 * f2;
    f1 = f3 + f4;
    f2 = f0 - f1;
    f3 = f2 * 1.1f;
    f4 = f3 / 2.0f;
    
    d0 = d1 + d2;
    d1 = d3 * d4;
    d2 = d0 - d1;
    d3 = d2 * 1.01;
    d4 = d3 / 2.0;
    
    l0 = l1 + l2;
    l1 = l3 * l4;
    l2 = l0 - l1;
    l3 = l2 * 3;
    l4 = l3 / 2;
    
    /* Force register usage with inline asm */
    asm volatile("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4));
    asm volatile("" : : "r"(f0), "r"(f1), "r"(f2), "r"(f3), "r"(f4));
    asm volatile("" : : "r"(d0), "r"(d1), "r"(d2), "r"(d3), "r"(d4));
    asm volatile("" : : "r"(l0), "r"(l1), "r"(l2), "r"(l3), "r"(l4));
    
    /* Irreducible control flow - jump between loops */
    switch (state) {
        case 0:
            if (i % 7 == 0) goto INNER_LOOP_A;
            state = 1;
            break;
        case 1:
            if (i % 11 == 0) goto INNER_LOOP_B;
            state = 2;
            break;
        case 2:
            if (i % 13 == 0) goto INNER_LOOP_C;
            state = 0;
            break;
    }
    
    i++;
    goto LOOP_START;
    
    INNER_LOOP_A:
    v5 = v6 + v7;
    v6 = v8 * v9;
    if (i % 3 == 0) goto INNER_LOOP_B;
    i++;
    goto LOOP_START;
    
    INNER_LOOP_B:
    v7 = v0 - v1;
    v8 = v2 ^ v3;
    if (i % 5 == 0) goto INNER_LOOP_C;
    i++;
    goto LOOP_START;
    
    INNER_LOOP_C:
    v9 = v4 + v5;
    v0 = v6 * v7;
    if (i % 2 == 0) goto LOOP_START;
    i++;
    goto INNER_LOOP_A;
    
    LOOP_END:
    
    /* Compute checksum to prevent dead code elimination */
    checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
               (int)f0 + (int)f1 + (int)f2 + (int)f3 + (int)f4 +
               (int)d0 + (int)d1 + (int)d2 + (int)d3 + (int)d4 +
               l0 + l1 + l2 + l3 + l4;
    
    return checksum;
}

__attribute__((noinline, noipa))
static unsigned long test_switch_goto(int iterations, int seed) {
    volatile int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    volatile int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    volatile int i = seed + 8, j = seed + 9, k = seed + 10, l = seed + 11;
    volatile int m = seed + 12, n = seed + 13, o = seed + 14, p = seed + 15;
    
    volatile float fa = seed * 1.1f, fb = seed * 1.2f, fc = seed * 1.3f;
    volatile double da = seed * 2.1, db = seed * 2.2, dc = seed * 2.3;
    
    unsigned long checksum = 0;
    int counter = 0;
    int switch_var = seed % 5;
    
    START_SWITCH_LOOP:
    if (counter >= iterations) goto END_SWITCH_LOOP;
    
    /* Complex arithmetic chains */
    a = b + c;
    b = d * e;
    c = f - g;
    d = h / (i + 1);
    e = j ^ k;
    f = l + m;
    g = n * o;
    h = p - a;
    i = b ^ c;
    j = d + e;
    k = f * g;
    l = h - i;
    m = j ^ k;
    n = l + m;
    o = n * p;
    p = o - a;
    
    fa = fb * fc;
    fb = fa + 1.0f;
    fc = fb - 0.5f;
    
    da = db + dc;
    db = da * 1.5;
    dc = db - 0.25;
    
    /* Force register usage */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), "r"(g));
    asm volatile("" : : "r"(h), "r"(i), "r"(j), "r"(k), "r"(l), "r"(m), "r"(n));
    asm volatile("" : : "r"(o), "r"(p), "r"(fa), "r"(fb), "r"(fc));
    asm volatile("" : : "r"(da), "r"(db), "r"(dc));
    
    /* Switch with goto creating irreducible regions */
    switch (switch_var) {
        case 0:
            a = b + 1;
            if (counter % 7 == 0) goto LABEL_X;
            switch_var = 1;
            break;
        case 1:
            c = d * 2;
            if (counter % 11 == 0) goto LABEL_Y;
            switch_var = 2;
            break;
        case 2:
            e = f - 3;
            if (counter % 13 == 0) goto LABEL_Z;
            switch_var = 3;
            break;
        case 3:
            g = h ^ 4;
            if (counter % 17 == 0) goto LABEL_X;
            switch_var = 4;
            break;
        case 4:
            i = j + 5;
            if (counter % 19 == 0) goto LABEL_Y;
            switch_var = 0;
            break;
    }
    
    counter++;
    goto START_SWITCH_LOOP;
    
    LABEL_X:
    k = l * m;
    if (counter % 4 == 0) goto LABEL_Y;
    counter++;
    goto START_SWITCH_LOOP;
    
    LABEL_Y:
    n = o ^ p;
    if (counter % 6 == 0) goto LABEL_Z;
    counter++;
    goto LABEL_X;
    
    LABEL_Z:
    a = b + c;
    if (counter % 8 == 0) goto START_SWITCH_LOOP;
    counter++;
    goto LABEL_Y;
    
    END_SWITCH_LOOP:
    
    checksum = a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p +
               (int)fa + (int)fb + (int)fc + (int)da + (int)db + (int)dc;
    
    return checksum;
}

__attribute__((noinline, noipa))
static unsigned long test_computed_goto(int iterations, int seed) {
    /* State machine using computed goto */
    static void* labels[] = {
        &&STATE_A, &&STATE_B, &&STATE_C, 
        &&STATE_D, &&STATE_E, &&STATE_F
    };
    
    volatile int s0 = seed, s1 = seed + 1, s2 = seed + 2, s3 = seed + 3;
    volatile int s4 = seed + 4, s5 = seed + 5, s6 = seed + 6, s7 = seed + 7;
    volatile int s8 = seed + 8, s9 = seed + 9, s10 = seed + 10, s11 = seed + 11;
    volatile int s12 = seed + 12, s13 = seed + 13, s14 = seed + 14, s15 = seed + 15;
    
    volatile long ls0 = seed * 1000, ls1 = seed * 2000, ls2 = seed * 3000;
    volatile double ds0 = seed * 0.001, ds1 = seed * 0.002, ds2 = seed * 0.003;
    
    unsigned long checksum = 0;
    int count = 0;
    int state = seed % 6;
    
    /* Initial jump */
    goto *labels[state];
    
    STATE_A:
    if (count >= iterations) goto FINAL_STATE;
    
    /* Complex computations */
    s0 = s1 + s2;
    s1 = s3 * s4;
    s2 = s5 - s6;
    s3 = s7 ^ s8;
    s4 = s9 + s10;
    s5 = s11 * s12;
    s6 = s13 - s14;
    s7 = s15 ^ s0;
    
    ls0 = ls1 + ls2;
    ls1 = ls0 * 2;
    ls2 = ls1 - 1000;
    
    ds0 = ds1 + ds2;
    ds1 = ds0 * 1.1;
    ds2 = ds1 - 0.1;
    
    asm volatile("" : : "r"(s0), "r"(s1), "r"(s2), "r"(s3), "r"(s4), "r"(s5));
    asm volatile("" : : "r"(s6), "r"(s7), "r"(s8), "r"(s9), "r"(s10), "r"(s11));
    asm volatile("" : : "r"(s12), "r"(s13), "r"(s14), "r"(s15));
    asm volatile("" : : "r"(ls0), "r"(ls1), "r"(ls2), "r"(ds0), "r"(ds1), "r"(ds2));
    
    state = (state + 1) % 6;
    count++;
    goto *labels[state];
    
    STATE_B:
    s8 = s9 + s10;
    s9 = s11 * s12;
    s10 = s13 - s14;
    s11 = s15 ^ s0;
    s12 = s1 + s2;
    
    state = (state + (count % 3)) % 6;
    count++;
    goto *labels[state];
    
    STATE_C:
    s13 = s14 + s15;
    s14 = s0 * s1;
    s15 = s2 - s3;
    
    state = (state + (count % 5)) % 6;
    count++;
    goto *labels[state];
    
    STATE_D:
    ls0 = ls1 * ls2;
    ls1 = ls0 + 500;
    ls2 = ls1 - 250;
    
    state = 4;  /* Force specific transition */
    count++;
    goto *labels[state];
    
    STATE_E:
    ds0 = ds1 / ds2;
    ds1 = ds0 * 2.0;
    ds2 = ds1 + 0.5;
    
    state = (count % 2 == 0) ? 5 : 0;
    count++;
    goto *labels[state];
    
    STATE_F:
    /* Mix all variables */
    s0 = s1 + s2 + s3 + s4;
    s1 = s5 * s6 * s7;
    s2 = s8 - s9 - s10;
    
    state = (count % 7 == 0) ? 0 : 1;
    count++;
    if (count < iterations) {
        goto *labels[state];
    }
    
    FINAL_STATE:
    checksum = s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8 + s9 + s10 +
               s11 + s12 + s13 + s14 + s15 + ls0 + ls1 + ls2 +
               (long)ds0 + (long)ds1 + (long)ds2;
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int iterations = DEFAULT_ITERATIONS;
    unsigned long total_checksum = 0;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) {
            iterations = DEFAULT_ITERATIONS;
        }
    }
    
    printf("Running MCF coverage test with %d iterations...\n", iterations);
    
    /* Run all test patterns to maximize coverage chance */
    total_checksum += test_irreducible_goto(iterations, 42);
    total_checksum += test_switch_goto(iterations, 123);
    total_checksum += test_computed_goto(iterations, 789);
    
    printf("Final checksum: %lu\n", total_checksum);
    
    return 0;
}

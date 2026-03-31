/* Complex control flow program to trigger MCF fixup graph debug output */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
__attribute__((noinline, optimize("no-goto")))
unsigned long test_irreducible_goto(int iterations, int seed) {
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
    volatile long l0 = seed * 2;
    volatile long l1 = seed * 3;
    volatile long l2 = seed * 4;
    volatile long l3 = seed * 5;
    volatile float f0 = seed * 0.1f;
    volatile float f1 = seed * 0.2f;
    volatile float f2 = seed * 0.3f;
    volatile float f3 = seed * 0.4f;
    volatile double d0 = seed * 0.01;
    volatile double d1 = seed * 0.02;
    volatile double d2 = seed * 0.03;
    volatile double d3 = seed * 0.04;
    volatile double d4 = seed * 0.05;
    volatile double d5 = seed * 0.06;
    volatile double d6 = seed * 0.07;
    volatile double d7 = seed * 0.08;
    volatile double d8 = seed * 0.09;
    volatile double d9 = seed * 0.10;
    
    unsigned long checksum = 0;
    
    /* Create irreducible loop with goto */
    int i = 0;
    int state = 0;
    
    /* Label definitions for goto jumps */
    outer_loop:
    if (i >= iterations) goto end_func;
    
    inner_loop_start:
    /* Complex arithmetic to keep variables live */
    v0 = v1 + v2;
    v1 = v3 ^ v4;
    v2 = v5 | v6;
    v3 = v7 & v8;
    v4 = v9 * v0;
    l0 = l1 + l2;
    l1 = l3 * l0;
    l2 = l0 - l1;
    l3 = l2 / (l1 + 1);
    f0 = f1 * f2;
    f1 = f3 + f0;
    f2 = f0 - f1;
    f3 = f2 / (f1 + 0.1f);
    d0 = d1 + d2;
    d1 = d3 * d4;
    d2 = d5 - d6;
    d3 = d7 / (d8 + 0.1);
    d4 = d9 + d0;
    d5 = d1 * d2;
    d6 = d3 - d4;
    d7 = d5 / (d6 + 0.1);
    d8 = d7 + d9;
    d9 = d8 * d0;
    
    /* Mark variables as used to prevent optimization */
    asm volatile("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4));
    asm volatile("" : : "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9));
    asm volatile("" : : "r"(l0), "r"(l1), "r"(l2), "r"(l3));
    asm volatile("" : : "r"(f0), "r"(f1), "r"(f2), "r"(f3));
    asm volatile("" : : "r"(d0), "r"(d1), "r"(d2), "r"(d3), "r"(d4));
    asm volatile("" : : "r"(d5), "r"(d6), "r"(d7), "r"(d8), "r"(d9));
    
    /* Irreducible control flow with goto */
    switch (state) {
        case 0:
            if (i % 3 == 0) goto label_a;
            else goto label_b;
        case 1:
            if (i % 5 == 0) goto label_c;
            else goto inner_loop_start;
        case 2:
            goto outer_loop;
    }
    
    label_a:
    v0 = v0 * 2 + 1;
    state = 1;
    if (i % 7 == 0) goto label_c;
    else goto inner_loop_start;
    
    label_b:
    v1 = v1 * 3 - 1;
    state = 2;
    if (i % 11 == 0) goto outer_loop;
    else goto label_a;
    
    label_c:
    v2 = v2 / 2 + 5;
    state = 0;
    i++;
    if (i % 13 == 0) goto label_b;
    else goto outer_loop;
    
    end_func:
    /* Compute checksum */
    checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    checksum += l0 + l1 + l2 + l3;
    checksum += (unsigned long)f0 + (unsigned long)f1 + (unsigned long)f2 + (unsigned long)f3;
    checksum += (unsigned long)d0 + (unsigned long)d1 + (unsigned long)d2 + (unsigned long)d3;
    
    return checksum;
}

__attribute__((noinline))
unsigned long test_switch_goto(int iterations, int seed) {
    /* Another set of many variables */
    volatile int a0 = seed, a1 = seed+1, a2 = seed+2, a3 = seed+3, a4 = seed+4;
    volatile int a5 = seed+5, a6 = seed+6, a7 = seed+7, a8 = seed+8, a9 = seed+9;
    volatile int b0 = seed*2, b1 = seed*3, b2 = seed*4, b3 = seed*5, b4 = seed*6;
    volatile float c0 = seed*0.5f, c1 = seed*0.6f, c2 = seed*0.7f, c3 = seed*0.8f;
    volatile double d0 = seed*0.15, d1 = seed*0.25, d2 = seed*0.35, d3 = seed*0.45;
    volatile long e0 = seed*10, e1 = seed*11, e2 = seed*12, e3 = seed*13;
    
    unsigned long checksum = 0;
    int i = 0;
    int mode = 0;
    
    loop_start:
    if (i >= iterations) goto finish;
    
    /* Complex arithmetic operations */
    a0 = a1 + a2 * a3 - a4;
    a1 = a5 ^ a6 | a7 & a8;
    a2 = a9 * a0 / (a1 + 1);
    a3 = b0 + b1 - b2 * b3;
    a4 = b4 ^ a0 | a1 & a2;
    b0 = a3 + a4 - a5 * a6;
    b1 = a7 ^ a8 | a9 & b0;
    b2 = b1 * b3 / (b4 + 1);
    c0 = c1 * c2 + c3;
    c1 = c0 - c2 * c3;
    d0 = d1 + d2 * d3;
    d1 = d0 - d2 / (d3 + 0.1);
    e0 = e1 + e2 * e3;
    e1 = e0 - e2 / (e3 + 1);
    
    /* Force register usage */
    asm volatile("" : : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4));
    asm volatile("" : : "r"(a5), "r"(a6), "r"(a7), "r"(a8), "r"(a9));
    asm volatile("" : : "r"(b0), "r"(b1), "r"(b2), "r"(b3), "r"(b4));
    asm volatile("" : : "r"(c0), "r"(c1), "r"(c2), "r"(c3));
    asm volatile("" : : "r"(d0), "r"(d1), "r"(d2), "r"(d3));
    asm volatile("" : : "r"(e0), "r"(e1), "r"(e2), "r"(e3));
    
    /* Switch with goto creating irreducible flow */
    switch (mode) {
        case 0:
            if (i % 2 == 0) goto case0_block1;
            else goto case0_block2;
        case 1:
            if (i % 3 == 0) goto case1_block1;
            else goto case1_block2;
        case 2:
            if (i % 5 == 0) goto case2_block1;
            else goto loop_start;
    }
    
    case0_block1:
    a0 = a0 * 3 + 7;
    mode = 1;
    if (i % 4 == 0) goto case1_block1;
    else goto case0_block2;
    
    case0_block2:
    a1 = a1 / 2 - 3;
    mode = 2;
    if (i % 6 == 0) goto case2_block1;
    else goto loop_start;
    
    case1_block1:
    b0 = b0 ^ 0xAAAA;
    mode = 0;
    if (i % 8 == 0) goto case0_block1;
    else goto case1_block2;
    
    case1_block2:
    b1 = b1 | 0x5555;
    mode = 2;
    if (i % 10 == 0) goto case2_block1;
    else goto loop_start;
    
    case2_block1:
    c0 = c0 * 1.5f;
    mode = 1;
    i++;
    if (i % 12 == 0) goto case1_block1;
    else goto loop_start;
    
    finish:
    checksum = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9;
    checksum += b0 + b1 + b2 + b3 + b4;
    checksum += (unsigned long)c0 + (unsigned long)c1 + (unsigned long)c2 + (unsigned long)c3;
    checksum += (unsigned long)d0 + (unsigned long)d1 + (unsigned long)d2 + (unsigned long)d3;
    checksum += e0 + e1 + e2 + e3;
    
    return checksum;
}

__attribute__((noinline))
unsigned long test_computed_goto(int iterations, int seed) {
    /* State machine using computed goto */
    static void* labels[] = {
        &&state0, &&state1, &&state2, &&state3, 
        &&state4, &&state5, &&state6, &&state7
    };
    
    /* Many variables for register pressure */
    volatile int r0 = seed, r1 = seed+1, r2 = seed+2, r3 = seed+3, r4 = seed+4;
    volatile int r5 = seed+5, r6 = seed+6, r7 = seed+7, r8 = seed+8, r9 = seed+9;
    volatile int s0 = seed*2, s1 = seed*3, s2 = seed*4, s3 = seed*5, s4 = seed*6;
    volatile float t0 = seed*0.3f, t1 = seed*0.4f, t2 = seed*0.5f, t3 = seed*0.6f;
    volatile double u0 = seed*0.07, u1 = seed*0.08, u2 = seed*0.09, u3 = seed*0.10;
    volatile long v0 = seed*20, v1 = seed*21, v2 = seed*22, v3 = seed*23;
    
    unsigned long checksum = 0;
    int state = 0;
    int counter = 0;
    
    /* Initial goto */
    goto *labels[state];
    
    state0:
    r0 = r1 + r2 * r3 - r4;
    r1 = r5 ^ r6 | r7 & r8;
    state = (counter % 3 == 0) ? 1 : 4;
    goto next;
    
    state1:
    r2 = r9 * r0 / (r1 + 1);
    r3 = s0 + s1 - s2 * s3;
    state = (counter % 5 == 0) ? 2 : 5;
    goto next;
    
    state2:
    r4 = s4 ^ r0 | r1 & r2;
    s0 = r3 + r4 - r5 * r6;
    state = (counter % 7 == 0) ? 3 : 6;
    goto next;
    
    state3:
    s1 = r7 ^ r8 | r9 & s0;
    s2 = s1 * s3 / (s4 + 1);
    state = (counter % 11 == 0) ? 4 : 7;
    goto next;
    
    state4:
    t0 = t1 * t2 + t3;
    t1 = t0 - t2 * t3;
    state = (counter % 13 == 0) ? 5 : 0;
    goto next;
    
    state5:
    u0 = u1 + u2 * u3;
    u1 = u0 - u2 / (u3 + 0.1);
    state = (counter % 17 == 0) ? 6 : 1;
    goto next;
    
    state6:
    v0 = v1 + v2 * v3;
    v1 = v0 - v2 / (v3 + 1);
    state = (counter % 19 == 0) ? 7 : 2;
    goto next;
    
    state7:
    /* Mix all variables */
    r0 = r0 ^ s0 ^ (int)t0 ^ (int)u0 ^ (int)v0;
    r1 = r1 ^ s1 ^ (int)t1 ^ (int)u1 ^ (int)v1;
    state = (counter % 23 == 0) ? 0 : 3;
    goto next;
    
    next:
    /* Force all variables to be live */
    asm volatile("" : : 
        "r"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r4),
        "r"(r5), "r"(r6), "r"(r7), "r"(r8), "r"(r9),
        "r"(s0), "r"(s1), "r"(s2), "r"(s3), "r"(s4),
        "r"(t0), "r"(t1), "r"(t2), "r"(t3),
        "r"(u0), "r"(u1), "r"(u2), "r"(u3),
        "r"(v0), "r"(v1), "r"(v2), "r"(v3)
    );
    
    counter++;
    if (counter >= iterations) goto done;
    goto *labels[state];
    
    done:
    checksum = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9;
    checksum += s0 + s1 + s2 + s3 + s4;
    checksum += (unsigned long)t0 + (unsigned long)t1 + (unsigned long)t2 + (unsigned long)t3;
    checksum += (unsigned long)u0 + (unsigned long)u1 + (unsigned long)u2 + (unsigned long)u3;
    checksum += v0 + v1 + v2 + v3;
    
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
    
    printf("Running MCF stress test with %d iterations, seed=%d\n", iterations, seed);
    
    unsigned long total_checksum = 0;
    
    /* Call all test functions to create complex CFGs */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    printf("Total checksum: %lu\n", total_checksum);
    
    return 0;
}

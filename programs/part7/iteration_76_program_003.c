/* mcf_coverage.c - Program to trigger GCC's MCF algorithm fixup graph printing */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define KEEP_ALIVE(var) asm volatile("" : : "r"(var))

/* Function 1: Irreducible loops with goto jumping across loop boundaries */
NOINLINE static uint64_t test_irreducible_goto(int iterations, int seed) {
    /* Declare many variables to increase register pressure */
    volatile int v0 = seed + 1;    int v1 = v0 * 2;
    volatile int v2 = seed + 2;    int v3 = v2 * 3;
    volatile int v4 = seed + 3;    int v5 = v4 * 4;
    volatile int v6 = seed + 4;    int v7 = v6 * 5;
    volatile int v8 = seed + 5;    int v9 = v8 * 6;
    volatile int v10 = seed + 6;   int v11 = v10 * 7;
    volatile int v12 = seed + 7;   int v13 = v12 * 8;
    volatile int v14 = seed + 8;   int v15 = v14 * 9;
    volatile int v16 = seed + 9;   int v17 = v16 * 10;
    volatile int v18 = seed + 10;  int v19 = v18 * 11;
    volatile int v20 = seed + 11;  int v21 = v20 * 12;
    volatile int v22 = seed + 12;  int v23 = v22 * 13;
    volatile int v24 = seed + 13;  int v25 = v24 * 14;
    volatile int v26 = seed + 14;  int v27 = v26 * 15;
    volatile int v28 = seed + 15;  int v29 = v28 * 16;
    
    uint64_t checksum = 0;
    int i = 0;
    
    /* Outer loop */
    outer_loop_start:
    if (i >= iterations) goto outer_loop_end;
    
    /* Complex arithmetic to keep variables live */
    v1 = v0 + v2;   v3 = v1 * v4;   v5 = v3 - v6;
    v7 = v5 * v8;   v9 = v7 + v10;  v11 = v9 / (v12 ? v12 : 1);
    v13 = v11 ^ v14; v15 = v13 | v16; v17 = v15 & v18;
    v19 = v17 << 2; v21 = v19 >> 1; v23 = v21 * v22;
    v25 = v23 % 997; v27 = v25 + v26; v29 = v27 * v28;
    
    /* Jump to inner loop based on condition */
    if ((i & 3) == 0) goto inner_loop_a;
    if ((i & 3) == 1) goto inner_loop_b;
    
    inner_loop_a:
    for (int j = 0; j < 5; j++) {
        /* More arithmetic mixing */
        v0 = v29 + j; v2 = v1 * v0; v4 = v3 ^ v2;
        v6 = v5 | v4; v8 = v7 & v6; v10 = v9 + v8;
        if ((j & 1) == 0 && i > 10) goto outer_loop_start; /* Jump back to outer */
    }
    goto after_inner;
    
    inner_loop_b:
    for (int j = 0; j < 3; j++) {
        v12 = v11 - j; v14 = v13 * v12; v16 = v15 ^ v14;
        v18 = v17 | v16; v20 = v19 & v18; v22 = v21 + v20;
        if (j == 2 && (i % 7) == 0) goto inner_loop_a; /* Jump to other inner loop */
    }
    
    after_inner:
    /* Mix all variables into checksum */
    checksum += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    checksum += v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
    checksum += v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
    
    i++;
    goto outer_loop_start;
    
    outer_loop_end:
    
    /* Force all variables to be considered live */
    KEEP_ALIVE(v0); KEEP_ALIVE(v1); KEEP_ALIVE(v2); KEEP_ALIVE(v3);
    KEEP_ALIVE(v4); KEEP_ALIVE(v5); KEEP_ALIVE(v6); KEEP_ALIVE(v7);
    KEEP_ALIVE(v8); KEEP_ALIVE(v9); KEEP_ALIVE(v10); KEEP_ALIVE(v11);
    KEEP_ALIVE(v12); KEEP_ALIVE(v13); KEEP_ALIVE(v14); KEEP_ALIVE(v15);
    KEEP_ALIVE(v16); KEEP_ALIVE(v17); KEEP_ALIVE(v18); KEEP_ALIVE(v19);
    KEEP_ALIVE(v20); KEEP_ALIVE(v21); KEEP_ALIVE(v22); KEEP_ALIVE(v23);
    KEEP_ALIVE(v24); KEEP_ALIVE(v25); KEEP_ALIVE(v26); KEEP_ALIVE(v27);
    KEEP_ALIVE(v28); KEEP_ALIVE(v29);
    
    return checksum;
}

/* Function 2: Switch with goto creating irreducible regions */
NOINLINE static uint64_t test_switch_goto(int iterations, int seed) {
    /* Another set of many variables */
    volatile double d0 = seed * 1.1;   double d1 = d0 * 2.2;
    volatile double d2 = seed * 1.2;   double d3 = d2 * 3.3;
    volatile double d4 = seed * 1.3;   double d5 = d4 * 4.4;
    volatile float f0 = seed * 0.5f;   float f1 = f0 * 1.5f;
    volatile float f2 = seed * 0.6f;   float f3 = f2 * 2.5f;
    volatile long l0 = seed * 100L;    long l1 = l0 * 200L;
    volatile long l2 = seed * 150L;    long l3 = l2 * 250L;
    volatile int i0 = seed;            int i1 = i0 * 3;
    volatile int i2 = seed + 100;      int i3 = i2 * 4;
    volatile int i4 = seed + 200;      int i5 = i4 * 5;
    
    uint64_t checksum = 0;
    int state = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex floating point and integer mixing */
        d1 = d0 * d2 + d3; d3 = d1 - d4 * d5;
        f1 = f0 + f2 * f3; f3 = f1 / (f2 ? f2 : 1.0f);
        l1 = l0 ^ l2 | l3; l3 = l1 & (l2 + i);
        i1 = i0 * i2 + i3; i3 = i1 - i4 * i5;
        i5 = (i4 + i) % 256;
        
        /* Switch with goto to different labels */
        switch (state) {
            case 0:
                d0 = d1 * 1.01;
                if ((i & 1) == 0) goto label_a;
                state = 1;
                break;
            case 1:
                f0 = f1 * 1.02f;
                if ((i & 3) == 0) goto label_b;
                state = 2;
                break;
            case 2:
                l0 = l1 + i;
                if ((i & 7) == 0) goto label_c;
                state = 0;
                break;
            default:
                state = 0;
        }
        
        /* Continue normal flow */
        checksum += (uint64_t)(d0 + d1 + d2 + d3 + d4 + d5);
        checksum += (uint64_t)(f0 + f1 + f2 + f3);
        checksum += l0 + l1 + l2 + l3 + i0 + i1 + i2 + i3 + i4 + i5;
        continue;
        
        label_a:
        d2 = d3 * 0.99;
        state = 2;
        goto after_labels;
        
        label_b:
        f2 = f3 * 0.98f;
        state = 0;
        goto after_labels;
        
        label_c:
        l2 = l3 - i;
        state = 1;
        /* fall through */
        
        after_labels:
        /* More operations after label jumps */
        i0 = i1 ^ i2;
        i2 = i3 | i4;
        i4 = i5 & i;
    }
    
    /* Keep variables alive */
    KEEP_ALIVE(d0); KEEP_ALIVE(d1); KEEP_ALIVE(d2); KEEP_ALIVE(d3);
    KEEP_ALIVE(d4); KEEP_ALIVE(d5); KEEP_ALIVE(f0); KEEP_ALIVE(f1);
    KEEP_ALIVE(f2); KEEP_ALIVE(f3); KEEP_ALIVE(l0); KEEP_ALIVE(l1);
    KEEP_ALIVE(l2); KEEP_ALIVE(l3); KEEP_ALIVE(i0); KEEP_ALIVE(i1);
    KEEP_ALIVE(i2); KEEP_ALIVE(i3); KEEP_ALIVE(i4); KEEP_ALIVE(i5);
    
    return checksum;
}

/* Function 3: Computed goto simulating state machine */
NOINLINE static uint64_t test_computed_goto(int iterations, int seed) {
    /* Yet another set of variables */
    volatile int a0 = seed;           int a1 = a0 * 2;
    volatile int a2 = seed + 1;       int a3 = a2 * 3;
    volatile int a4 = seed + 2;       int a5 = a4 * 4;
    volatile int a6 = seed + 3;       int a7 = a6 * 5;
    volatile int a8 = seed + 4;       int a9 = a8 * 6;
    volatile int a10 = seed + 5;      int a11 = a10 * 7;
    volatile int a12 = seed + 6;      int a13 = a12 * 8;
    volatile int a14 = seed + 7;      int a15 = a14 * 9;
    volatile int a16 = seed + 8;      int a17 = a16 * 10;
    volatile int a18 = seed + 9;      int a19 = a18 * 11;
    
    /* Labels for computed goto */
    static void* labels[] = { &&state0, &&state1, &&state2, &&state3, &&state4 };
    
    uint64_t checksum = 0;
    int state = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Long dependency chain */
        a1 = a0 + i;      a3 = a2 * a1;    a5 = a4 - a3;
        a7 = a6 ^ a5;     a9 = a8 | a7;    a11 = a10 & a9;
        a13 = a12 << (i & 3); a15 = a14 >> 1; a17 = a16 * a15;
        a19 = a18 % 997;  a0 = a19 + a1;   a2 = a0 * a3;
        a4 = a2 + a5;     a6 = a4 ^ a7;    a8 = a6 | a9;
        a10 = a8 & a11;   a12 = a10 + a13; a14 = a12 * a15;
        a16 = a14 - a17;  a18 = a16 ^ a19;
        
        /* Update checksum */
        checksum += a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9;
        checksum += a10 + a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19;
        
        /* Computed goto based on complex condition */
        int next_state = (a0 + a5 + a10 + a15 + i) % 5;
        goto *labels[next_state];
        
        state0:
            a0 = a1 * 2;
            a2 = a3 + 1;
            state = 1;
            continue;
            
        state1:
            a4 = a5 / (a6 ? a6 : 1);
            a8 = a7 ^ 0xFF;
            state = 2;
            continue;
            
        state2:
            a10 = a9 | a11;
            a12 = a13 << 2;
            state = 3;
            continue;
            
        state3:
            a14 = a15 & a16;
            a18 = a17 % 13;
            state = 4;
            continue;
            
        state4:
            a1 = a0 + a19;
            a3 = a2 * 3;
            state = 0;
            continue;
    }
    
    /* Keep all variables alive */
    KEEP_ALIVE(a0); KEEP_ALIVE(a1); KEEP_ALIVE(a2); KEEP_ALIVE(a3);
    KEEP_ALIVE(a4); KEEP_ALIVE(a5); KEEP_ALIVE(a6); KEEP_ALIVE(a7);
    KEEP_ALIVE(a8); KEEP_ALIVE(a9); KEEP_ALIVE(a10); KEEP_ALIVE(a11);
    KEEP_ALIVE(a12); KEEP_ALIVE(a13); KEEP_ALIVE(a14); KEEP_ALIVE(a15);
    KEEP_ALIVE(a16); KEEP_ALIVE(a17); KEEP_ALIVE(a18); KEEP_ALIVE(a19);
    
    return checksum;
}

/* Main function that runs all tests */
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
    
    /* Run all test functions to trigger different CFG patterns */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    printf("Final checksum: %lu\n", total_checksum);
    
    return 0;
}

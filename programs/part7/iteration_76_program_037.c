/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow algorithm fixup graph debugging output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define KEEP_ALIVE(var) asm volatile("" : : "r"(var))

/* Function 1: Irreducible loop with goto jumping across loop boundaries */
NOINLINE static uint64_t test_irreducible_goto(int iterations, int seed) {
    /* Declare many variables to increase register pressure */
    volatile int v0 = seed + 1;
    int v1 = seed + 2, v2 = seed + 3, v3 = seed + 4, v4 = seed + 5;
    int v5 = seed + 6, v6 = seed + 7, v7 = seed + 8, v8 = seed + 9, v9 = seed + 10;
    float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f, f3 = seed * 0.4f;
    double d0 = seed * 0.01, d1 = seed * 0.02, d2 = seed * 0.03, d3 = seed * 0.04;
    long l0 = seed * 100, l1 = seed * 200, l2 = seed * 300, l3 = seed * 400;
    int v10 = seed + 11, v11 = seed + 12, v12 = seed + 13, v13 = seed + 14;
    int v14 = seed + 15, v15 = seed + 16, v16 = seed + 17, v17 = seed + 18;
    int v18 = seed + 19, v19 = seed + 20;
    
    uint64_t checksum = 0;
    int i = 0;
    
    /* Label definitions for irreducible control flow */
    outer_loop_start:
    if (i >= iterations) goto loop_end;
    
    inner_loop_start:
    /* Complex arithmetic to keep variables live */
    v1 = v0 * v2 + v3;
    v2 = v1 ^ v4 | v5;
    v3 = v2 + v6 - v7;
    v4 = v3 * v8 / (v9 + 1);
    v5 = v4 & v10 | v11;
    v6 = v5 + v12 - v13;
    v7 = v6 * v14 + v15;
    v8 = v7 ^ v16;
    v9 = v8 + v17 - v18;
    v10 = v9 * v19;
    
    /* Floating point operations */
    f0 = f1 * f2 + f3;
    f1 = f0 - f2 * f3;
    f2 = f1 + f3 / (f0 + 0.1f);
    f3 = f2 * 1.1f - f0;
    
    /* Double precision operations */
    d0 = d1 + d2 * d3;
    d1 = d0 - d2 / (d3 + 0.001);
    d2 = d1 * 1.01 + d3;
    d3 = d2 - d0 * 0.99;
    
    /* Long operations */
    l0 = l1 + l2 * l3;
    l1 = l0 ^ l2 | l3;
    l2 = l1 + l3 - l0;
    l3 = l2 * 2 + l1;
    
    /* Irreducible control flow: goto jumps between loops */
    if ((i & 3) == 0) {
        /* Jump from inner loop back to outer loop start */
        i++;
        goto outer_loop_start;
    }
    if ((i & 7) == 0) {
        /* Jump to a label inside the switch in another function */
        /* This creates cross-function complexity in the CFG */
        i++;
        goto inner_loop_end;
    }
    
    /* More arithmetic to ensure variables stay live */
    v11 = v10 + v0 - v1;
    v12 = v11 * v2 + v3;
    v13 = v12 ^ v4;
    v14 = v13 + v5 - v6;
    v15 = v14 * v7;
    v16 = v15 ^ v8 | v9;
    v17 = v16 + v10 - v11;
    v18 = v17 * v12;
    v19 = v18 ^ v13;
    
    inner_loop_end:
    i++;
    if ((i & 1) == 0) {
        goto inner_loop_start;
    } else {
        goto outer_loop_start;
    }
    
    loop_end:
    
    /* Aggregate checksum from all variables */
    checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
               v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
               (uint64_t)f0 + (uint64_t)f1 + (uint64_t)f2 + (uint64_t)f3 +
               (uint64_t)d0 + (uint64_t)d1 + (uint64_t)d2 + (uint64_t)d3 +
               l0 + l1 + l2 + l3;
    
    /* Mark variables as used to prevent optimization */
    KEEP_ALIVE(v0); KEEP_ALIVE(v1); KEEP_ALIVE(v2); KEEP_ALIVE(v3);
    KEEP_ALIVE(v4); KEEP_ALIVE(v5); KEEP_ALIVE(v6); KEEP_ALIVE(v7);
    KEEP_ALIVE(v8); KEEP_ALIVE(v9); KEEP_ALIVE(v10); KEEP_ALIVE(v11);
    KEEP_ALIVE(v12); KEEP_ALIVE(v13); KEEP_ALIVE(v14); KEEP_ALIVE(v15);
    KEEP_ALIVE(v16); KEEP_ALIVE(v17); KEEP_ALIVE(v18); KEEP_ALIVE(v19);
    KEEP_ALIVE(f0); KEEP_ALIVE(f1); KEEP_ALIVE(f2); KEEP_ALIVE(f3);
    KEEP_ALIVE(d0); KEEP_ALIVE(d1); KEEP_ALIVE(d2); KEEP_ALIVE(d3);
    KEEP_ALIVE(l0); KEEP_ALIVE(l1); KEEP_ALIVE(l2); KEEP_ALIVE(l3);
    
    return checksum;
}

/* Function 2: Switch with goto creating irreducible regions */
NOINLINE static uint64_t test_switch_goto(int iterations, int seed) {
    volatile int s0 = seed;
    int s1 = seed + 100, s2 = seed + 200, s3 = seed + 300, s4 = seed + 400;
    int s5 = seed + 500, s6 = seed + 600, s7 = seed + 700, s8 = seed + 800;
    int s9 = seed + 900, s10 = seed + 1000, s11 = seed + 1100;
    float sf0 = seed * 1.5f, sf1 = seed * 2.5f, sf2 = seed * 3.5f;
    double sd0 = seed * 0.15, sd1 = seed * 0.25, sd2 = seed * 0.35;
    long sl0 = seed * 150, sl1 = seed * 250, sl2 = seed * 350;
    
    uint64_t checksum = 0;
    int state = 0;
    
    /* Labels for goto targets */
    state_a:
    state_b:
    state_c:
    state_d:
    
    for (int i = 0; i < iterations; i++) {
        /* Complex arithmetic chain */
        s1 = s0 * s2 - s3;
        s2 = s1 ^ s4 | s5;
        s3 = s2 + s6 / (s7 + 1);
        s4 = s3 * s8 + s9;
        s5 = s4 & s10 | s11;
        s6 = s5 + s0 - s1;
        s7 = s6 * s2;
        s8 = s7 ^ s3;
        s9 = s8 + s4 - s5;
        s10 = s9 * s6;
        s11 = s10 ^ s7;
        
        /* FP operations */
        sf0 = sf1 + sf2 * 0.5f;
        sf1 = sf0 - sf2 / 1.5f;
        sf2 = sf1 * 2.0f + sf0;
        
        /* Double operations */
        sd0 = sd1 * 1.1 + sd2;
        sd1 = sd0 - sd2 / 2.1;
        sd2 = sd1 + sd0 * 0.9;
        
        /* Long operations */
        sl0 = sl1 + sl2 * 2;
        sl1 = sl0 ^ sl2;
        sl2 = sl1 - sl0 + 1;
        
        /* Switch with goto creating irreducible CFG */
        switch (state) {
            case 0:
                if ((i & 1) == 0) {
                    /* Jump to label outside switch */
                    state = 1;
                    goto state_b;
                }
                s0 = s11 + i;
                break;
            case 1:
                if ((i & 3) == 0) {
                    /* Jump to different case label */
                    state = 2;
                    goto state_c;
                }
                s0 = s10 - i;
                break;
            case 2:
                if ((i & 7) == 0) {
                    /* Jump to label before the loop */
                    state = 3;
                    goto state_a;
                }
                s0 = s9 * i;
                break;
            case 3:
                if ((i & 15) == 0) {
                    /* Jump to label after switch */
                    state = 0;
                    goto state_d;
                }
                s0 = s8 / (i + 1);
                break;
        }
        
        state_d:
        /* More operations to keep variables live */
        s0 = s0 + s1 - s2 + s3 - s4 + s5 - s6 + s7 - s8 + s9 - s10 + s11;
        
        /* Update state cyclically */
        state = (state + 1) & 3;
    }
    
    checksum = s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8 + s9 + s10 + s11 +
               (uint64_t)sf0 + (uint64_t)sf1 + (uint64_t)sf2 +
               (uint64_t)sd0 + (uint64_t)sd1 + (uint64_t)sd2 +
               sl0 + sl1 + sl2;
    
    KEEP_ALIVE(s0); KEEP_ALIVE(s1); KEEP_ALIVE(s2); KEEP_ALIVE(s3);
    KEEP_ALIVE(s4); KEEP_ALIVE(s5); KEEP_ALIVE(s6); KEEP_ALIVE(s7);
    KEEP_ALIVE(s8); KEEP_ALIVE(s9); KEEP_ALIVE(s10); KEEP_ALIVE(s11);
    KEEP_ALIVE(sf0); KEEP_ALIVE(sf1); KEEP_ALIVE(sf2);
    KEEP_ALIVE(sd0); KEEP_ALIVE(sd1); KEEP_ALIVE(sd2);
    KEEP_ALIVE(sl0); KEEP_ALIVE(sl1); KEEP_ALIVE(sl2);
    
    return checksum;
}

/* Function 3: Computed goto simulating a state machine */
NOINLINE static uint64_t test_computed_goto(int iterations, int seed) {
    /* Even more variables for maximum register pressure */
    int c0 = seed, c1 = seed * 2, c2 = seed * 3, c3 = seed * 4, c4 = seed * 5;
    int c5 = seed * 6, c6 = seed * 7, c7 = seed * 8, c8 = seed * 9, c9 = seed * 10;
    int c10 = seed * 11, c11 = seed * 12, c12 = seed * 13, c13 = seed * 14;
    int c14 = seed * 15, c15 = seed * 16, c16 = seed * 17, c17 = seed * 18;
    int c18 = seed * 19, c19 = seed * 20, c20 = seed * 21, c21 = seed * 22;
    float cf0 = seed * 0.05f, cf1 = seed * 0.10f, cf2 = seed * 0.15f;
    float cf3 = seed * 0.20f, cf4 = seed * 0.25f, cf5 = seed * 0.30f;
    double cd0 = seed * 0.005, cd1 = seed * 0.010, cd2 = seed * 0.015;
    double cd3 = seed * 0.020, cd4 = seed * 0.025, cd5 = seed * 0.030;
    long cl0 = seed * 50, cl1 = seed * 100, cl2 = seed * 150;
    long cl3 = seed * 200, cl4 = seed * 250, cl5 = seed * 300;
    
    /* Labels for computed goto */
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3, 
                            &&label4, &&label5, &&label6, &&label7 };
    
    uint64_t checksum = 0;
    int state = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Massive arithmetic chain to create register pressure */
        c1 = c0 * c2 + c3;
        c2 = c1 ^ c4 | c5;
        c3 = c2 + c6 - c7;
        c4 = c3 * c8 / (c9 + 1);
        c5 = c4 & c10 | c11;
        c6 = c5 + c12 - c13;
        c7 = c6 * c14 + c15;
        c8 = c7 ^ c16;
        c9 = c8 + c17 - c18;
        c10 = c9 * c19;
        c11 = c10 ^ c20 | c21;
        c12 = c11 + c0 - c1;
        c13 = c12 * c2;
        c14 = c13 ^ c3;
        c15 = c14 + c4 - c5;
        c16 = c15 * c6;
        c17 = c16 ^ c7;
        c18 = c17 + c8 - c9;
        c19 = c18 * c10;
        c20 = c19 ^ c11;
        c21 = c20 + c12 - c13;
        
        /* Floating point operations */
        cf0 = cf1 + cf2 * cf3;
        cf1 = cf0 - cf2 / (cf3 + 0.1f);
        cf2 = cf1 * 1.2f + cf3;
        cf3 = cf2 - cf0 * 0.8f;
        cf4 = cf3 + cf5 * 1.5f;
        cf5 = cf4 - cf0 / 2.0f;
        
        /* Double operations */
        cd0 = cd1 * cd2 + cd3;
        cd1 = cd0 - cd2 / (cd3 + 0.01);
        cd2 = cd1 + cd3 * 1.1;
        cd3 = cd2 - cd0 * 0.9;
        cd4 = cd3 + cd5 * 1.2;
        cd5 = cd4 - cd1 / 1.5;
        
        /* Long operations */
        cl0 = cl1 + cl2 * cl3;
        cl1 = cl0 ^ cl2 | cl3;
        cl2 = cl1 + cl3 - cl0;
        cl3 = cl2 * 3 + cl1;
        cl4 = cl3 + cl5 * 2;
        cl5 = cl4 ^ cl0;
        
        /* Computed goto - creates complex CFG */
        goto *labels[state];
        
        label0:
            c0 = c21 + i;
            state = (state + 1) & 7;
            continue;
        label1:
            c0 = c20 - i;
            state = (state + 3) & 7;
            continue;
        label2:
            c0 = c19 * i;
            state = (state + 5) & 7;
            continue;
        label3:
            c0 = c18 / (i + 1);
            state = (state + 2) & 7;
            continue;
        label4:
            c0 = c17 ^ i;
            state = (state + 4) & 7;
            continue;
        label5:
            c0 = c16 + i * 2;
            state = (state + 1) & 7;
            continue;
        label6:
            c0 = c15 - i * 3;
            state = (state + 6) & 7;
            continue;
        label7:
            c0 = c14 * (i + 2);
            state = (state + 7) & 7;
            continue;
    }
    
    checksum = c0 + c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8 + c9 + c10 +
               c11 + c12 + c13 + c14 + c15 + c16 + c17 + c18 + c19 + c20 + c21 +
               (uint64_t)cf0 + (uint64_t)cf1 + (uint64_t)cf2 + (uint64_t)cf3 +
               (uint64_t)cf4 + (uint64_t)cf5 +
               (uint64_t)cd0 + (uint64_t)cd1 + (uint64_t)cd2 + (uint64_t)cd3 +
               (uint64_t)cd4 + (uint64_t)cd5 +
               cl0 + cl1 + cl2 + cl3 + cl4 + cl5;
    
    /* Mark all variables as used */
    KEEP_ALIVE(c0); KEEP_ALIVE(c1); KEEP_ALIVE(c2); KEEP_ALIVE(c3);
    KEEP_ALIVE(c4); KEEP_ALIVE(c5); KEEP_ALIVE(c6); KEEP_ALIVE(c7);
    KEEP_ALIVE(c8); KEEP_ALIVE(c9); KEEP_ALIVE(c10); KEEP_ALIVE(c11);
    KEEP_ALIVE(c12); KEEP_ALIVE(c13); KEEP_ALIVE(c14); KEEP_ALIVE(c15);
    KEEP_ALIVE(c16); KEEP_ALIVE(c17); KEEP_ALIVE(c18); KEEP_ALIVE(c19);
    KEEP_ALIVE(c20); KEEP_ALIVE(c21);
    KEEP_ALIVE(cf0); KEEP_ALIVE(cf1); KEEP_ALIVE(cf2);
    KEEP_ALIVE(cf3); KEEP_ALIVE(cf4); KEEP_ALIVE(cf5);
    KEEP_ALIVE(cd0); KEEP_ALIVE(cd1); KEEP_ALIVE(cd2);
    KEEP_ALIVE(cd3); KEEP_ALIVE(cd4); KEEP_ALIVE(cd5);
    KEEP_ALIVE(cl0); KEEP_ALIVE(cl1); KEEP_ALIVE(cl2);
    KEEP_ALIVE(cl3); KEEP_ALIVE(cl4); KEEP_ALIVE(cl5);
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int iterations = 10000;
    int seed = time(NULL);
    uint64_t total_checksum = 0;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10000;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Running MCF coverage test with iterations=%d, seed=%d\n", 
           iterations, seed);
    
    /* Run all test functions to trigger different CFG patterns */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    printf("Final checksum: %lu\n", total_checksum);
    
    return 0;
}

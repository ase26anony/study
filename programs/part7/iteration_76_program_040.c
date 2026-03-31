/* mcf_coverage.c - Program to trigger MCF fixup graph debugging output */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define KEEP_ALIVE asm volatile("" : : "r"(var))

/* Complex irreducible CFG with goto jumps */
NOINLINE unsigned long test_irreducible_goto(int iterations, int seed) {
    /* Many local variables to create register pressure */
    volatile int v0 = seed + 1;
    int v1 = seed + 2, v2 = seed + 3, v3 = seed + 4, v4 = seed + 5;
    int v5 = seed + 6, v6 = seed + 7, v7 = seed + 8, v8 = seed + 9, v9 = seed + 10;
    float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f, f3 = seed * 0.4f;
    double d0 = seed * 0.01, d1 = seed * 0.02, d2 = seed * 0.03, d3 = seed * 0.04;
    long l0 = seed * 100, l1 = seed * 200, l2 = seed * 300, l3 = seed * 400;
    int v10 = seed + 11, v11 = seed + 12, v12 = seed + 13, v13 = seed + 14;
    int v14 = seed + 15, v15 = seed + 16, v16 = seed + 17, v17 = seed + 18;
    int v18 = seed + 19, v19 = seed + 20;
    unsigned long checksum = 0;
    
    /* Labels for irreducible loop */
    loop_start:
    inner_loop:
    after_inner:
    outside_loop:
    
    for (int i = 0; i < iterations; i++) {
        /* Complex arithmetic to keep variables live */
        v0 = v1 + v2 * v3 - v4 / (v5 + 1);
        v1 = v2 ^ v3 | v4 & v5;
        v2 = v3 * v4 - v5 + v6;
        v3 = v4 + v5 - v6 * v7;
        v4 = v5 ^ v6 | v7 & v8;
        v5 = v6 * v7 - v8 + v9;
        v6 = v7 + v8 - v9 * v10;
        v7 = v8 ^ v9 | v10 & v11;
        v8 = v9 * v10 - v11 + v12;
        v9 = v10 + v11 - v12 * v13;
        
        /* Floating point operations */
        f0 = f1 * f2 + f3;
        f1 = f2 - f3 * f0;
        f2 = f3 + f0 / (f1 + 0.001f);
        f3 = f0 * f1 - f2;
        
        /* Double precision */
        d0 = d1 + d2 * d3;
        d1 = d2 - d3 / (d0 + 0.0001);
        d2 = d3 * d0 + d1;
        d3 = d0 - d1 * d2;
        
        /* Long operations */
        l0 = l1 + l2 - l3;
        l1 = l2 * l3 + l0;
        l2 = l3 - l0 * l1;
        l3 = l0 + l1 - l2;
        
        /* More variables */
        v10 = v11 + v12 - v13 * v14;
        v11 = v12 ^ v13 | v14 & v15;
        v12 = v13 * v14 - v15 + v16;
        v13 = v14 + v15 - v16 * v17;
        v14 = v15 ^ v16 | v17 & v18;
        v15 = v16 * v17 - v18 + v19;
        v16 = v17 + v18 - v19 * v0;
        v17 = v18 ^ v19 | v0 & v1;
        v18 = v19 * v0 - v1 + v2;
        v19 = v0 + v1 - v2 * v3;
        
        /* Irreducible control flow using goto */
        if ((i % 7) == 0) {
            goto inner_loop;  /* Jump into inner loop */
        }
        if ((i % 13) == 0) {
            goto outside_loop; /* Jump out of loop structure */
        }
        if ((i % 5) == 0) {
            goto after_inner; /* Jump to middle */
        }
        
        continue;
        
        inner_loop:
        /* More operations in inner loop label */
        v0 = v0 ^ v19;
        v19 = v19 ^ v0;
        if ((i % 3) == 0) {
            goto loop_start; /* Jump back to start */
        }
        goto after_inner;
        
        after_inner:
        v1 = v1 ^ v18;
        v18 = v18 ^ v1;
        if ((i % 11) == 0) {
            goto inner_loop; /* Jump back to inner */
        }
        continue;
        
        outside_loop:
        v2 = v2 ^ v17;
        v17 = v17 ^ v2;
        if ((i % 17) == 0) {
            goto loop_start; /* Jump back into loop */
        }
        /* Force use of all variables to keep them live */
        KEEP_ALIVE;
    }
    
    /* Aggregate checksum */
    checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
               (int)f0 + (int)f1 + (int)f2 + (int)f3 +
               (long)d0 + (long)d1 + (long)d2 + (long)d3 +
               l0 + l1 + l2 + l3 +
               v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
    
    return checksum;
}

/* Complex switch with goto creating irreducible regions */
NOINLINE unsigned long test_switch_goto(int iterations, int seed) {
    volatile int state = seed % 5;
    int a0 = seed, a1 = seed + 1, a2 = seed + 2, a3 = seed + 3, a4 = seed + 4;
    int a5 = seed + 5, a6 = seed + 6, a7 = seed + 7, a8 = seed + 8, a9 = seed + 9;
    int a10 = seed + 10, a11 = seed + 11, a12 = seed + 12, a13 = seed + 13;
    int a14 = seed + 14, a15 = seed + 15, a16 = seed + 16, a17 = seed + 17;
    int a18 = seed + 18, a19 = seed + 19;
    float fa0 = seed * 0.5f, fa1 = seed * 1.5f, fa2 = seed * 2.5f;
    double da0 = seed * 0.05, da1 = seed * 0.15, da2 = seed * 0.25;
    unsigned long checksum = 0;
    
    /* Labels for switch jumps */
    case_label_1:
    case_label_2:
    case_label_3:
    loop_exit_label:
    
    for (int i = 0; i < iterations; i++) {
        /* Heavy arithmetic */
        a0 = a1 * a2 - a3 + a4 / (a5 + 1);
        a1 = a2 ^ a3 | a4 & a5;
        a2 = a3 + a4 - a5 * a6;
        a3 = a4 * a5 - a6 + a7;
        a4 = a5 + a6 - a7 * a8;
        a5 = a6 ^ a7 | a8 & a9;
        a6 = a7 * a8 - a9 + a10;
        a7 = a8 + a9 - a10 * a11;
        a8 = a9 ^ a10 | a11 & a12;
        a9 = a10 * a11 - a12 + a13;
        
        fa0 = fa1 + fa2 * 0.3f;
        fa1 = fa2 - fa0 * 0.7f;
        fa2 = fa0 * fa1 + 1.0f;
        
        da0 = da1 + da2 * 0.1;
        da1 = da2 - da0 * 0.3;
        da2 = da0 * da1 + 0.5;
        
        a10 = a11 + a12 - a13 * a14;
        a11 = a12 ^ a13 | a14 & a15;
        a12 = a13 * a14 - a15 + a16;
        a13 = a14 + a15 - a16 * a17;
        a14 = a15 ^ a16 | a17 & a18;
        a15 = a16 * a17 - a18 + a19;
        a16 = a17 + a18 - a19 * a0;
        a17 = a18 ^ a19 | a0 & a1;
        a18 = a19 * a0 - a1 + a2;
        a19 = a0 + a1 - a2 * a3;
        
        /* Complex switch with goto to labels outside */
        switch (state) {
            case 0:
                a0 = a1 + a2;
                state = (state + 1) % 5;
                if ((i % 3) == 0) goto case_label_2;
                break;
            case 1:
                a1 = a2 * a3;
                state = (state + i) % 5;
                if ((i % 7) == 0) goto case_label_3;
                break;
            case 2:
                a2 = a3 - a4;
                state = (state * 2) % 5;
                if ((i % 5) == 0) goto case_label_1;
                break;
            case 3:
                a3 = a4 ^ a5;
                state = (state + 3) % 5;
                if ((i % 11) == 0) goto loop_exit_label;
                break;
            case 4:
                a4 = a5 | a6;
                state = (state * 3) % 5;
                if ((i % 13) == 0) goto case_label_1;
                break;
        }
        
        continue;
        
        case_label_1:
        a5 = a6 + a7;
        if ((i % 17) == 0) goto case_label_3;
        continue;
        
        case_label_2:
        a6 = a7 * a8;
        if ((i % 19) == 0) goto case_label_1;
        continue;
        
        case_label_3:
        a7 = a8 - a9;
        if ((i % 23) == 0) goto case_label_2;
        continue;
        
        loop_exit_label:
        a8 = a9 ^ a10;
        if ((i % 29) == 0) {
            /* Jump back into the loop */
            state = 0;
            continue;
        }
        KEEP_ALIVE;
    }
    
    checksum = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
               a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 +
               (int)fa0 + (int)fa1 + (int)fa2 +
               (long)da0 + (long)da1 + (long)da2;
    
    return checksum;
}

/* Computed goto state machine */
NOINLINE unsigned long test_computed_goto(int iterations, int seed) {
    static void* labels[] = { &&state0, &&state1, &&state2, &&state3, &&state4, &&state_exit };
    
    volatile int state = seed % 5;
    int s0 = seed, s1 = seed + 1, s2 = seed + 2, s3 = seed + 3, s4 = seed + 4;
    int s5 = seed + 5, s6 = seed + 6, s7 = seed + 7, s8 = seed + 8, s9 = seed + 9;
    int s10 = seed + 10, s11 = seed + 11, s12 = seed + 12, s13 = seed + 13;
    int s14 = seed + 14, s15 = seed + 15, s16 = seed + 16, s17 = seed + 17;
    int s18 = seed + 18, s19 = seed + 19;
    float fs0 = seed * 1.1f, fs1 = seed * 2.2f, fs2 = seed * 3.3f, fs3 = seed * 4.4f;
    double ds0 = seed * 0.11, ds1 = seed * 0.22, ds2 = seed * 0.33, ds3 = seed * 0.44;
    unsigned long checksum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Arithmetic chains */
        s0 = s1 + s2 * s3 - s4 / (s5 + 1);
        s1 = s2 ^ s3 | s4 & s5;
        s2 = s3 * s4 - s5 + s6;
        s3 = s4 + s5 - s6 * s7;
        s4 = s5 ^ s6 | s7 & s8;
        s5 = s6 * s7 - s8 + s9;
        s6 = s7 + s8 - s9 * s10;
        s7 = s8 ^ s9 | s10 & s11;
        s8 = s9 * s10 - s11 + s12;
        s9 = s10 + s11 - s12 * s13;
        
        fs0 = fs1 * fs2 + fs3;
        fs1 = fs2 - fs3 * fs0;
        fs2 = fs3 + fs0 / (fs1 + 0.001f);
        fs3 = fs0 * fs1 - fs2;
        
        ds0 = ds1 + ds2 * ds3;
        ds1 = ds2 - ds3 / (ds0 + 0.0001);
        ds2 = ds3 * ds0 + ds1;
        ds3 = ds0 - ds1 * ds2;
        
        s10 = s11 + s12 - s13 * s14;
        s11 = s12 ^ s13 | s14 & s15;
        s12 = s13 * s14 - s15 + s16;
        s13 = s14 + s15 - s16 * s17;
        s14 = s15 ^ s16 | s17 & s18;
        s15 = s16 * s17 - s18 + s19;
        s16 = s17 + s18 - s19 * s0;
        s17 = s18 ^ s19 | s0 & s1;
        s18 = s19 * s0 - s1 + s2;
        s19 = s0 + s1 - s2 * s3;
        
        /* Computed goto */
        goto *labels[state];
        
        state0:
            s0 = s1 + s2;
            state = (state + 1) % 6;
            if ((i % 2) == 0) goto *labels[2];
            continue;
            
        state1:
            s1 = s2 * s3;
            state = (state + i) % 6;
            if ((i % 3) == 0) goto *labels[3];
            continue;
            
        state2:
            s2 = s3 - s4;
            state = (state * 2) % 6;
            if ((i % 5) == 0) goto *labels[4];
            continue;
            
        state3:
            s3 = s4 ^ s5;
            state = (state + 3) % 6;
            if ((i % 7) == 0) goto *labels[0];
            continue;
            
        state4:
            s4 = s5 | s6;
            state = (state * 3) % 6;
            if ((i % 11) == 0) goto *labels[1];
            continue;
            
        state_exit:
            s5 = s6 + s7;
            if ((i % 13) == 0) {
                state = 0;
                continue;
            }
            KEEP_ALIVE;
    }
    
    checksum = s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8 + s9 + s10 +
               s11 + s12 + s13 + s14 + s15 + s16 + s17 + s18 + s19 +
               (int)fs0 + (int)fs1 + (int)fs2 + (int)fs3 +
               (long)ds0 + (long)ds1 + (long)ds2 + (long)ds3;
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int iterations = 10000;
    int seed = time(NULL);
    unsigned long total_checksum = 0;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10000;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Running MCF coverage test with iterations=%d, seed=%d\n", 
           iterations, seed);
    
    /* Run all test functions */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    printf("Total checksum: %lu\n", total_checksum);
    
    return 0;
}

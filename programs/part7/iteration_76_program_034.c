/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow algorithm
   with fixup graph creation for coverage of special node printing */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
__attribute__((noinline,noipa))
unsigned long test_irreducible_goto(int iterations, int seed) {
    /* Many local variables to create register pressure */
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
    volatile double d0 = seed * 0.01;
    volatile double d1 = seed * 0.02;
    volatile double d2 = seed * 0.03;
    volatile double d3 = seed * 0.04;
    volatile long l0 = seed * 100;
    volatile long l1 = seed * 200;
    volatile long l2 = seed * 300;
    volatile long l3 = seed * 400;
    volatile long l4 = seed * 500;
    volatile long l5 = seed * 600;
    volatile long l6 = seed * 700;
    volatile long l7 = seed * 800;
    volatile long l8 = seed * 900;
    volatile long l9 = seed * 1000;
    
    unsigned long checksum = 0;
    int i;
    
    /* Create irreducible loop with goto jumping across boundaries */
    for (i = 0; i < iterations; i++) {
        /* Force all variables to be live */
        asm volatile("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4));
        asm volatile("" : : "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9));
        asm volatile("" : : "r"(f0), "r"(f1), "r"(f2), "r"(f3));
        asm volatile("" : : "r"(d0), "r"(d1), "r"(d2), "r"(d3));
        asm volatile("" : : "r"(l0), "r"(l1), "r"(l2), "r"(l3), "r"(l4));
        asm volatile("" : : "r"(l5), "r"(l6), "r"(l7), "r"(l8), "r"(l9));
        
        /* Complex arithmetic chains keeping variables live */
        v0 = v1 + v2 * v3 - v4 / (v5 + 1);
        v1 = v2 + v3 * v4 - v5 / (v6 + 1);
        v2 = v3 + v4 * v5 - v6 / (v7 + 1);
        v3 = v4 + v5 * v6 - v7 / (v8 + 1);
        v4 = v5 + v6 * v7 - v8 / (v9 + 1);
        
        f0 = f1 * 1.1f + f2 * 0.9f - f3 * 0.5f;
        f1 = f2 * 1.2f + f3 * 0.8f - f0 * 0.6f;
        f2 = f3 * 1.3f + f0 * 0.7f - f1 * 0.7f;
        f3 = f0 * 1.4f + f1 * 0.6f - f2 * 0.8f;
        
        d0 = d1 * 1.01 + d2 * 0.99 - d3 * 0.55;
        d1 = d2 * 1.02 + d3 * 0.98 - d0 * 0.66;
        d2 = d3 * 1.03 + d0 * 0.97 - d1 * 0.77;
        d3 = d0 * 1.04 + d1 * 0.96 - d2 * 0.88;
        
        l0 = l1 + l2 - l3 * l4 / (l5 + 1);
        l1 = l2 + l3 - l4 * l5 / (l6 + 1);
        l2 = l3 + l4 - l5 * l6 / (l7 + 1);
        l3 = l4 + l5 - l6 * l7 / (l8 + 1);
        l4 = l5 + l6 - l7 * l8 / (l9 + 1);
        
        /* Irreducible control flow with goto */
        if (i % 7 == 0) {
            goto inner_label1;
        } else if (i % 11 == 0) {
            goto inner_label2;
        }
        
        continue;
        
    inner_label1:
        /* More arithmetic */
        v5 = v6 + v7 * v8 - v9 / (v0 + 1);
        v6 = v7 + v8 * v9 - v0 / (v1 + 1);
        l5 = l6 + l7 - l8 * l9 / (l0 + 1);
        l6 = l7 + l8 - l9 * l0 / (l1 + 1);
        
        if (i % 3 == 0) {
            goto outer_label;
        } else {
            goto inner_label2;
        }
        
    inner_label2:
        v7 = v8 + v9 * v0 - v1 / (v2 + 1);
        v8 = v9 + v0 * v1 - v2 / (v3 + 1);
        l7 = l8 + l9 - l0 * l1 / (l2 + 1);
        l8 = l9 + l0 - l1 * l2 / (l3 + 1);
        
        if (i % 5 == 0) {
            goto inner_label1;
        }
        
    outer_label:
        /* Continue loop */
        checksum += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
        checksum += (unsigned long)(f0 + f1 + f2 + f3);
        checksum += (unsigned long)(d0 + d1 + d2 + d3);
        checksum += l0 + l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9;
    }
    
    return checksum;
}

__attribute__((noinline,noipa))
unsigned long test_switch_goto(int iterations, int seed) {
    /* Another set of many variables */
    volatile int a0 = seed, a1 = seed+1, a2 = seed+2, a3 = seed+3, a4 = seed+4;
    volatile int a5 = seed+5, a6 = seed+6, a7 = seed+7, a8 = seed+8, a9 = seed+9;
    volatile int b0 = seed+10, b1 = seed+11, b2 = seed+12, b3 = seed+13, b4 = seed+14;
    volatile float fa0 = seed*0.11f, fa1 = seed*0.22f, fa2 = seed*0.33f, fa3 = seed*0.44f;
    volatile double da0 = seed*0.011, da1 = seed*0.022, da2 = seed*0.033, da3 = seed*0.044;
    volatile long la0 = seed*111, la1 = seed*222, la2 = seed*333, la3 = seed*444;
    volatile long la4 = seed*555, la5 = seed*666, la6 = seed*777, la7 = seed*888;
    
    unsigned long checksum = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Keep variables live */
        asm volatile("" : : 
            "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4),
            "r"(a5), "r"(a6), "r"(a7), "r"(a8), "r"(a9),
            "r"(b0), "r"(b1), "r"(b2), "r"(b3), "r"(b4),
            "r"(fa0), "r"(fa1), "r"(fa2), "r"(fa3),
            "r"(da0), "r"(da1), "r"(da2), "r"(da3),
            "r"(la0), "r"(la1), "r"(la2), "r"(la3),
            "r"(la4), "r"(la5), "r"(la6), "r"(la7));
        
        /* Long arithmetic chains */
        a0 = a1 * a2 - a3 + a4 / (a5 + 1);
        a1 = a2 * a3 - a4 + a5 / (a6 + 1);
        a2 = a3 * a4 - a5 + a6 / (a7 + 1);
        a3 = a4 * a5 - a6 + a7 / (a8 + 1);
        a4 = a5 * a6 - a7 + a8 / (a9 + 1);
        
        fa0 = fa1 * 1.5f - fa2 * 0.5f + fa3;
        fa1 = fa2 * 1.6f - fa3 * 0.6f + fa0;
        fa2 = fa3 * 1.7f - fa0 * 0.7f + fa1;
        fa3 = fa0 * 1.8f - fa1 * 0.8f + fa2;
        
        /* Switch with goto creating irreducible flow */
        switch (i % 13) {
            case 0:
                a5 = a6 + a7 - a8 * a9;
                goto label_x;
            case 1:
                a6 = a7 + a8 - a9 * a0;
                goto label_y;
            case 2:
                a7 = a8 + a9 - a0 * a1;
                goto label_z;
            case 3:
                a8 = a9 + a0 - a1 * a2;
                /* fall through */
            case 4:
                a9 = a0 + a1 - a2 * a3;
                goto label_x;
            case 5:
                b0 = b1 + b2 - b3 * b4;
                goto label_y;
            case 6:
                b1 = b2 + b3 - b4 * a0;
                goto label_z;
            default:
                b2 = b3 + b4 - a0 * a1;
                /* continue in loop */
                break;
        }
        
        /* More arithmetic before checksum */
        da0 = da1 * 1.11 - da2 * 0.55 + da3;
        da1 = da2 * 1.22 - da3 * 0.66 + da0;
        da2 = da3 * 1.33 - da0 * 0.77 + da1;
        da3 = da0 * 1.44 - da1 * 0.88 + da2;
        
        la0 = la1 + la2 * la3 - la4;
        la1 = la2 + la3 * la4 - la5;
        la2 = la3 + la4 * la5 - la6;
        la3 = la4 + la5 * la6 - la7;
        
        checksum += a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9;
        checksum += b0 + b1 + b2 + b3 + b4;
        checksum += (unsigned long)(fa0 + fa1 + fa2 + fa3);
        checksum += (unsigned long)(da0 + da1 + da2 + da3);
        checksum += la0 + la1 + la2 + la3 + la4 + la5 + la6 + la7;
        continue;
        
    label_x:
        a0 = a1 * 2 - a2;
        goto continue_loop;
        
    label_y:
        a1 = a2 * 3 - a3;
        goto continue_loop;
        
    label_z:
        a2 = a3 * 4 - a4;
        /* fall through to continue_loop */
        
    continue_loop:
        /* More operations before continuing */
        la4 = la5 + la6 - la7 * la0;
        la5 = la6 + la7 - la0 * la1;
        checksum += i * 31;
    }
    
    return checksum;
}

/* State machine using computed goto */
__attribute__((noinline,noipa))
unsigned long test_computed_goto(int iterations, int seed) {
    volatile int s0 = seed, s1 = seed+1, s2 = seed+2, s3 = seed+3, s4 = seed+4;
    volatile int s5 = seed+5, s6 = seed+6, s7 = seed+7, s8 = seed+8, s9 = seed+9;
    volatile int t0 = seed+10, t1 = seed+11, t2 = seed+12, t3 = seed+13, t4 = seed+14;
    volatile float fs0 = seed*0.15f, fs1 = seed*0.25f, fs2 = seed*0.35f, fs3 = seed*0.45f;
    volatile double ds0 = seed*0.015, ds1 = seed*0.025, ds2 = seed*0.035, ds3 = seed*0.045;
    volatile long ls0 = seed*150, ls1 = seed*250, ls2 = seed*350, ls3 = seed*450;
    volatile long ls4 = seed*550, ls5 = seed*650, ls6 = seed*750, ls7 = seed*850;
    
    /* Labels for computed goto */
    static void* labels[] = {
        &&state0, &&state1, &&state2, &&state3,
        &&state4, &&state5, &&state6, &&state7
    };
    
    unsigned long checksum = 0;
    int i;
    int state = 0;
    
    for (i = 0; i < iterations; i++) {
        /* Keep all variables live */
        asm volatile("" : : 
            "r"(s0), "r"(s1), "r"(s2), "r"(s3), "r"(s4),
            "r"(s5), "r"(s6), "r"(s7), "r"(s8), "r"(s9),
            "r"(t0), "r"(t1), "r"(t2), "r"(t3), "r"(t4),
            "r"(fs0), "r"(fs1), "r"(fs2), "r"(fs3),
            "r"(ds0), "r"(ds1), "r"(ds2), "r"(ds3),
            "r"(ls0), "r"(ls1), "r"(ls2), "r"(ls3),
            "r"(ls4), "r"(ls5), "r"(ls6), "r"(ls7));
        
        /* Arithmetic operations */
        s0 = s1 + s2 * s3 - s4 / (s5 + 1);
        s1 = s2 + s3 * s4 - s5 / (s6 + 1);
        s2 = s3 + s4 * s5 - s6 / (s7 + 1);
        s3 = s4 + s5 * s6 - s7 / (s8 + 1);
        s4 = s5 + s6 * s7 - s8 / (s9 + 1);
        
        fs0 = fs1 * 1.15f - fs2 * 0.65f + fs3;
        fs1 = fs2 * 1.25f - fs3 * 0.75f + fs0;
        fs2 = fs3 * 1.35f - fs0 * 0.85f + fs1;
        fs3 = fs0 * 1.45f - fs1 * 0.95f + fs2;
        
        /* Computed goto state machine */
        goto *labels[state];
        
    state0:
        t0 = t1 + t2 - t3 * t4;
        state = (i % 3 == 0) ? 1 : 4;
        goto after_state;
        
    state1:
        t1 = t2 + t3 - t4 * s0;
        state = (i % 5 == 0) ? 2 : 5;
        goto after_state;
        
    state2:
        t2 = t3 + t4 - s0 * s1;
        state = (i % 7 == 0) ? 3 : 6;
        goto after_state;
        
    state3:
        t3 = t4 + s0 - s1 * s2;
        state = (i % 11 == 0) ? 0 : 7;
        goto after_state;
        
    state4:
        t4 = s0 + s1 - s2 * s3;
        state = (i % 2 == 0) ? 5 : 1;
        goto after_state;
        
    state5:
        s5 = s6 + s7 - s8 * s9;
        state = (i % 13 == 0) ? 6 : 2;
        goto after_state;
        
    state6:
        s6 = s7 + s8 - s9 * t0;
        state = (i % 17 == 0) ? 7 : 3;
        goto after_state;
        
    state7:
        s7 = s8 + s9 - t0 * t1;
        state = (i % 19 == 0) ? 0 : 4;
        /* fall through */
        
    after_state:
        /* More arithmetic */
        ds0 = ds1 * 1.015 - ds2 * 0.515 + ds3;
        ds1 = ds2 * 1.025 - ds3 * 0.525 + ds0;
        ds2 = ds3 * 1.035 - ds0 * 0.535 + ds1;
        ds3 = ds0 * 1.045 - ds1 * 0.545 + ds2;
        
        ls0 = ls1 + ls2 * ls3 - ls4;
        ls1 = ls2 + ls3 * ls4 - ls5;
        ls2 = ls3 + ls4 * ls5 - ls6;
        ls3 = ls4 + ls5 * ls6 - ls7;
        ls4 = ls5 + ls6 * ls7 - ls0;
        
        checksum += s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8 + s9;
        checksum += t0 + t1 + t2 + t3 + t4;
        checksum += (unsigned long)(fs0 + fs1 + fs2 + fs3);
        checksum += (unsigned long)(ds0 + ds1 + ds2 + ds3);
        checksum += ls0 + ls1 + ls2 + ls3 + ls4 + ls5 + ls6 + ls7;
        checksum += state;
    }
    
    return checksum;
}

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
    
    unsigned long total_checksum = 0;
    
    /* Call all test functions to increase coverage chance */
    total_checksum += test_irreducible_goto(iterations, seed);
    printf("test_irreducible_goto checksum: %lu\n", total_checksum);
    
    total_checksum += test_switch_goto(iterations, seed + 1);
    printf("test_switch_goto checksum: %lu\n", total_checksum);
    
    total_checksum += test_computed_goto(iterations, seed + 2);
    printf("test_computed_goto checksum: %lu\n", total_checksum);
    
    printf("Total checksum: %lu\n", total_checksum);
    
    return 0;
}

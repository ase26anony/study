/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow algorithm fixup graph debug output */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
__attribute__((noinline, noipa))
unsigned long test_irreducible_goto(int iterations, unsigned seed) {
    /* Many local variables to create register pressure */
    volatile int v0 = seed + 1, v1 = seed + 2, v2 = seed + 3, v3 = seed + 4;
    volatile int v4 = seed + 5, v5 = seed + 6, v6 = seed + 7, v7 = seed + 8;
    volatile int v8 = seed + 9, v9 = seed + 10, v10 = seed + 11, v11 = seed + 12;
    volatile float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f, f3 = seed * 0.4f;
    volatile double d0 = seed * 0.01, d1 = seed * 0.02, d2 = seed * 0.03, d3 = seed * 0.04;
    volatile long l0 = seed * 100, l1 = seed * 200, l2 = seed * 300, l3 = seed * 400;
    volatile int v12 = seed + 13, v13 = seed + 14, v14 = seed + 15, v15 = seed + 16;
    volatile int v16 = seed + 17, v17 = seed + 18, v18 = seed + 19, v19 = seed + 20;
    
    unsigned long checksum = 0;
    int i;
    
    /* Labels for irreducible loop with goto */
    loop_start:
    inner_loop:
    after_inner:
    loop_exit_point:
    
    for (i = 0; i < iterations; i++) {
        /* Complex arithmetic chains to keep variables live */
        v0 = v1 + v2; v1 = v3 * v4; v2 = v5 - v6; v3 = v7 / (v8 ? v8 : 1);
        v4 = v9 ^ v10; v5 = v11 | v12; v6 = v13 & v14; v7 = v15 << 2;
        v8 = v16 >> 1; v9 = v17 + v18; v10 = v19 - v0; v11 = v1 * v2;
        v12 = v3 + v4; v13 = v5 - v6; v14 = v7 * v8; v15 = v9 / (v10 ? v10 : 1);
        v16 = v11 ^ v12; v17 = v13 | v14; v18 = v15 & v16; v19 = v17 << 1;
        
        f0 = f1 * 1.1f; f1 = f2 + 2.2f; f2 = f3 - 3.3f; f3 = f0 * 0.5f;
        d0 = d1 * 1.01; d1 = d2 + 2.02; d2 = d3 - 3.03; d3 = d0 * 0.5;
        l0 = l1 + 1000; l1 = l2 - 500; l2 = l3 * 2; l3 = l0 >> 1;
        
        /* Irreducible control flow using goto */
        if ((i % 7) == 0) {
            goto inner_loop;
        }
        if ((i % 13) == 0) {
            goto loop_exit_point;
        }
        if ((i % 5) == 0) {
            goto after_inner;
        }
        
        /* More arithmetic to prevent dead code */
        v0 = v0 + i; v1 = v1 - i; v2 = v2 * (i & 0xFF); v3 = v3 ^ i;
        f0 = f0 + (i * 0.01f); d0 = d0 - (i * 0.001); l0 = l0 | i;
        
        continue;
        
        inner_loop:
        /* Different arithmetic pattern in inner loop */
        v4 = v4 + (i * 2); v5 = v5 - (i * 3); v6 = v6 * ((i + 1) & 0xFF);
        f1 = f1 * 1.5f; d1 = d1 * 1.05; l1 = l1 + (i << 2);
        
        if ((i % 3) == 0) {
            goto loop_start;
        }
        
        after_inner:
        v7 = v7 ^ (i * 7); v8 = v8 + (i % 19); v9 = v9 - (i % 23);
        f2 = f2 + 0.7f; d2 = d2 - 0.07; l2 = l2 ^ i;
        
        if ((i % 11) == 0) {
            goto inner_loop;
        }
        
        loop_exit_point:
        v10 = v10 * ((i % 5) + 1); v11 = v11 + (i % 7); v12 = v12 - (i % 11);
        f3 = f3 * 0.9f; d3 = d3 * 0.99; l3 = l3 + (i % 13);
    }
    
    /* Aggregate checksum from all variables */
    checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
               v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
               (unsigned long)f0 + (unsigned long)f1 + (unsigned long)f2 + (unsigned long)f3 +
               (unsigned long)d0 + (unsigned long)d1 + (unsigned long)d2 + (unsigned long)d3 +
               l0 + l1 + l2 + l3;
    
    /* Use inline assembly to mark variables as used */
    asm volatile("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3));
    asm volatile("" : : "r"(v4), "r"(v5), "r"(v6), "r"(v7));
    asm volatile("" : : "r"(v8), "r"(v9), "r"(v10), "r"(v11));
    
    return checksum;
}

__attribute__((noinline, noipa))
unsigned long test_switch_goto(int iterations, unsigned seed) {
    volatile int a0 = seed, a1 = seed * 2, a2 = seed * 3, a3 = seed * 4;
    volatile int a4 = seed * 5, a5 = seed * 6, a6 = seed * 7, a7 = seed * 8;
    volatile int a8 = seed * 9, a9 = seed * 10, a10 = seed * 11, a11 = seed * 12;
    volatile float fa0 = seed * 0.15f, fa1 = seed * 0.25f, fa2 = seed * 0.35f;
    volatile double da0 = seed * 0.015, da1 = seed * 0.025, da2 = seed * 0.035;
    volatile long la0 = seed * 150, la1 = seed * 250, la2 = seed * 350;
    
    unsigned long checksum = 0;
    int i;
    
    /* Labels for switch-based irreducible flow */
    switch_start:
    case_0_block:
    case_1_block:
    case_2_block:
    after_switch:
    
    for (i = 0; i < iterations; i++) {
        int switch_val = (i * 13 + seed) % 7;
        
        /* Long arithmetic dependency chain before switch */
        a0 = a1 + a2; a1 = a3 * a4; a2 = a5 - a6; a3 = a7 ^ a8;
        a4 = a9 | a10; a5 = a11 & a0; a6 = a1 << (i & 3); a7 = a2 >> 1;
        a8 = a3 + a4; a9 = a5 - a6; a10 = a7 * a8; a11 = a9 / ((a10 & 0xFF) ? (a10 & 0xFF) : 1);
        
        fa0 = fa1 * 1.2f; fa1 = fa2 + 2.3f; fa2 = fa0 - 3.4f;
        da0 = da1 * 1.02; da1 = da2 + 2.03; da2 = da0 - 3.04;
        la0 = la1 + 1200; la1 = la2 - 600; la2 = la0 * 3;
        
        switch (switch_val) {
            case 0:
                a0 = a0 + (i * 11);
                a1 = a1 - (i * 13);
                /* Jump to label outside switch */
                goto case_1_block;
                
            case 1:
                a2 = a2 * ((i % 17) + 1);
                a3 = a3 ^ (i * 19);
                /* Jump to different case block */
                goto case_2_block;
                
            case 2:
                a4 = a4 + (i % 29);
                a5 = a5 - (i % 31);
                /* Jump out of switch entirely */
                goto after_switch;
                
            case 3:
                a6 = a6 << 2;
                a7 = a7 >> 1;
                /* Jump back to switch start */
                goto switch_start;
                
            case 4:
                a8 = a8 | (i * 23);
                a9 = a9 & (i * 27);
                /* Jump to case 0 block */
                goto case_0_block;
                
            default:
                a10 = a10 + (i * 37);
                a11 = a11 - (i * 41);
                break;
        }
        
        case_0_block:
        a0 = a0 * 2; a1 = a1 / ((a0 & 0xFF) ? (a0 & 0xFF) : 1);
        fa0 = fa0 + 1.7f;
        if ((i % 19) == 0) goto after_switch;
        
        case_1_block:
        a2 = a2 + a3; a3 = a4 * a5;
        da0 = da0 * 1.07;
        if ((i % 23) == 0) goto switch_start;
        
        case_2_block:
        a6 = a6 ^ a7; a7 = a8 | a9;
        la0 = la0 + (i << 3);
        if ((i % 29) == 0) goto case_0_block;
        
        after_switch:
        a10 = a10 - a11; a11 = a0 & a1;
        fa1 = fa1 * 0.87f; da1 = da1 * 0.97; la1 = la1 ^ i;
        
        /* More arithmetic to ensure variables stay live */
        a0 = a0 + i; a1 = a1 - (i % 256); a2 = a2 * ((i % 128) + 1);
        a3 = a3 ^ (i * 3); a4 = a4 | (i * 5); a5 = a5 & (i * 7);
    }
    
    checksum = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11 +
               (unsigned long)fa0 + (unsigned long)fa1 + (unsigned long)fa2 +
               (unsigned long)da0 + (unsigned long)da1 + (unsigned long)da2 +
               la0 + la1 + la2;
    
    /* Force register usage */
    asm volatile("" : : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5));
    asm volatile("" : : "r"(fa0), "r"(fa1), "r"(fa2));
    
    return checksum;
}

__attribute__((noinline, noipa))
unsigned long test_computed_goto(int iterations, unsigned seed) {
    volatile int x0 = seed, x1 = seed * 3, x2 = seed * 5, x3 = seed * 7;
    volatile int x4 = seed * 9, x5 = seed * 11, x6 = seed * 13, x7 = seed * 15;
    volatile int x8 = seed * 17, x9 = seed * 19, x10 = seed * 21, x11 = seed * 23;
    volatile int x12 = seed * 25, x13 = seed * 27, x14 = seed * 29, x15 = seed * 31;
    volatile float fx0 = seed * 0.17f, fx1 = seed * 0.27f, fx2 = seed * 0.37f;
    volatile double dx0 = seed * 0.017, dx1 = seed * 0.027, dx2 = seed * 0.037;
    volatile long lx0 = seed * 170, lx1 = seed * 270, lx2 = seed * 370;
    
    /* Labels for computed goto state machine */
    static void* labels[] = {
        &&state0, &&state1, &&state2, &&state3, 
        &&state4, &&state5, &&state6, &&state7
    };
    
    unsigned long checksum = 0;
    int i, state = 0;
    
    for (i = 0; i < iterations; i++) {
        /* Extensive arithmetic before goto */
        x0 = x1 + x2; x1 = x3 * x4; x2 = x5 - x6; x3 = x7 ^ x8;
        x4 = x9 | x10; x5 = x11 & x12; x6 = x13 << (i & 3); x7 = x14 >> 1;
        x8 = x15 + x0; x9 = x1 - x2; x10 = x3 * x4; x11 = x5 / ((x6 & 0xFF) ? (x6 & 0xFF) : 1);
        x12 = x7 ^ x8; x13 = x9 | x10; x14 = x11 & x12; x15 = x13 << 2;
        
        fx0 = fx1 * 1.3f; fx1 = fx2 + 2.4f; fx2 = fx0 - 3.5f;
        dx0 = dx1 * 1.03; dx1 = dx2 + 2.04; dx2 = dx0 - 3.05;
        lx0 = lx1 + 1500; lx1 = lx2 - 750; lx2 = lx0 * 4;
        
        /* Update state based on complex condition */
        state = (state + (i * 17 + seed) % 8) % 8;
        
        /* Computed goto - creates irreducible control flow */
        goto *labels[state];
        
        state0:
        x0 = x0 + (i * 47);
        x1 = x1 - (i % 53);
        fx0 = fx0 * 1.11f;
        state = (state + 1) % 8;
        continue;
        
        state1:
        x2 = x2 * ((i % 59) + 1);
        x3 = x3 ^ (i * 61);
        dx0 = dx0 + 0.11;
        state = (state + 2) % 8;
        continue;
        
        state2:
        x4 = x4 | (i * 67);
        x5 = x5 & (i * 71);
        lx0 = lx0 + (i << 4);
        state = (state + 3) % 8;
        continue;
        
        state3:
        x6 = x6 << 3;
        x7 = x7 >> 2;
        fx1 = fx1 - 0.22f;
        state = (state + 4) % 8;
        continue;
        
        state4:
        x8 = x8 + x9;
        x9 = x10 * x11;
        dx1 = dx1 * 1.12;
        state = (state + 5) % 8;
        continue;
        
        state5:
        x10 = x10 ^ x11;
        x11 = x12 | x13;
        lx1 = lx1 ^ (i * 73);
        state = (state + 6) % 8;
        continue;
        
        state6:
        x12 = x12 - x13;
        x13 = x14 / ((x15 & 0xFF) ? (x15 & 0xFF) : 1);
        fx2 = fx2 + 0.33f;
        state = (state + 7) % 8;
        continue;
        
        state7:
        x14 = x14 & x15;
        x15 = x0 + x1;
        dx2 = dx2 - 0.44;
        lx2 = lx2 | (i * 79);
        state = 0;
        continue;
    }
    
    checksum = x0 + x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 +
               x10 + x11 + x12 + x13 + x14 + x15 +
               (unsigned long)fx0 + (unsigned long)fx1 + (unsigned long)fx2 +
               (unsigned long)dx0 + (unsigned long)dx1 + (unsigned long)dx2 +
               lx0 + lx1 + lx2;
    
    /* Mark all variables as used */
    asm volatile("" : : "r"(x0), "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5));
    asm volatile("" : : "r"(x6), "r"(x7), "r"(x8), "r"(x9), "r"(x10), "r"(x11));
    asm volatile("" : : "r"(fx0), "r"(fx1), "r"(fx2), "r"(dx0), "r"(dx1), "r"(dx2));
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int iterations = 10000;
    unsigned seed = (unsigned)time(NULL);
    unsigned long total_checksum = 0;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10000;
    }
    if (argc > 2) {
        seed = (unsigned)atoi(argv[2]);
    }
    
    printf("Running MCF coverage test with %d iterations, seed=%u\n", 
           iterations, seed);
    
    /* Run all test functions to trigger different CFG patterns */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    printf("Total checksum: %lu\n", total_checksum);
    
    return 0;
}

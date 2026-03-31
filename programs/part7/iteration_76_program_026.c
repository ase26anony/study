/* mcf_coverage.c - Program to trigger MCF fixup graph debug output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define KEEP_ALIVE(var) asm volatile("" : : "r"(var))

/* Function 1: Irreducible loop with goto jumping across loop boundaries */
NOINLINE static uint64_t test_irreducible_goto(int iterations, int seed) {
    /* Declare many variables to create register pressure */
    volatile int v0 = seed + 1;
    int v1 = seed + 2, v2 = seed + 3, v3 = seed + 4, v4 = seed + 5;
    int v5 = seed + 6, v6 = seed + 7, v7 = seed + 8, v8 = seed + 9, v9 = seed + 10;
    float f0 = seed * 0.1f, f1 = f0 + 1.1f, f2 = f1 + 2.2f, f3 = f2 + 3.3f;
    double d0 = seed * 0.01, d1 = d0 + 1.11, d2 = d1 + 2.22, d3 = d2 + 3.33;
    long l0 = seed * 100L, l1 = l0 + 111L, l2 = l1 + 222L, l3 = l2 + 333L;
    int v10 = seed + 11, v11 = seed + 12, v12 = seed + 13, v13 = seed + 14;
    int v14 = seed + 15, v15 = seed + 16, v16 = seed + 17, v17 = seed + 18;
    int v18 = seed + 19, v19 = seed + 20, v20 = seed + 21, v21 = seed + 22;
    int v22 = seed + 23, v23 = seed + 24, v24 = seed + 25, v25 = seed + 26;
    
    uint64_t checksum = 0;
    int i = 0;
    
    /* Outer loop */
    outer_loop:
    if (i >= iterations) goto done;
    
    /* Create long dependency chains */
    v1 = v0 + v2;
    v3 = v1 * v4 - v5;
    v6 = (v3 << 3) | (v2 & 0xFF);
    v7 = v6 / (v0 + 1) + v8;
    v9 = v7 ^ v1 ^ v3;
    v10 = v9 + v4 * v5;
    
    f1 = f0 * 2.0f + f2;
    f3 = f1 / (f0 + 0.5f) - f2;
    f0 = f3 * 1.1f;
    
    d1 = d0 * 1.5 + d2;
    d3 = d1 / (d0 + 0.1) - d2;
    d0 = d3 * 1.01;
    
    l1 = l0 + l2 * 3;
    l3 = l1 - l2 / 2;
    l0 = l3 ^ l1;
    
    /* Irreducible control flow: jump into inner loop */
    if ((v0 + i) % 7 == 0) {
        goto inner_loop_middle;
    }
    
    /* Inner loop */
    inner_loop_start:
    v11 = v10 + v12;
    v13 = v11 * v14 - v15;
    v16 = (v13 << 2) | (v12 & 0xF);
    v17 = v16 / (v10 + 1) + v18;
    
    inner_loop_middle:
    v19 = v17 ^ v11 ^ v13;
    v20 = v19 + v14 * v15;
    v21 = v20 - v16 + v17;
    
    /* Jump back to outer loop from inside inner loop */
    if ((v21 + i) % 11 == 0) {
        i++;
        goto outer_loop;
    }
    
    inner_loop_end:
    v22 = v21 * v23;
    v24 = v22 + v25;
    v25 = v24 ^ v0;
    
    /* Jump to start of inner loop */
    if ((v25 % 13) != 0) {
        goto inner_loop_start;
    }
    
    i++;
    goto outer_loop;
    
    done:
    /* Aggregate checksum from all variables */
    checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
               v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
               v21 + v22 + v23 + v24 + v25 +
               (uint64_t)f0 + (uint64_t)f1 + (uint64_t)f2 + (uint64_t)f3 +
               (uint64_t)d0 + (uint64_t)d1 + (uint64_t)d2 + (uint64_t)d3 +
               l0 + l1 + l2 + l3;
    
    /* Keep variables alive */
    KEEP_ALIVE(v0); KEEP_ALIVE(v1); KEEP_ALIVE(v2); KEEP_ALIVE(v3);
    KEEP_ALIVE(v4); KEEP_ALIVE(v5); KEEP_ALIVE(v6); KEEP_ALIVE(v7);
    KEEP_ALIVE(v8); KEEP_ALIVE(v9); KEEP_ALIVE(v10); KEEP_ALIVE(v11);
    KEEP_ALIVE(v12); KEEP_ALIVE(v13); KEEP_ALIVE(v14); KEEP_ALIVE(v15);
    KEEP_ALIVE(v16); KEEP_ALIVE(v17); KEEP_ALIVE(v18); KEEP_ALIVE(v19);
    KEEP_ALIVE(v20); KEEP_ALIVE(v21); KEEP_ALIVE(v22); KEEP_ALIVE(v23);
    KEEP_ALIVE(v24); KEEP_ALIVE(v25);
    KEEP_ALIVE(f0); KEEP_ALIVE(f1); KEEP_ALIVE(f2); KEEP_ALIVE(f3);
    KEEP_ALIVE(d0); KEEP_ALIVE(d1); KEEP_ALIVE(d2); KEEP_ALIVE(d3);
    KEEP_ALIVE(l0); KEEP_ALIVE(l1); KEEP_ALIVE(l2); KEEP_ALIVE(l3);
    
    return checksum;
}

/* Function 2: Switch with goto creating complex CFG */
NOINLINE static uint64_t test_switch_goto(int iterations, int seed) {
    volatile int state = seed % 5;
    int a0 = seed, a1 = seed + 1, a2 = seed + 2, a3 = seed + 3, a4 = seed + 4;
    int a5 = seed + 5, a6 = seed + 6, a7 = seed + 7, a8 = seed + 8, a9 = seed + 9;
    float fa0 = seed * 0.2f, fa1 = fa0 + 1.0f, fa2 = fa1 * 2.0f;
    double da0 = seed * 0.02, da1 = da0 * 1.5, da2 = da1 + 3.14;
    long la0 = seed * 50L, la1 = la0 * 3L, la2 = la1 / 2L;
    int b0 = seed + 10, b1 = seed + 11, b2 = seed + 12, b3 = seed + 13;
    int b4 = seed + 14, b5 = seed + 15, b6 = seed + 16, b7 = seed + 17;
    int b8 = seed + 18, b9 = seed + 19, b10 = seed + 20, b11 = seed + 21;
    
    uint64_t checksum = 0;
    
    /* Labels for goto targets */
    loop_start:
    for (int i = 0; i < iterations; i++) {
        /* Complex arithmetic creating register pressure */
        a1 = a0 * a2 - a3;
        a4 = a1 + a5 * a6;
        a7 = a4 ^ a2 ^ a8;
        a9 = a7 / (a0 + 1) + a3;
        
        fa1 = fa0 * 3.0f + fa2;
        fa2 = fa1 / (fa0 + 0.3f) - fa0;
        fa0 = fa2 * 1.2f;
        
        da1 = da0 * 2.0 + da2;
        da2 = da1 - da0 * 0.5;
        da0 = da2 / 1.1;
        
        la1 = la0 + la2 * 7;
        la2 = la1 ^ la0;
        la0 = la2 - la1 / 3;
        
        b0 = a9 + b1;
        b2 = b0 * b3 - b4;
        b5 = b2 + b6 / (b1 + 1);
        b7 = b5 ^ b3 ^ b8;
        b9 = b7 * b10 - b11;
        b11 = b9 + b0;
        
        /* Switch with goto to different labels */
        switch (state) {
            case 0:
                a0 = b1 + i;
                if ((a0 % 3) == 0) goto case_0_exit;
                state = 1;
                break;
            case 1:
                a2 = b3 * i;
                if ((a2 % 5) == 0) goto case_1_continue;
                state = 2;
                /* Fall through to next iteration */
                continue;
            case 2:
                a3 = b5 - i;
                if ((a3 % 7) == 0) goto loop_start;  /* Jump to loop start */
                state = 3;
                break;
            case 3:
                a5 = b7 ^ i;
                if ((a5 % 11) == 0) goto case_3_skip;
                state = 4;
                break;
            case 4:
                a6 = b9 + i * 2;
                state = 0;
                /* Jump to outer label */
                goto switch_end;
            default:
                state = 0;
                break;
        }
        
        case_0_exit:
        a8 = b11 * 3;
        state = 3;
        continue;
        
        case_1_continue:
        a9 = b0 / 2;
        state = 4;
        continue;
        
        case_3_skip:
        b1 = a0 * 5;
        state = 2;
        continue;
        
        switch_end:
        b2 = a6 + 100;
    }
    
    /* Aggregate checksum */
    checksum = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 +
               b0 + b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 + b11 +
               (uint64_t)fa0 + (uint64_t)fa1 + (uint64_t)fa2 +
               (uint64_t)da0 + (uint64_t)da1 + (uint64_t)da2 +
               la0 + la1 + la2 + state;
    
    /* Keep alive */
    KEEP_ALIVE(a0); KEEP_ALIVE(a1); KEEP_ALIVE(a2); KEEP_ALIVE(a3);
    KEEP_ALIVE(a4); KEEP_ALIVE(a5); KEEP_ALIVE(a6); KEEP_ALIVE(a7);
    KEEP_ALIVE(a8); KEEP_ALIVE(a9);
    KEEP_ALIVE(b0); KEEP_ALIVE(b1); KEEP_ALIVE(b2); KEEP_ALIVE(b3);
    KEEP_ALIVE(b4); KEEP_ALIVE(b5); KEEP_ALIVE(b6); KEEP_ALIVE(b7);
    KEEP_ALIVE(b8); KEEP_ALIVE(b9); KEEP_ALIVE(b10); KEEP_ALIVE(b11);
    KEEP_ALIVE(fa0); KEEP_ALIVE(fa1); KEEP_ALIVE(fa2);
    KEEP_ALIVE(da0); KEEP_ALIVE(da1); KEEP_ALIVE(da2);
    KEEP_ALIVE(la0); KEEP_ALIVE(la1); KEEP_ALIVE(la2);
    KEEP_ALIVE(state);
    
    return checksum;
}

/* Function 3: Computed goto simulating state machine */
NOINLINE static uint64_t test_computed_goto(int iterations, int seed) {
    static void* labels[] = { &&state_a, &&state_b, &&state_c, 
                             &&state_d, &&state_e, &&state_f };
    
    volatile int state = seed % 6;
    int x0 = seed, x1 = seed + 1, x2 = seed + 2, x3 = seed + 3;
    int x4 = seed + 4, x5 = seed + 5, x6 = seed + 6, x7 = seed + 7;
    int x8 = seed + 8, x9 = seed + 9, x10 = seed + 10, x11 = seed + 11;
    float fx0 = seed * 0.3f, fx1 = fx0 + 2.0f, fx2 = fx1 * 1.5f;
    double dx0 = seed * 0.03, dx1 = dx0 * 2.5, dx2 = dx1 - 1.0;
    long lx0 = seed * 75L, lx1 = lx0 * 4L, lx2 = lx1 / 3L;
    int y0 = seed + 12, y1 = seed + 13, y2 = seed + 14, y3 = seed + 15;
    int y4 = seed + 16, y5 = seed + 17, y6 = seed + 18, y7 = seed + 19;
    int y8 = seed + 20, y9 = seed + 21, y10 = seed + 22, y11 = seed + 23;
    
    uint64_t checksum = 0;
    int i = 0;
    
    /* Start of state machine */
    goto *labels[state];
    
    state_a:
    if (i >= iterations) goto finish;
    x1 = x0 * x2 - x3;
    x4 = x1 + x5;
    state = (x4 % 6);
    i++;
    goto *labels[state];
    
    state_b:
    x6 = x4 ^ x2 ^ x7;
    x8 = x6 / (x0 + 1) + x9;
    fx1 = fx0 * 4.0f + fx2;
    state = ((x8 + i) % 6);
    goto *labels[state];
    
    state_c:
    x10 = x8 * x11 - x1;
    dx1 = dx0 * 3.0 + dx2;
    lx1 = lx0 + lx2 * 5;
    state = ((x10 ^ i) % 6);
    /* Jump to different label */
    goto state_f;
    
    state_d:
    y0 = x10 + y1;
    y2 = y0 * y3 - y4;
    fx2 = fx1 / (fx0 + 0.4f) - fx0;
    state = ((y2 + i * 2) % 6);
    goto *labels[state];
    
    state_e:
    y5 = y2 + y6 / (y1 + 1);
    y7 = y5 ^ y3 ^ y8;
    dx2 = dx1 - dx0 * 0.6;
    state = ((y7 % 13) % 6);
    /* Jump back to state_a */
    goto state_a;
    
    state_f:
    y9 = y7 * y10 - y11;
    y11 = y9 + y0;
    lx2 = lx1 ^ lx0;
    fx0 = fx2 * 1.3f;
    dx0 = dx2 / 1.2;
    lx0 = lx2 - lx1 / 4;
    state = ((y11 + i * 3) % 6);
    goto *labels[state];
    
    finish:
    /* More arithmetic before returning */
    x0 = y11 + x3;
    x2 = x0 * x5 - x7;
    x9 = x2 ^ x6 ^ x8;
    
    checksum = x0 + x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10 + x11 +
               y0 + y1 + y2 + y3 + y4 + y5 + y6 + y7 + y8 + y9 + y10 + y11 +
               (uint64_t)fx0 + (uint64_t)fx1 + (uint64_t)fx2 +
               (uint64_t)dx0 + (uint64_t)dx1 + (uint64_t)dx2 +
               lx0 + lx1 + lx2 + state;
    
    /* Keep alive */
    KEEP_ALIVE(x0); KEEP_ALIVE(x1); KEEP_ALIVE(x2); KEEP_ALIVE(x3);
    KEEP_ALIVE(x4); KEEP_ALIVE(x5); KEEP_ALIVE(x6); KEEP_ALIVE(x7);
    KEEP_ALIVE(x8); KEEP_ALIVE(x9); KEEP_ALIVE(x10); KEEP_ALIVE(x11);
    KEEP_ALIVE(y0); KEEP_ALIVE(y1); KEEP_ALIVE(y2); KEEP_ALIVE(y3);
    KEEP_ALIVE(y4); KEEP_ALIVE(y5); KEEP_ALIVE(y6); KEEP_ALIVE(y7);
    KEEP_ALIVE(y8); KEEP_ALIVE(y9); KEEP_ALIVE(y10); KEEP_ALIVE(y11);
    KEEP_ALIVE(fx0); KEEP_ALIVE(fx1); KEEP_ALIVE(fx2);
    KEEP_ALIVE(dx0); KEEP_ALIVE(dx1); KEEP_ALIVE(dx2);
    KEEP_ALIVE(lx0); KEEP_ALIVE(lx1); KEEP_ALIVE(lx2);
    KEEP_ALIVE(state);
    
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
    
    printf("Running MCF coverage test with iterations=%d, seed=%d\n", 
           iterations, seed);
    
    uint64_t total_checksum = 0;
    
    /* Call all test functions to trigger different CFG patterns */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    printf("Total checksum: %lu\n", (unsigned long)total_checksum);
    
    return 0;
}

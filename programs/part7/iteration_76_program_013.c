/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow algorithm
   with fixup graph creation to cover special node printing code */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* Complex irreducible CFG with goto jumps */
NOINLINE static unsigned long test_irreducible_goto(int iterations, int seed) {
    VOLATILE_VAR int i, j, k;
    VOLATILE_VAR unsigned long checksum = seed;
    
    /* Many local variables to create register pressure */
    VOLATILE_VAR int v0 = seed, v1 = seed + 1, v2 = seed + 2, v3 = seed + 3;
    VOLATILE_VAR int v4 = seed + 4, v5 = seed + 5, v6 = seed + 6, v7 = seed + 7;
    VOLATILE_VAR int v8 = seed + 8, v9 = seed + 9, v10 = seed + 10, v11 = seed + 11;
    VOLATILE_VAR int v12 = seed + 12, v13 = seed + 13, v14 = seed + 14, v15 = seed + 15;
    VOLATILE_VAR int v16 = seed + 16, v17 = seed + 17, v18 = seed + 18, v19 = seed + 19;
    VOLATILE_VAR int v20 = seed + 20, v21 = seed + 21, v22 = seed + 22, v23 = seed + 23;
    VOLATILE_VAR int v24 = seed + 24, v25 = seed + 25, v26 = seed + 26, v27 = seed + 27;
    VOLATILE_VAR int v28 = seed + 28, v29 = seed + 29;
    
    VOLATILE_VAR float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f;
    VOLATILE_VAR double d0 = seed * 0.01, d1 = seed * 0.02, d2 = seed * 0.03;
    VOLATILE_VAR long l0 = seed * 1000L, l1 = seed * 2000L, l2 = seed * 3000L;
    
    /* Labels for irreducible loop */
    loop_start:
    for (i = 0; i < iterations; i++) {
        /* Complex arithmetic chains keeping variables live */
        v0 = v1 + v2; v1 = v2 + v3; v2 = v3 + v4; v3 = v4 + v5;
        v4 = v5 + v6; v5 = v6 + v7; v6 = v7 + v8; v7 = v8 + v9;
        v8 = v9 + v10; v9 = v10 + v11; v10 = v11 + v12; v11 = v12 + v13;
        v12 = v13 + v14; v13 = v14 + v15; v14 = v15 + v16; v15 = v16 + v17;
        
        f0 = f1 * 1.1f; f1 = f2 * 1.2f; f2 = f0 * 1.3f;
        d0 = d1 * 1.01; d1 = d2 * 1.02; d2 = d0 * 1.03;
        l0 = l1 >> 1; l1 = l2 << 1; l2 = l0 ^ l1;
        
        /* Force register usage with inline asm */
        asm volatile("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3));
        asm volatile("" : : "r"(v4), "r"(v5), "r"(v6), "r"(v7));
        asm volatile("" : : "r"(f0), "r"(f1), "r"(f2));
        asm volatile("" : : "r"(d0), "r"(d1), "r"(d2));
        
        /* Irreducible control flow with goto jumping into inner loop */
        if ((i & 3) == 0) {
            goto inner_block_a;
        } else if ((i & 3) == 1) {
            goto inner_block_b;
        } else if ((i & 3) == 2) {
            goto inner_block_c;
        } else {
            goto inner_block_d;
        }
        
        /* These labels create irreducible regions */
        inner_block_a:
        v16 = v17 + v18; v17 = v18 + v19; v18 = v19 + v20;
        if ((v16 & 1) == 0) goto loop_middle;
        else goto inner_block_b;
        
        inner_block_b:
        v19 = v20 + v21; v20 = v21 + v22; v21 = v22 + v23;
        if ((v19 & 2) == 0) goto loop_middle;
        else goto inner_block_c;
        
        inner_block_c:
        v22 = v23 + v24; v23 = v24 + v25; v24 = v25 + v26;
        if ((v22 & 4) == 0) goto loop_middle;
        else goto inner_block_d;
        
        inner_block_d:
        v25 = v26 + v27; v26 = v27 + v28; v27 = v28 + v29;
        if ((v25 & 8) == 0) goto loop_middle;
        else goto loop_end;
        
        loop_middle:
        v28 = v29 + v0; v29 = v0 + v1;
        f0 = f1 + f2; f1 = f2 + 1.0f;
        d0 = d1 + d2; d1 = d2 + 1.0;
        goto loop_end;
        
        loop_end:
        /* More arithmetic to keep variables live */
        v0 = v0 ^ v1; v1 = v1 ^ v2; v2 = v2 ^ v3; v3 = v3 ^ v4;
        checksum += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
        checksum += (unsigned long)(f0 + f1 + f2);
        checksum += (unsigned long)(d0 + d1 + d2);
        checksum += l0 + l1 + l2;
    }
    
    goto loop_exit;
    
    loop_exit:
    return checksum;
}

/* Complex switch with goto creating irreducible CFG */
NOINLINE static unsigned long test_switch_goto(int iterations, int seed) {
    VOLATILE_VAR int i;
    VOLATILE_VAR unsigned long checksum = seed;
    
    /* Another set of many variables */
    VOLATILE_VAR int a0 = seed, a1 = seed + 1, a2 = seed + 2, a3 = seed + 3;
    VOLATILE_VAR int a4 = seed + 4, a5 = seed + 5, a6 = seed + 6, a7 = seed + 7;
    VOLATILE_VAR int a8 = seed + 8, a9 = seed + 9, a10 = seed + 10, a11 = seed + 11;
    VOLATILE_VAR int a12 = seed + 12, a13 = seed + 13, a14 = seed + 14, a15 = seed + 15;
    
    VOLATILE_VAR float fa0 = seed * 0.5f, fa1 = seed * 0.6f, fa2 = seed * 0.7f;
    VOLATILE_VAR double da0 = seed * 0.05, da1 = seed * 0.06, da2 = seed * 0.07;
    
    /* Labels for switch goto targets */
    switch_label_a:
    switch_label_b:
    switch_label_c:
    switch_label_d:
    switch_label_e:
    
    for (i = 0; i < iterations; i++) {
        /* Long dependency chains */
        a0 = a1 * a2; a1 = a2 * a3; a2 = a3 * a4; a3 = a4 * a5;
        a4 = a5 * a6; a5 = a6 * a7; a6 = a7 * a8; a7 = a8 * a9;
        a8 = a9 * a10; a9 = a10 * a11; a10 = a11 * a12; a11 = a12 * a13;
        
        fa0 = fa1 * 2.0f; fa1 = fa2 * 3.0f; fa2 = fa0 * 4.0f;
        da0 = da1 * 2.0; da1 = da2 * 3.0; da2 = da0 * 4.0;
        
        /* Force register usage */
        asm volatile("" : : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4));
        asm volatile("" : : "r"(a5), "r"(a6), "r"(a7), "r"(a8), "r"(a9));
        asm volatile("" : : "r"(fa0), "r"(fa1), "r"(fa2));
        
        /* Switch with goto to different labels - creates complex CFG */
        switch (i % 5) {
            case 0:
                a12 = a13 + a14;
                goto switch_label_a;
            case 1:
                a13 = a14 + a15;
                goto switch_label_b;
            case 2:
                a14 = a15 + a0;
                goto switch_label_c;
            case 3:
                a15 = a0 + a1;
                goto switch_label_d;
            case 4:
                a0 = a1 + a2;
                goto switch_label_e;
        }
        
        switch_label_a:
        a1 = a2 + a3;
        fa0 = fa0 + 1.0f;
        goto switch_continue;
        
        switch_label_b:
        a2 = a3 + a4;
        fa1 = fa1 + 2.0f;
        goto switch_continue;
        
        switch_label_c:
        a3 = a4 + a5;
        fa2 = fa2 + 3.0f;
        goto switch_continue;
        
        switch_label_d:
        a4 = a5 + a6;
        da0 = da0 + 1.0;
        goto switch_continue;
        
        switch_label_e:
        a5 = a6 + a7;
        da1 = da1 + 2.0;
        /* Fall through */
        
        switch_continue:
        a6 = a7 + a8; a7 = a8 + a9; a8 = a9 + a10;
        da2 = da2 + 3.0;
        
        checksum += a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9;
        checksum += (unsigned long)(fa0 + fa1 + fa2);
        checksum += (unsigned long)(da0 + da1 + da2);
    }
    
    return checksum;
}

/* Computed goto state machine creating irreducible CFG */
NOINLINE static unsigned long test_computed_goto(int iterations, int seed) {
    VOLATILE_VAR int i, state;
    VOLATILE_VAR unsigned long checksum = seed;
    
    /* Yet another set of variables */
    VOLATILE_VAR int x0 = seed, x1 = seed + 1, x2 = seed + 2, x3 = seed + 3;
    VOLATILE_VAR int x4 = seed + 4, x5 = seed + 5, x6 = seed + 6, x7 = seed + 7;
    VOLATILE_VAR int x8 = seed + 8, x9 = seed + 9, x10 = seed + 10, x11 = seed + 11;
    VOLATILE_VAR int x12 = seed + 12, x13 = seed + 13, x14 = seed + 14, x15 = seed + 15;
    
    VOLATILE_VAR float fx0 = seed * 0.8f, fx1 = seed * 0.9f;
    VOLATILE_VAR double dx0 = seed * 0.08, dx1 = seed * 0.09;
    VOLATILE_VAR long lx0 = seed * 4000L, lx1 = seed * 5000L;
    
    /* Labels for computed goto */
    static void* labels[] = {
        &&state_a, &&state_b, &&state_c, 
        &&state_d, &&state_e, &&state_f
    };
    
    state = 0;
    
    for (i = 0; i < iterations; i++) {
        /* Arithmetic chains */
        x0 = x1 * x2; x1 = x2 * x3; x2 = x3 * x4; x3 = x4 * x5;
        x4 = x5 * x6; x5 = x6 * x7; x6 = x7 * x8; x7 = x8 * x9;
        x8 = x9 * x10; x9 = x10 * x11; x10 = x11 * x12; x11 = x12 * x13;
        
        fx0 = fx1 * 1.5f; fx1 = fx0 * 1.6f;
        dx0 = dx1 * 1.05; dx1 = dx0 * 1.06;
        lx0 = lx1 ^ (lx0 << 2); lx1 = lx0 ^ (lx1 >> 2);
        
        /* Force register usage */
        asm volatile("" : : "r"(x0), "r"(x1), "r"(x2), "r"(x3), "r"(x4));
        asm volatile("" : : "r"(x5), "r"(x6), "r"(x7), "r"(x8), "r"(x9));
        asm volatile("" : : "r"(fx0), "r"(fx1), "r"(dx0), "r"(dx1));
        
        /* Computed goto - creates very complex CFG */
        goto *labels[state];
        
        state_a:
        x12 = x13 + x14; x13 = x14 + x15; x14 = x15 + x0;
        fx0 = fx0 + 0.1f;
        state = (state + 1) % 6;
        goto state_continue;
        
        state_b:
        x15 = x0 + x1; x0 = x1 + x2; x1 = x2 + x3;
        fx1 = fx1 + 0.2f;
        state = (state + 2) % 6;
        goto state_continue;
        
        state_c:
        x2 = x3 + x4; x3 = x4 + x5; x4 = x5 + x6;
        dx0 = dx0 + 0.01;
        state = (state + 3) % 6;
        goto state_continue;
        
        state_d:
        x5 = x6 + x7; x6 = x7 + x8; x7 = x8 + x9;
        dx1 = dx1 + 0.02;
        state = (state + 4) % 6;
        goto state_continue;
        
        state_e:
        x8 = x9 + x10; x9 = x10 + x11; x10 = x11 + x12;
        lx0 = lx0 + 100;
        state = (state + 5) % 6;
        goto state_continue;
        
        state_f:
        x11 = x12 + x13; x12 = x13 + x14; x13 = x14 + x15;
        lx1 = lx1 + 200;
        state = (state + 1) % 6;
        /* Fall through */
        
        state_continue:
        x14 = x15 + x0; x15 = x0 + x1;
        
        checksum += x0 + x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9;
        checksum += x10 + x11 + x12 + x13 + x14 + x15;
        checksum += (unsigned long)(fx0 + fx1);
        checksum += (unsigned long)(dx0 + dx1);
        checksum += lx0 + lx1;
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    int iterations = 10000;
    unsigned long total_checksum = 0;
    int seed = 42;
    
    /* Parse command line for iteration count */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 10000;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Running MCF coverage test with %d iterations, seed=%d\n", 
           iterations, seed);
    
    /* Call all test functions to create different CFG patterns */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    /* Additional calls with different parameters */
    total_checksum += test_irreducible_goto(iterations / 2, seed + 3);
    total_checksum += test_switch_goto(iterations / 2, seed + 4);
    
    printf("Final checksum: %lu\n", total_checksum);
    
    /* Use checksum to prevent dead code elimination */
    if (total_checksum == 0) {
        printf("This should never happen\n");
    }
    
    return 0;
}

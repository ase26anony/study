/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow algorithm
   with fixup graph creation and debug output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* Complex irreducible control flow with goto */
NOINLINE static uint64_t test_irreducible_goto(int iterations, int seed) {
    VOLATILE_VAR int i = seed;
    uint64_t checksum = 0;
    
    /* Many local variables to create register pressure */
    VOLATILE_VAR int v0 = seed * 1, v1 = seed * 2, v2 = seed * 3, v3 = seed * 4;
    VOLATILE_VAR int v4 = seed * 5, v5 = seed * 6, v6 = seed * 7, v7 = seed * 8;
    VOLATILE_VAR int v8 = seed * 9, v9 = seed * 10, v10 = seed * 11, v11 = seed * 12;
    VOLATILE_VAR int v12 = seed * 13, v13 = seed * 14, v14 = seed * 15, v15 = seed * 16;
    VOLATILE_VAR int v16 = seed * 17, v17 = seed * 18, v18 = seed * 19, v19 = seed * 20;
    VOLATILE_VAR int v20 = seed * 21, v21 = seed * 22, v22 = seed * 23, v23 = seed * 24;
    VOLATILE_VAR int v24 = seed * 25, v25 = seed * 26, v26 = seed * 27, v27 = seed * 28;
    VOLATILE_VAR int v28 = seed * 29, v29 = seed * 30;
    
    /* Labels for irreducible loop */
    loop_start:
    if (i >= iterations) goto loop_end;
    
    /* Complex arithmetic to keep variables live */
    v0 = v1 + v2; v1 = v3 + v4; v2 = v5 + v6; v3 = v7 + v8;
    v4 = v9 + v10; v5 = v11 + v12; v6 = v13 + v14; v7 = v15 + v16;
    v8 = v17 + v18; v9 = v19 + v20; v10 = v21 + v22; v11 = v23 + v24;
    v12 = v25 + v26; v13 = v27 + v28; v14 = v29 + v0; v15 = v1 + v2;
    
    /* Irreducible goto pattern - jumping into middle of loop */
    if ((i & 3) == 0) goto middle_block;
    
    inner_loop:
    v16 = v3 + v4; v17 = v5 + v6; v18 = v7 + v8; v19 = v9 + v10;
    v20 = v11 + v12; v21 = v13 + v14; v22 = v15 + v16; v23 = v17 + v18;
    
    if ((i & 1) == 0) goto loop_start;  /* Jump to outer loop start */
    
    middle_block:
    v24 = v19 + v20; v25 = v21 + v22; v26 = v23 + v24; v27 = v25 + v26;
    v28 = v27 + v28; v29 = v29 + v0;
    
    if ((i & 2) == 0) goto inner_loop;  /* Jump to inner loop */
    
    /* More arithmetic */
    v0 = v0 ^ v1; v1 = v1 ^ v2; v2 = v2 ^ v3; v3 = v3 ^ v4;
    v4 = v4 ^ v5; v5 = v5 ^ v6; v6 = v6 ^ v7; v7 = v7 ^ v8;
    
    i++;
    goto loop_start;
    
    loop_end:
    
    /* Aggregate checksum from all variables */
    checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
               v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
               v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
    
    /* Use inline assembly to prevent optimization */
    asm volatile("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4));
    asm volatile("" : : "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9));
    
    return checksum;
}

/* Switch with goto creating complex CFG */
NOINLINE static uint64_t test_switch_goto(int iterations, int seed) {
    VOLATILE_VAR int i = 0;
    uint64_t checksum = seed;
    
    /* Many floating point variables for different register classes */
    VOLATILE_VAR float f0 = seed * 1.1f, f1 = seed * 2.2f, f2 = seed * 3.3f;
    VOLATILE_VAR float f3 = seed * 4.4f, f4 = seed * 5.5f, f5 = seed * 6.6f;
    VOLATILE_VAR double d0 = seed * 1.11, d1 = seed * 2.22, d2 = seed * 3.33;
    VOLATILE_VAR double d3 = seed * 4.44, d4 = seed * 5.55, d5 = seed * 6.66;
    VOLATILE_VAR long l0 = seed * 7, l1 = seed * 8, l2 = seed * 9, l3 = seed * 10;
    
    /* Integer variables */
    VOLATILE_VAR int v0 = seed, v1 = seed + 1, v2 = seed + 2, v3 = seed + 3;
    VOLATILE_VAR int v4 = seed + 4, v5 = seed + 5, v6 = seed + 6, v7 = seed + 7;
    VOLATILE_VAR int v8 = seed + 8, v9 = seed + 9, v10 = seed + 10, v11 = seed + 11;
    
    for (i = 0; i < iterations; i++) {
        /* Switch with goto to different labels */
        switch ((i + seed) % 7) {
            case 0:
                f0 = f1 + f2; f1 = f3 + f4; f2 = f5 + f0;
                d0 = d1 + d2; d1 = d3 + d4; d2 = d5 + d0;
                goto label_a;
            case 1:
                v0 = v1 + v2; v1 = v3 + v4; v2 = v5 + v6;
                goto label_b;
            case 2:
                f3 = f4 + f5; f4 = f0 + f1; f5 = f2 + f3;
                goto label_c;
            case 3:
                d3 = d4 + d5; d4 = d0 + d1; d5 = d2 + d3;
                goto label_a;
            case 4:
                v3 = v4 + v5; v4 = v6 + v7; v5 = v8 + v9;
                goto label_d;
            case 5:
                l0 = l1 + l2; l1 = l3 + l0; l2 = l1 + l2;
                goto label_b;
            default:
                v6 = v7 + v8; v7 = v9 + v10; v8 = v11 + v0;
                goto label_c;
        }
        
        label_a:
        v9 = v10 + v11; v10 = v0 + v1; v11 = v2 + v3;
        f0 = f0 * 1.01f; f1 = f1 * 1.02f;
        continue;
        
        label_b:
        v0 = v1 * 2; v1 = v2 * 3; v2 = v3 * 4;
        d0 = d0 * 1.01; d1 = d1 * 1.02;
        continue;
        
        label_c:
        v3 = v4 / 2; v4 = v5 / 3; v5 = v6 / 4;
        l0 = l1 ^ l2; l1 = l3 ^ l0;
        continue;
        
        label_d:
        v6 = v7 << 1; v7 = v8 << 2; v8 = v9 << 3;
        f2 = f3 * f4; f3 = f5 * f0;
        /* Fall through to continue */
    }
    
    /* Mix all values into checksum */
    checksum += (uint64_t)(f0 * 100) + (uint64_t)(f1 * 100) + (uint64_t)(f2 * 100);
    checksum += (uint64_t)(d0 * 100) + (uint64_t)(d1 * 100) + (uint64_t)(d2 * 100);
    checksum += l0 + l1 + l2 + l3;
    checksum += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11;
    
    /* Prevent optimization */
    asm volatile("" : : "r"(f0), "r"(d0), "r"(l0), "r"(v0));
    
    return checksum;
}

/* Computed goto for state machine */
NOINLINE static uint64_t test_computed_goto(int iterations, int seed) {
    static void* labels[] = { &&state0, &&state1, &&state2, &&state3, 
                             &&state4, &&state5, &&state6, &&state7 };
    
    VOLATILE_VAR int state = seed % 8;
    VOLATILE_VAR int i = 0;
    uint64_t checksum = 0;
    
    /* Many variables of different types */
    VOLATILE_VAR int a0 = 1, a1 = 2, a2 = 3, a3 = 4, a4 = 5, a5 = 6, a6 = 7, a7 = 8;
    VOLATILE_VAR int b0 = 9, b1 = 10, b2 = 11, b3 = 12, b4 = 13, b5 = 14, b6 = 15, b7 = 16;
    VOLATILE_VAR int c0 = 17, c1 = 18, c2 = 19, c3 = 20, c4 = 21, c5 = 22, c6 = 23, c7 = 24;
    VOLATILE_VAR int d0 = 25, d1 = 26, d2 = 27, d3 = 28, d4 = 29, d5 = 30, d6 = 31, d7 = 32;
    
    for (i = 0; i < iterations; i++) {
        /* Computed goto */
        goto *labels[state];
        
        state0:
        a0 = a1 + a2; a1 = a3 + a4; a2 = a5 + a6; a3 = a7 + a0;
        state = (state + 1) % 8;
        continue;
        
        state1:
        b0 = b1 * b2; b1 = b3 * b4; b2 = b5 * b6; b3 = b7 * b0;
        state = (state + 3) % 8;
        continue;
        
        state2:
        c0 = c1 - c2; c1 = c3 - c4; c2 = c5 - c6; c3 = c7 - c0;
        state = (state + 5) % 8;
        continue;
        
        state3:
        d0 = d1 ^ d2; d1 = d3 ^ d4; d2 = d5 ^ d6; d3 = d7 ^ d0;
        state = (state + 7) % 8;
        continue;
        
        state4:
        a4 = a5 | a6; a5 = a7 | a0; a6 = a1 | a2; a7 = a3 | a4;
        state = (state + 2) % 8;
        continue;
        
        state5:
        b4 = b5 & b6; b5 = b7 & b0; b6 = b1 & b2; b7 = b3 & b4;
        state = (state + 4) % 8;
        continue;
        
        state6:
        c4 = c5 << 1; c5 = c6 << 2; c6 = c7 << 3; c7 = c0 << 4;
        state = (state + 6) % 8;
        continue;
        
        state7:
        d4 = d5 >> 1; d5 = d6 >> 2; d6 = d7 >> 3; d7 = d0 >> 4;
        state = (state * 2 + 1) % 8;
        continue;
    }
    
    /* Aggregate results */
    checksum = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 +
               b0 + b1 + b2 + b3 + b4 + b5 + b6 + b7 +
               c0 + c1 + c2 + c3 + c4 + c5 + c6 + c7 +
               d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7;
    
    asm volatile("" : : "r"(a0), "r"(b0), "r"(c0), "r"(d0));
    
    return checksum;
}

/* Main driver */
int main(int argc, char** argv) {
    int iterations = 10000;
    int seed = 42;
    uint64_t total_checksum = 0;
    
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
    
    /* Run all test patterns */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations, seed + 2);
    
    /* Also call recursively to create more complex call graph */
    if (iterations > 1000) {
        total_checksum += test_irreducible_goto(iterations / 10, seed + 3);
    }
    
    printf("Final checksum: %lu\n", (unsigned long)total_checksum);
    
    /* Use result to prevent dead code elimination */
    if (total_checksum == 0) {
        printf("Unexpected zero checksum\n");
    }
    
    return (int)(total_checksum % 256);
}

/* mcf_coverage.c - Program to trigger MCF fixup graph debug output */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline))
#define KEEP_ALIVE(var) asm volatile("" : : "r"(var))

/* Global volatile to prevent dead code elimination */
volatile int global_result = 0;

/* Function 1: Irreducible loops with goto jumping across boundaries */
NOINLINE static unsigned long test_irreducible_goto(int iterations, int seed) {
    /* Declare many variables to increase register pressure */
    volatile int v0 = seed + 1, v1 = seed + 2, v2 = seed + 3, v3 = seed + 4;
    volatile int v4 = seed + 5, v5 = seed + 6, v6 = seed + 7, v7 = seed + 8;
    volatile int v8 = seed + 9, v9 = seed + 10, v10 = seed + 11, v11 = seed + 12;
    volatile int v12 = seed + 13, v13 = seed + 14, v14 = seed + 15, v15 = seed + 16;
    volatile int v16 = seed + 17, v17 = seed + 18, v18 = seed + 19, v19 = seed + 20;
    volatile int v20 = seed + 21, v21 = seed + 22, v22 = seed + 23, v23 = seed + 24;
    volatile int v24 = seed + 25, v25 = seed + 26, v26 = seed + 27, v27 = seed + 28;
    volatile int v28 = seed + 29, v29 = seed + 30;
    
    volatile float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f;
    volatile double d0 = seed * 0.01, d1 = seed * 0.02, d2 = seed * 0.03;
    volatile long l0 = seed * 100L, l1 = seed * 200L, l2 = seed * 300L;
    
    unsigned long checksum = 0;
    int i;
    
    /* Outer loop */
    for (i = 0; i < iterations; i++) {
        /* Create irreducible region with goto jumping into inner loop */
        if (v0 % 7 == 0) {
            goto inner_loop_start;
        }
        
        /* Normal path */
        v0 = v1 + v2 * v3 - v4 / (v5 + 1);
        v1 = v6 ^ v7 | v8 & v9;
        v2 = v10 << (v11 % 8) >> (v12 % 8);
        v3 = v13 - v14 + v15 * v16;
        
        /* Complex floating point operations */
        f0 = f1 * f2 + (float)v0;
        f1 = f0 / (f2 + 1.0f) - (float)v1;
        d0 = d1 * d2 + (double)v2;
        d1 = d0 / (d2 + 1.0) - (double)v3;
        l0 = l1 + l2 * (long)v4;
        l1 = l0 - l2 / ((long)v5 + 1);
        
    inner_loop_start:
        /* Inner loop that can be entered from multiple points */
        for (int j = 0; j < 5; j++) {
            v4 = v17 + v18 - v19 * v20;
            v5 = v21 | v22 ^ v23 & v24;
            v6 = v25 << (v26 % 8);
            v7 = v27 - v28 + v29;
            
            f2 = f0 * f1 + (float)v4;
            d2 = d0 * d1 + (double)v5;
            l2 = l0 + l1 * (long)v6;
            
            if (v8 % 11 == j) {
                /* Jump out to outer_label, creating irreducible flow */
                goto outer_label;
            }
            
            v8 = v9 + v10 - v11 * v12;
            v9 = v13 | v14 ^ v15 & v16;
        }
        
        /* Continue normal flow */
        v10 = v17 + v18 - v19 * v20;
        v11 = v21 | v22 ^ v23 & v24;
        
    outer_label:
        /* Target of goto from inner loop */
        v12 = v25 + v26 - v27 * v28;
        v13 = v29 | v0 ^ v1 & v2;
        
        /* More arithmetic to keep variables live */
        v14 = v3 + v4 - v5 * v6;
        v15 = v7 | v8 ^ v9 & v10;
        v16 = v11 + v12 - v13 * v14;
        v17 = v15 | v16 ^ v17 & v18;
        v18 = v19 + v20 - v21 * v22;
        v19 = v23 | v24 ^ v25 & v26;
        v20 = v27 + v28 - v29 * v0;
        
        /* Update checksum with all variables */
        checksum += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
        checksum += v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
        checksum += v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
        checksum += (unsigned long)f0 + (unsigned long)f1 + (unsigned long)f2;
        checksum += (unsigned long)d0 + (unsigned long)d1 + (unsigned long)d2;
        checksum += l0 + l1 + l2;
        
        /* Modify conditions for next iteration */
        v0 = (v0 * 1103515245 + 12345) & 0x7fffffff;
        KEEP_ALIVE(v0);
    }
    
    return checksum;
}

/* Function 2: Switch with goto creating complex CFG */
NOINLINE static unsigned long test_switch_goto(int iterations, int seed) {
    volatile int v0 = seed, v1 = seed + 1, v2 = seed + 2, v3 = seed + 3;
    volatile int v4 = seed + 4, v5 = seed + 5, v6 = seed + 6, v7 = seed + 7;
    volatile int v8 = seed + 8, v9 = seed + 9, v10 = seed + 10, v11 = seed + 11;
    volatile int v12 = seed + 12, v13 = seed + 13, v14 = seed + 14, v15 = seed + 15;
    volatile int v16 = seed + 16, v17 = seed + 17, v18 = seed + 18, v19 = seed + 19;
    
    volatile float f0 = seed * 1.1f, f1 = seed * 1.2f, f2 = seed * 1.3f, f3 = seed * 1.4f;
    volatile double d0 = seed * 0.11, d1 = seed * 0.12, d2 = seed * 0.13, d3 = seed * 0.14;
    
    unsigned long checksum = 0;
    int state = seed % 5;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex switch with goto to different labels */
        switch (state) {
            case 0:
                v0 = v1 + v2 * v3 - v4;
                v1 = v5 ^ v6 | v7 & v8;
                if (v0 % 3 == 0) goto label_a;
                else goto label_b;
                
            case 1:
                v2 = v9 + v10 - v11 * v12;
                v3 = v13 | v14 ^ v15 & v16;
                if (v2 % 5 == 0) goto label_c;
                else goto label_d;
                
            case 2:
                v4 = v17 + v18 - v19 * v0;
                v5 = v1 | v2 ^ v3 & v4;
                goto label_a;
                
            case 3:
                v6 = v5 + v6 - v7 * v8;
                v7 = v9 | v10 ^ v11 & v12;
                goto label_b;
                
            case 4:
                v8 = v13 + v14 - v15 * v16;
                v9 = v17 | v18 ^ v19 & v0;
                goto label_c;
                
            default:
                v10 = v1 + v2 - v3 * v4;
                goto label_d;
        }
        
    label_a:
        f0 = f1 * f2 + (float)v0;
        f1 = f0 / (f2 + 1.0f) - (float)v1;
        v11 = v12 + v13 - v14 * v15;
        if (v11 % 7 == 0) goto label_d;
        
    label_b:
        f2 = f3 * f0 + (float)v2;
        f3 = f2 / (f0 + 1.0f) - (float)v3;
        v12 = v16 + v17 - v18 * v19;
        if (v12 % 11 == 0) goto label_a;
        
    label_c:
        d0 = d1 * d2 + (double)v4;
        d1 = d0 / (d2 + 1.0) - (double)v5;
        v13 = v0 + v1 - v2 * v3;
        if (v13 % 13 == 0) goto label_b;
        
    label_d:
        d2 = d3 * d0 + (double)v6;
        d3 = d2 / (d0 + 1.0) - (double)v7;
        v14 = v4 + v5 - v6 * v7;
        
        /* Long dependency chain */
        v15 = v8 + v9 - v10 * v11;
        v16 = v12 + v13 - v14 * v15;
        v17 = v16 + v17 - v18 * v19;
        v18 = v0 + v1 - v2 * v3;
        v19 = v4 + v5 - v6 * v7;
        
        /* Update checksum */
        checksum += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
        checksum += v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
        checksum += (unsigned long)f0 + (unsigned long)f1 + (unsigned long)f2 + (unsigned long)f3;
        checksum += (unsigned long)d0 + (unsigned long)d1 + (unsigned long)d2 + (unsigned long)d3;
        
        /* Change state for next iteration */
        state = (state * 31 + i) % 5;
        KEEP_ALIVE(state);
    }
    
    return checksum;
}

/* Function 3: Computed goto simulating state machine */
NOINLINE static unsigned long test_computed_goto(int iterations, int seed) {
    /* Declare labels for computed goto */
    static void* labels[] = {
        &&state0, &&state1, &&state2, &&state3, &&state4,
        &&state5, &&state6, &&state7, &&state8, &&state9
    };
    
    /* Many variables for register pressure */
    volatile int vars[MANY_VARS];
    volatile float floats[10];
    volatile double doubles[10];
    
    /* Initialize variables */
    for (int i = 0; i < MANY_VARS; i++) {
        vars[i] = seed + i * 3;
    }
    for (int i = 0; i < 10; i++) {
        floats[i] = seed * (0.1f * i + 0.1f);
        doubles[i] = seed * (0.01 * i + 0.01);
    }
    
    unsigned long checksum = 0;
    int state = seed % 10;
    
    for (int i = 0; i < iterations; i++) {
        /* Computed goto - creates very complex CFG */
        goto *labels[state];
        
    state0:
        vars[0] = vars[1] + vars[2] * vars[3] - vars[4];
        vars[1] = vars[5] ^ vars[6] | vars[7] & vars[8];
        floats[0] = floats[1] * floats[2] + (float)vars[0];
        state = (vars[0] % 9) + 1;
        goto end_state;
        
    state1:
        vars[2] = vars[9] + vars[10] - vars[11] * vars[12];
        vars[3] = vars[13] | vars[14] ^ vars[15] & vars[16];
        floats[1] = floats[2] * floats[3] + (float)vars[2];
        state = (vars[2] % 9) + 2;
        goto end_state;
        
    state2:
        vars[4] = vars[17] + vars[18] - vars[19] * vars[0];
        vars[5] = vars[1] | vars[2] ^ vars[3] & vars[4];
        floats[2] = floats[3] * floats[4] + (float)vars[4];
        state = (vars[4] % 9) + 3;
        goto end_state;
        
    state3:
        vars[6] = vars[5] + vars[6] - vars[7] * vars[8];
        vars[7] = vars[9] | vars[10] ^ vars[11] & vars[12];
        doubles[0] = doubles[1] * doubles[2] + (double)vars[6];
        state = (vars[6] % 9) + 4;
        goto end_state;
        
    state4:
        vars[8] = vars[13] + vars[14] - vars[15] * vars[16];
        vars[9] = vars[17] | vars[18] ^ vars[19] & vars[0];
        doubles[1] = doubles[2] * doubles[3] + (double)vars[8];
        state = (vars[8] % 9) + 5;
        goto end_state;
        
    state5:
        vars[10] = vars[1] + vars[2] - vars[3] * vars[4];
        vars[11] = vars[5] | vars[6] ^ vars[7] & vars[8];
        doubles[2] = doubles[3] * doubles[4] + (double)vars[10];
        state = (vars[10] % 9) + 6;
        goto end_state;
        
    state6:
        vars[12] = vars[9] + vars[10] - vars[11] * vars[12];
        vars[13] = vars[13] | vars[14] ^ vars[15] & vars[16];
        floats[3] = floats[4] * floats[5] + (float)vars[12];
        state = (vars[12] % 9) + 7;
        goto end_state;
        
    state7:
        vars[14] = vars[17] + vars[18] - vars[19] * vars[0];
        vars[15] = vars[1] | vars[2] ^ vars[3] & vars[4];
        floats[4] = floats[5] * floats[6] + (float)vars[14];
        state = (vars[14] % 9) + 8;
        goto end_state;
        
    state8:
        vars[16] = vars[5] + vars[6] - vars[7] * vars[8];
        vars[17] = vars[9] | vars[10] ^ vars[11] & vars[12];
        doubles[3] = doubles[4] * doubles[5] + (double)vars[16];
        state = (vars[16] % 9) + 9;
        goto end_state;
        
    state9:
        vars[18] = vars[13] + vars[14] - vars[15] * vars[16];
        vars[19] = vars[17] | vars[18] ^ vars[19] & vars[0];
        doubles[4] = doubles[5] * doubles[6] + (double)vars[18];
        state = vars[18] % 9;
        goto end_state;
        
    end_state:
        /* Complex arithmetic operations on all variables */
        for (int j = 0; j < MANY_VARS - 1; j++) {
            vars[j] = vars[j] + vars[j + 1] - vars[(j + 2) % MANY_VARS] * 
                     ((vars[(j + 3) % MANY_VARS] & 0xFF) + 1);
        }
        
        for (int j = 0; j < 9; j++) {
            floats[j] = floats[j] * floats[(j + 1) % 10] + 
                       (float)vars[j % MANY_VARS];
            doubles[j] = doubles[j] * doubles[(j + 1) % 10] + 
                        (double)vars[(j + 5) % MANY_VARS];
        }
        
        /* Update checksum */
        for (int j = 0; j < MANY_VARS; j++) {
            checksum += vars[j];
        }
        for (int j = 0; j < 10; j++) {
            checksum += (unsigned long)floats[j] + (unsigned long)doubles[j];
        }
        
        KEEP_ALIVE(state);
    }
    
    return checksum;
}

/* Main function that runs all tests */
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
    
    printf("Running MCF coverage test with iterations=%d, seed=%d\n", 
           iterations, seed);
    
    /* Run all test functions */
    unsigned long result = 0;
    
    result += test_irreducible_goto(iterations / 10, seed);
    printf("  test_irreducible_goto completed\n");
    
    result += test_switch_goto(iterations / 10, seed + 1);
    printf("  test_switch_goto completed\n");
    
    result += test_computed_goto(iterations / 10, seed + 2);
    printf("  test_computed_goto completed\n");
    
    /* Store result in global to prevent optimization */
    global_result = (int)(result & 0xFFFFFFFF);
    
    printf("Final checksum: %lu (truncated to global: %d)\n", 
           result, global_result);
    
    return global_result == 0 ? 0 : 1;
}

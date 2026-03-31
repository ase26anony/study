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

/* Function 1: Irreducible loop with goto jumping across loop boundaries */
NOINLINE static unsigned long test_irreducible_goto(int iterations, int seed) {
    /* Declare many local variables to increase register pressure */
    volatile int v0 = seed + 1, v1 = seed + 2, v2 = seed + 3, v3 = seed + 4;
    volatile int v4 = seed + 5, v5 = seed + 6, v6 = seed + 7, v7 = seed + 8;
    volatile int v8 = seed + 9, v9 = seed + 10, v10 = seed + 11, v11 = seed + 12;
    volatile float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f, f3 = seed * 0.4f;
    volatile double d0 = seed * 0.01, d1 = seed * 0.02, d2 = seed * 0.03, d3 = seed * 0.04;
    volatile long l0 = seed * 100L, l1 = seed * 200L, l2 = seed * 300L, l3 = seed * 400L;
    volatile int v12 = seed + 13, v13 = seed + 14, v14 = seed + 15, v15 = seed + 16;
    volatile int v16 = seed + 17, v17 = seed + 18, v18 = seed + 19, v19 = seed + 20;
    
    unsigned long checksum = 0;
    int i = 0;
    
    /* Label definitions for irreducible control flow */
    outer_loop_start:
    if (i >= iterations) goto end_func1;
    
    /* Complex arithmetic to create register pressure */
    v0 = v1 + v2 * v3 - v4 / (v5 + 1);
    v1 = v6 ^ v7 | v8 & v9;
    v2 = v10 << (v11 & 3) >> (v12 % 4);
    f0 = f1 * f2 + f3 / (f0 + 0.001f);
    d0 = d1 - d2 * d3 + (d0 * 0.5);
    l0 = l1 + l2 - l3 * (l0 % 100);
    
    /* Irreducible loop structure with goto jumping into inner loop */
    if (v0 % 7 == 0) {
        goto inner_loop_middle;
    }
    
    inner_loop_start:
    for (int j = 0; j < 5; j++) {
        v3 = v4 + v5 * j;
        v4 = v6 - v7 / (j + 1);
        f1 = f2 * j + f3;
        d1 = d2 - d3 * j;
        
        if ((v3 + j) % 11 == 0) {
            goto outer_loop_middle;  /* Jump out to outer loop */
        }
    }
    goto after_inner;
    
    inner_loop_middle:
    v5 = v6 * v7 + v8;
    v6 = v9 ^ v10 | v11;
    f2 = f3 * 2.0f - f0;
    d2 = d3 / 2.0 + d1;
    goto inner_loop_start;  /* Jump back to loop start */
    
    outer_loop_middle:
    v7 = v8 + v9 * v10;
    v8 = v11 & v12 | v13;
    f3 = f0 + f1 - f2;
    d3 = d0 * d1 + d2;
    
    after_inner:
    /* More arithmetic operations */
    v9 = v10 << 2 | v11 >> 3;
    v10 = v12 + v13 - v14 * v15;
    v11 = v16 ^ v17 & v18 | v19;
    v12 = v13 * v14 / (v15 + 1);
    
    /* Update checksum with all variables */
    checksum += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    checksum += v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
    checksum += (int)f0 + (int)f1 + (int)f2 + (int)f3;
    checksum += (int)d0 + (int)d1 + (int)d2 + (int)d3;
    checksum += l0 % 1000 + l1 % 1000 + l2 % 1000 + l3 % 1000;
    
    i++;
    if (i % 2 == 0) {
        goto outer_loop_start;
    } else {
        goto outer_loop_middle;
    }
    
    end_func1:
    KEEP_ALIVE(checksum);
    return checksum;
}

/* Function 2: Switch with goto creating complex CFG */
NOINLINE static unsigned long test_switch_goto(int iterations, int seed) {
    volatile int v0 = seed, v1 = seed * 2, v2 = seed * 3, v3 = seed * 4;
    volatile int v4 = seed * 5, v5 = seed * 6, v6 = seed * 7, v7 = seed * 8;
    volatile int v8 = seed * 9, v9 = seed * 10, v10 = seed * 11, v11 = seed * 12;
    volatile float f0 = seed * 0.5f, f1 = seed * 1.5f, f2 = seed * 2.5f, f3 = seed * 3.5f;
    volatile double d0 = seed * 0.05, d1 = seed * 0.15, d2 = seed * 0.25, d3 = seed * 0.35;
    volatile long l0 = seed * 50L, l1 = seed * 150L, l2 = seed * 250L, l3 = seed * 350L;
    volatile int v12 = seed * 13, v13 = seed * 14, v14 = seed * 15, v15 = seed * 16;
    
    unsigned long checksum = 0;
    int state = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Heavy arithmetic before switch */
        v0 = v1 + v2 * v3 - i;
        v1 = v4 ^ v5 | v6 & v7;
        v2 = v8 << (v9 % 4) >> (v10 & 3);
        f0 = f1 * f2 + f3 / (i + 1.0f);
        d0 = d1 - d2 * d3 + (i * 0.01);
        l0 = l1 + l2 - l3 * (i % 10);
        
        /* Complex switch with goto to different labels */
        switch (state % 6) {
            case 0:
                v3 = v4 + v5;
                v4 = v6 * v7;
                if (v3 % 3 == 0) goto case_2_label;
                break;
            case 1:
                v5 = v8 - v9;
                v6 = v10 / (v11 + 1);
                goto case_4_label;  /* Jump to another case's code */
            case 2:
            case_2_label:
                v7 = v12 ^ v13;
                v8 = v14 | v15;
                f1 = f2 * 3.0f;
                if (v7 % 5 == 0) goto case_5_label;
                break;
            case 3:
                v9 = v0 << 1;
                v10 = v1 >> 2;
                goto after_switch;  /* Jump out of switch */
            case 4:
            case_4_label:
                v11 = v2 + v3 * v4;
                v12 = v5 & v6 | v7;
                f2 = f3 - f0 * 0.5f;
                break;
            case 5:
            case_5_label:
                v13 = v8 ^ v9 & v10;
                v14 = v11 * v12;
                d1 = d2 + d3 / 2.0;
                goto case_0_label;
            case_0_label:
                v15 = v0 + v1 + v2;
                f3 = f0 + f1 + f2;
                break;
        }
        
        after_switch:
        /* More operations after switch */
        d2 = d0 * d1 + d3;
        d3 = d1 - d2 * 0.75;
        l1 = l0 + l2 - l3;
        l2 = l1 * 2 + l3;
        
        /* Update checksum */
        checksum += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
        checksum += v9 + v10 + v11 + v12 + v13 + v14 + v15;
        checksum += (int)(f0 * 10) + (int)(f1 * 10) + (int)(f2 * 10) + (int)(f3 * 10);
        checksum += (int)(d0 * 100) + (int)(d1 * 100) + (int)(d2 * 100) + (int)(d3 * 100);
        checksum += l0 % 10000 + l1 % 10000 + l2 % 10000 + l3 % 10000;
        
        state = (state * 1103515245 + 12345) & 0x7fffffff;
    }
    
    KEEP_ALIVE(checksum);
    return checksum;
}

/* Function 3: Computed goto simulating state machine */
NOINLINE static unsigned long test_computed_goto(int iterations, int seed) {
    volatile int v0 = seed, v1 = seed + 100, v2 = seed + 200, v3 = seed + 300;
    volatile int v4 = seed + 400, v5 = seed + 500, v6 = seed + 600, v7 = seed + 700;
    volatile int v8 = seed + 800, v9 = seed + 900, v10 = seed + 1000, v11 = seed + 1100;
    volatile float f0 = seed * 0.01f, f1 = seed * 0.02f, f2 = seed * 0.03f, f3 = seed * 0.04f;
    volatile double d0 = seed * 0.001, d1 = seed * 0.002, d2 = seed * 0.003, d3 = seed * 0.004;
    volatile long l0 = seed * 1000L, l1 = seed * 2000L, l2 = seed * 3000L, l3 = seed * 4000L;
    volatile int v12 = seed + 1200, v13 = seed + 1300, v14 = seed + 1400, v15 = seed + 1500;
    volatile int v16 = seed + 1600, v17 = seed + 1700, v18 = seed + 1800, v19 = seed + 1900;
    
    /* Labels for computed goto */
    static void* labels[] = {
        &&state_a, &&state_b, &&state_c, 
        &&state_d, &&state_e, &&state_f,
        &&state_g, &&state_h
    };
    
    unsigned long checksum = 0;
    int state = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Register pressure operations */
        v0 = v1 + v2 * i - v3;
        v1 = v4 ^ v5 | v6 & v7;
        v2 = v8 << (v9 % 4);
        f0 = f1 * f2 + f3 / (i * 0.1f + 1.0f);
        d0 = d1 - d2 * d3;
        l0 = l1 + l2 - l3 * (i % 7);
        
        /* Computed goto - creates very complex CFG */
        goto *labels[state % 8];
        
        state_a:
        v3 = v10 + v11 * v12;
        v4 = v13 & v14 | v15;
        f1 = f2 * 2.0f - f0;
        state = (state + 1) % 8;
        goto after_state;
        
        state_b:
        v5 = v16 ^ v17;
        v6 = v18 << 1 | v19 >> 2;
        f2 = f3 + f0 * 0.5f;
        state = (state + 3) % 8;
        goto after_state;
        
        state_c:
        v7 = v0 * v1 + v2;
        v8 = v3 - v4 / (v5 + 1);
        d1 = d2 * d3 + d0;
        state = (state + 5) % 8;
        goto after_state;
        
        state_d:
        v9 = v6 + v7 * v8;
        v10 = v9 ^ v10 & v11;
        d2 = d3 - d0 * 0.25;
        state = (state + 2) % 8;
        goto after_state;
        
        state_e:
        v11 = v12 | v13 & v14;
        v12 = v15 << (v16 % 3);
        f3 = f0 * f1 - f2;
        state = (state + 4) % 8;
        goto after_state;
        
        state_f:
        v13 = v17 + v18 - v19;
        v14 = v0 * v1 / (v2 + 1);
        d3 = d0 + d1 * 0.1;
        state = (state + 6) % 8;
        goto after_state;
        
        state_g:
        v15 = v3 ^ v4 | v5 & v6;
        v16 = v7 << 2 >> 1;
        l1 = l2 + l3 * (i % 5);
        state = (state + 7) % 8;
        goto after_state;
        
        state_h:
        v17 = v8 + v9 * v10;
        v18 = v11 & v12 | v13;
        l2 = l0 - l1 + l3;
        state = (state + 1) % 8;
        /* Fall through */
        
        after_state:
        /* More operations after state machine */
        v19 = v14 + v15 - v16 * v17;
        f0 = f1 + f2 - f3;
        d0 = d1 * 2.0 + d2 - d3;
        l3 = l0 * 2 + l1 - l2;
        
        /* Update checksum */
        checksum += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
        checksum += v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
        checksum += (int)(f0 * 100) + (int)(f1 * 100) + (int)(f2 * 100) + (int)(f3 * 100);
        checksum += (int)(d0 * 1000) + (int)(d1 * 1000) + (int)(d2 * 1000) + (int)(d3 * 1000);
        checksum += l0 % 100000 + l1 % 100000 + l2 % 100000 + l3 % 100000;
    }
    
    KEEP_ALIVE(checksum);
    return checksum;
}

/* Main function */
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
    
    printf("Running MCF coverage test with %d iterations, seed=%d\n", 
           iterations, seed);
    
    /* Call all test functions to trigger different CFG patterns */
    unsigned long result1 = test_irreducible_goto(iterations, seed);
    printf("Test 1 result: %lu\n", result1);
    
    unsigned long result2 = test_switch_goto(iterations, seed + 1);
    printf("Test 2 result: %lu\n", result2);
    
    unsigned long result3 = test_computed_goto(iterations / 2, seed + 2);
    printf("Test 3 result: %lu\n", result3);
    
    /* Aggregate results to prevent dead code elimination */
    global_result = (result1 + result2 + result3) % 1000000;
    printf("Final checksum: %d\n", global_result);
    
    return global_result == 0 ? 0 : 1;
}

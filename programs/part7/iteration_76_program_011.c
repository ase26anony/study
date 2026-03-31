/* mcf_coverage.c - Program to trigger MCF fixup graph debug output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MANY_VARS 30

/* Prevent optimization and inlining */
__attribute__((noinline, noipa))
static uint64_t test_irreducible_goto(int iterations, int seed) {
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
    
    uint64_t checksum = 0;
    int i = 0;
    
    /* Labels for irreducible loop with goto jumps */
    outer_loop:
    if (i >= iterations) goto end_func;
    
    /* Complex arithmetic to keep variables live */
    v0 = v1 + v2 * v3 - v4 / (v5 + 1);
    v1 = v2 + v3 * v4 - v5 / (v6 + 1);
    v2 = v3 + v4 * v5 - v6 / (v7 + 1);
    f0 = f1 * 1.1f + f2 - f3 * 0.9f;
    f1 = f2 * 1.2f + f3 - f0 * 0.8f;
    d0 = d1 * 1.01 + d2 - d3 * 0.99;
    d1 = d2 * 1.02 + d3 - d0 * 0.98;
    l0 = l1 + l2 - l3 * l4 / (l5 + 1);
    l1 = l2 + l3 - l4 * l5 / (l6 + 1);
    
    /* Mark variables as used to prevent optimization */
    asm volatile("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4));
    asm volatile("" : : "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9));
    asm volatile("" : : "r"(f0), "r"(f1), "r"(f2), "r"(f3));
    asm volatile("" : : "r"(d0), "r"(d1), "r"(d2), "r"(d3));
    asm volatile("" : : "r"(l0), "r"(l1), "r"(l2), "r"(l3), "r"(l4));
    asm volatile("" : : "r"(l5), "r"(l6), "r"(l7), "r"(l8), "r"(l9));
    
    /* Irreducible control flow: jump into inner loop */
    if ((i % 17) == 0) goto inner_loop_middle;
    
    inner_loop_start:
    for (int j = 0; j < 5; j++) {
        inner_loop_middle:
        v0 += j;
        v1 -= j;
        /* Jump out to outer label */
        if ((j % 3) == 1 && (i % 13) == 0) goto outer_loop;
        
        inner_loop_end:
        v2 *= (j + 1);
        /* Jump across loop boundaries */
        if ((j % 4) == 2 && (i % 11) == 0) goto after_inner;
    }
    
    after_inner:
    v3 = v4 ^ v5 | v6 & v7;
    
    /* Another irreducible region */
    if ((i % 7) == 0) goto label_a;
    if ((i % 5) == 0) goto label_b;
    
    label_c:
    v4 = v5 + v6 - v7;
    i++;
    goto outer_loop;
    
    label_a:
    v5 = v6 * v7 / (v8 + 1);
    if ((i % 3) == 0) goto label_c;
    goto label_b;
    
    label_b:
    v6 = v7 - v8 + v9;
    if ((i % 2) == 0) goto label_c;
    goto label_a;
    
    end_func:
    checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9
               + (uint64_t)f0 + (uint64_t)f1 + (uint64_t)f2 + (uint64_t)f3
               + (uint64_t)d0 + (uint64_t)d1 + (uint64_t)d2 + (uint64_t)d3
               + l0 + l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9;
    return checksum;
}

__attribute__((noinline, noipa))
static uint64_t test_switch_goto(int iterations, int seed) {
    volatile int vars[MANY_VARS];
    for (int k = 0; k < MANY_VARS; k++) {
        vars[k] = seed + k * 3;
    }
    
    uint64_t checksum = 0;
    int state = seed % 5;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex switch with goto to create irreducible CFG */
        switch (state) {
            case 0:
                vars[0] = vars[1] + vars[2];
                vars[1] = vars[2] * vars[3];
                /* Jump to label outside switch */
                if ((i % 19) == 0) goto switch_exit_label;
                state = 1;
                break;
                
            case 1:
                vars[2] = vars[3] - vars[4];
                vars[3] = vars[4] / (vars[5] + 1);
                /* Another external jump */
                if ((i % 17) == 0) goto switch_middle_label;
                state = 2;
                break;
                
            case 2:
                vars[4] = vars[5] | vars[6];
                vars[5] = vars[6] & vars[7];
                if ((i % 13) == 0) goto switch_start_label;
                state = 3;
                break;
                
            case 3:
                vars[6] = vars[7] ^ vars[8];
                vars[7] = vars[8] + vars[9];
                if ((i % 11) == 0) goto switch_end_label;
                state = 4;
                break;
                
            case 4:
                vars[8] = vars[9] * vars[10];
                vars[9] = vars[10] - vars[11];
                state = 0;
                break;
        }
        
        switch_middle_label:
        vars[10] = vars[11] + vars[12];
        vars[11] = vars[12] * vars[13];
        
        switch_start_label:
        vars[12] = vars[13] - vars[14];
        vars[13] = vars[14] / (vars[15] + 1);
        
        switch_end_label:
        vars[14] = vars[15] | vars[16];
        vars[15] = vars[16] & vars[17];
        
        switch_exit_label:
        vars[16] = vars[17] ^ vars[18];
        vars[17] = vars[18] + vars[19];
        
        /* Long dependency chain */
        for (int k = 0; k < MANY_VARS - 1; k++) {
            vars[k] = vars[k] + vars[k + 1] * (i + 1) / (k + 2);
        }
        
        /* Prevent optimization */
        asm volatile("" : : "r"(vars[0]), "r"(vars[1]), "r"(vars[2]), "r"(vars[3]));
        asm volatile("" : : "r"(vars[4]), "r"(vars[5]), "r"(vars[6]), "r"(vars[7]));
        asm volatile("" : : "r"(vars[8]), "r"(vars[9]), "r"(vars[10]), "r"(vars[11]));
        asm volatile("" : : "r"(vars[12]), "r"(vars[13]), "r"(vars[14]), "r"(vars[15]));
        asm volatile("" : : "r"(vars[16]), "r"(vars[17]), "r"(vars[18]), "r"(vars[19]));
        
        /* Update state based on computation */
        state = (vars[0] + i) % 5;
    }
    
    for (int k = 0; k < MANY_VARS; k++) {
        checksum += vars[k];
    }
    return checksum;
}

__attribute__((noinline, noipa))
static uint64_t test_computed_goto(int iterations, int seed) {
    /* State machine using computed goto */
    static void* labels[] = {
        &&state0, &&state1, &&state2, &&state3, &&state4,
        &&state5, &&state6, &&state7, &&state8, &&state9
    };
    
    volatile int regs[20];
    for (int k = 0; k < 20; k++) {
        regs[k] = seed + k * 7;
    }
    
    int state = seed % 10;
    uint64_t checksum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Computed goto - creates complex CFG */
        goto *labels[state];
        
        state0:
        regs[0] = regs[1] + regs[2] * regs[3];
        regs[1] = regs[2] - regs[3] / (regs[4] + 1);
        state = (regs[0] % 9) + 1;
        continue;
        
        state1:
        regs[2] = regs[3] | regs[4] & regs[5];
        regs[3] = regs[4] ^ regs[5] + regs[6];
        state = (regs[1] % 9) + 2;
        continue;
        
        state2:
        regs[4] = regs[5] * regs[6] - regs[7];
        regs[5] = regs[6] / (regs[7] + 1) + regs[8];
        state = (regs[2] % 9) + 3;
        continue;
        
        state3:
        regs[6] = regs[7] & regs[8] | regs[9];
        regs[7] = regs[8] + regs[9] * regs[10];
        state = (regs[3] % 9) + 4;
        continue;
        
        state4:
        regs[8] = regs[9] - regs[10] / regs[11];
        regs[9] = regs[10] * regs[11] + regs[12];
        state = (regs[4] % 9) + 5;
        continue;
        
        state5:
        regs[10] = regs[11] | regs[12] & regs[13];
        regs[11] = regs[12] ^ regs[13] + regs[14];
        state = (regs[5] % 9) + 6;
        continue;
        
        state6:
        regs[12] = regs[13] * regs[14] - regs[15];
        regs[13] = regs[14] / (regs[15] + 1) + regs[16];
        state = (regs[6] % 9) + 7;
        continue;
        
        state7:
        regs[14] = regs[15] & regs[16] | regs[17];
        regs[15] = regs[16] + regs[17] * regs[18];
        state = (regs[7] % 9) + 8;
        continue;
        
        state8:
        regs[16] = regs[17] - regs[18] / regs[19];
        regs[17] = regs[18] * regs[19] + regs[0];
        state = (regs[8] % 9) + 9;
        continue;
        
        state9:
        regs[18] = regs[19] | regs[0] & regs[1];
        regs[19] = regs[0] ^ regs[1] + regs[2];
        state = (regs[9] % 9);
        continue;
    }
    
    /* Long dependency chain across all registers */
    for (int k = 0; k < 19; k++) {
        regs[k] = regs[k] + regs[k + 1] * 3 - regs[(k + 2) % 20] / 2;
    }
    
    /* Force all registers to be live */
    for (int k = 0; k < 20; k++) {
        asm volatile("" : : "r"(regs[k]));
        checksum += regs[k];
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
    
    uint64_t total_checksum = 0;
    
    /* Run all test functions to increase coverage chance */
    total_checksum += test_irreducible_goto(iterations, seed);
    total_checksum += test_switch_goto(iterations, seed + 1);
    total_checksum += test_computed_goto(iterations / 10, seed + 2);
    
    printf("Total checksum: %lu\n", total_checksum);
    
    return (int)(total_checksum % 256);
}

/* test_sel_sched.c - Test case for GCC selective scheduling debug coverage */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and mixed operations */
unsigned long complex_loop(unsigned int seed, int iterations) {
    volatile unsigned long state = seed; /* Prevent optimization */
    unsigned long result = 0;
    int i, j;
    
    /* Outer loop with loop-carried dependency */
    for (i = 0; i < iterations; i++) {
        /* Multiple arithmetic operations creating dependencies */
        state = (state * 1103515245 + 12345) & 0x7fffffff;
        
        /* Data-dependent branching */
        if (state % 3 == 0) {
            result += (state & 0xFF) * 3;
        } else if (state % 5 == 0) {
            result ^= (state >> 8) | 0x1F;
        } else {
            result = (result << 2) | (state % 31);
        }
        
        /* Nested loop with break condition */
        for (j = 0; j < 3; j++) {
            /* Mixed integer operations */
            unsigned long temp = state * (j + 1);
            temp = (temp % 1023) + (temp >> 10);
            
            /* Conditional operation */
            if (temp > 500) {
                result -= temp / 7;
                if (result < 1000) break; /* Early exit */
            } else {
                result |= temp & 0x3F;
            }
            
            /* Memory barrier simulation */
            asm volatile("" ::: "memory");
        }
        
        /* Bitwise operations */
        result = (result ^ (state >> 16)) + (result & 0xFFFF);
        
        /* More arithmetic with different types */
        int signed_val = (int)(state % 1000) - 500;
        if (signed_val > 0) {
            result += signed_val * 2;
        } else {
            result -= (-signed_val) / 3;
        }
    }
    
    return result;
}

/* Another function with switch statement for control flow variety */
int switch_based_computation(int base, int mod) {
    int acc = base;
    volatile int counter = 0; /* Prevent dead code elimination */
    
    for (int i = 0; i < 100; i++) {
        int val = (base * i + mod) % 7;
        
        /* Switch with multiple cases */
        switch (val) {
            case 0:
                acc += i * 2;
                asm volatile("" ::: "memory"); /* Barrier */
                break;
            case 1:
                acc ^= (i << 3);
                break;
            case 2:
                acc = (acc * 3) / 2;
                break;
            case 3:
                acc |= 0x0F0F0F0F;
                break;
            case 4:
                acc -= mod * i;
                break;
            case 5:
                acc = (acc % 1024) + (i & 0xFF);
                break;
            default:
                acc = ~acc;
                break;
        }
        
        /* Prevent loop unrolling */
        counter++;
    }
    
    return acc;
}

/* Main driver with runtime input to prevent compile-time computation */
int main(int argc, char *argv[]) {
    unsigned int seed = 42;
    int iterations = 1000;
    
    /* Use command line input if available */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations > 10000) iterations = 10000;
        if (iterations < 100) iterations = 100;
    }
    
    printf("Starting computation with seed=%u, iterations=%d\n", seed, iterations);
    
    /* Call complex functions multiple times */
    unsigned long total = 0;
    for (int run = 0; run < 3; run++) {
        unsigned long res1 = complex_loop(seed + run, iterations / 3);
        int res2 = switch_based_computation(seed + run * 100, run + 1);
        
        total += res1 + res2;
        printf("Run %d: res1=%lu, res2=%d\n", run, res1, res2);
        
        /* Create dependency between runs */
        seed = (seed + res1) & 0xFFFF;
    }
    
    printf("Final result: %lu\n", total);
    return (int)(total % 256);
}

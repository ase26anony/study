/* test_sel_sched.c - Test case for GCC selective scheduling debug coverage */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and mixed operations */
static unsigned long complex_loop(int iterations, int seed) {
    volatile int memory_barrier; /* Prevent optimization */
    unsigned long state = seed;
    unsigned long result = 0;
    int i, j;
    
    /* Outer loop with loop-carried dependency */
    for (i = 0; i < iterations; i++) {
        /* Inner loop to increase instruction count */
        for (j = 0; j < 5; j++) {
            /* Data-dependent branching */
            if ((state & 0xFF) % 3 == 0) {
                /* Arithmetic operations */
                state = state * 1103515245 + 12345;
                result += (state >> 16) & 0x7FFF;
            } else if ((state & 0xFF) % 5 == 0) {
                /* Bitwise operations */
                state = (state ^ (state >> 3)) * 1664525 + 1013904223;
                result ^= state & 0xFFFF;
            } else {
                /* Mixed operations */
                state = (state * 6364136223846793005ULL) + 1442695040888963407ULL;
                result += ((state >> 32) & 0xFFFF) * ((state >> 16) & 0xFFFF);
            }
            
            /* More arithmetic to increase scheduling complexity */
            result = (result << 3) | (result >> 61); /* Rotate left 3 */
            state = state % 0x7FFFFFFF;
            
            /* Artificial memory dependency */
            memory_barrier = (int)state;
            asm volatile("" : : "r"(memory_barrier) : "memory");
        }
        
        /* Conditional break based on computed value */
        if ((result & 0xFFFFFF) == 0x123456) {
            break;
        }
        
        /* Switch-like structure */
        switch (i % 4) {
            case 0:
                result += i * 7;
                break;
            case 1:
                result -= i * 3;
                break;
            case 2:
                result ^= i * 11;
                break;
            case 3:
                result |= i * 13;
                break;
        }
    }
    
    /* Final mixing */
    result = (result ^ (result >> 33)) * 0xff51afd7ed558ccdULL;
    result = (result ^ (result >> 33)) * 0xc4ceb9fe1a85ec53ULL;
    result = result ^ (result >> 33);
    
    return result;
}

/* Another function with different pattern */
static unsigned long another_scheduler_test(int start, int mod) {
    volatile int sink;
    unsigned long acc = start;
    int i;
    
    for (i = 0; i < 1000; i++) {
        /* Division/modulo operations are expensive and interesting for scheduling */
        acc = (acc * 3) / (mod + 1) + (acc % (mod + 2));
        
        /* Bit manipulation */
        acc = (acc & 0xAAAAAAAA) >> 1 | (acc & 0x55555555) << 1;
        acc = acc ^ (i * 0x9e3779b9);
        
        /* Memory barrier */
        sink = (int)acc;
        asm volatile("" : : "r"(sink) : "memory");
        
        /* Early exit condition */
        if (acc > 0x10000000) {
            acc >>= 8;
            if (acc < 100) break;
        }
    }
    
    return acc;
}

/* Main driver that prevents compile-time computation */
int main(int argc, char **argv) {
    int iterations, seed;
    unsigned long result1, result2, final_result;
    
    /* Use command line arguments to prevent constant folding */
    if (argc > 2) {
        iterations = atoi(argv[1]);
        seed = atoi(argv[2]);
    } else {
        /* Default values if no args provided */
        iterations = 100;
        seed = 42;
    }
    
    /* Call complex functions multiple times */
    result1 = complex_loop(iterations, seed);
    result2 = another_scheduler_test(seed, iterations % 100 + 1);
    
    /* Combine results in non-trivial way */
    final_result = result1 ^ result2;
    final_result = final_result * 0x9e3779b97f4a7c15ULL;
    
    /* Print result to prevent dead code elimination */
    printf("Result: 0x%016lx\n", final_result);
    
    return (int)(final_result & 0x7FFFFFFF);
}

/* test_sel_sched.c - Test case for GCC selective scheduling debug coverage */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent control flow and operations */
unsigned long complex_loop(unsigned int seed, int iterations) {
    volatile unsigned long state = seed;  /* Prevent optimization */
    unsigned long result = 0;
    int i, j;
    
    for (i = 0; i < iterations; i++) {
        /* Multiple arithmetic operations creating dependencies */
        state = (state * 1103515245 + 12345) & 0x7fffffff;
        
        /* Bitwise operations mixed with arithmetic */
        unsigned long temp = (state >> 16) & 0x7ff;
        temp = (temp * 16807) % 2147483647;
        
        /* Control flow based on computed values */
        if (state % 3 == 0) {
            result += (temp << 2) | 0x1;
        } else if (state % 5 == 0) {
            result ^= (temp >> 1) & 0x3ff;
        } else {
            result = (result * 13 + temp) % 1000000007;
        }
        
        /* Nested loop with break condition */
        for (j = 0; j < 3; j++) {
            if ((state >> j) & 1) {
                result += j * 17;
                if (result > 1000000) {
                    result %= 1000;
                }
            }
        }
        
        /* Mixed data types operations */
        int signed_val = (int)(state & 0xfff) - 2048;
        if (signed_val > 0) {
            result += (unsigned long)signed_val * 31;
        } else {
            result -= (unsigned long)(-signed_val) / 7;
        }
        
        /* Memory barrier via inline asm */
        asm volatile("" ::: "memory");
    }
    
    return result;
}

/* Another function with different pattern */
unsigned long branching_pattern(int start, int count) {
    unsigned long acc = start;
    int i;
    
    for (i = 0; i < count; i++) {
        /* Switch-like behavior */
        switch (i % 4) {
            case 0:
                acc = (acc * 3 + 1) & 0xffff;
                break;
            case 1:
                acc = (acc ^ 0x5a5a) << 1;
                break;
            case 2:
                acc = (acc + i * 7) % 7919;
                break;
            case 3:
                acc = (acc >> 2) | (acc << 14);
                /* Early exit condition */
                if (acc > 100000) {
                    return acc;
                }
                break;
        }
        
        /* Division/modulo operations (expensive) */
        if (acc != 0) {
            acc = (acc / 3) | (acc % 17);
        }
        
        /* More arithmetic chains */
        acc = acc + (acc << 4);
        acc = acc ^ (acc >> 7);
        acc = acc * 0x9e3779b9;
    }
    
    return acc;
}

/* Main driver that uses both functions */
int main(int argc, char **argv) {
    unsigned int seed = 42;
    int iterations = 100;
    
    /* Use command line arguments to prevent compile-time computation */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations > 1000) iterations = 1000;
        if (iterations < 10) iterations = 10;
    }
    
    printf("Seed: %u, Iterations: %d\n", seed, iterations);
    
    /* Call both complex functions */
    unsigned long result1 = complex_loop(seed, iterations);
    unsigned long result2 = branching_pattern(seed % 100, iterations / 2);
    
    /* Combine results with volatile write to prevent elimination */
    volatile unsigned long sink;
    sink = result1 + result2;
    
    printf("Result1: %lu, Result2: %lu, Combined: %lu\n", 
           result1, result2, (unsigned long)sink);
    
    return (int)((result1 + result2) & 0x7fffffff);
}

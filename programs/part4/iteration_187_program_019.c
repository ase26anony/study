/* test_sel_sched.c - Test case for GCC selective scheduling debug coverage */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and mixed operations */
unsigned long complex_loop(unsigned seed, int iterations) {
    volatile unsigned long sink; /* Prevent dead code elimination */
    unsigned long a = seed;
    unsigned long b = 0x5A5A5A5A;
    unsigned long c = 0;
    unsigned long d = 1;
    
    /* Loop with data-dependent control flow and mixed operations */
    for (int i = 0; i < iterations; i++) {
        /* Multiple arithmetic operations creating dependencies */
        a = (a * 1103515245 + 12345) & 0x7FFFFFFF;
        b = b ^ (a << 13) ^ (a >> 17);
        c = c + (a % 100) - (b % 50);
        
        /* Data-dependent branching */
        if (a % 3 == 0) {
            d = d * 2 + (c & 0xFF);
            /* Bitwise operations */
            b = b | (d << 8);
        } else if (a % 7 == 0) {
            d = d / 3 + (c >> 4);
            /* More mixed operations */
            c = c ^ (d * 0x9E3779B9);
        } else {
            d = d + a - b;
            /* Inline asm to create memory barrier */
            asm volatile("" ::: "memory");
        }
        
        /* Nested conditional */
        if (i % 100 == 0) {
            /* Division/modulo operations are expensive */
            unsigned long temp = d % 1000;
            c = c ^ temp;
            /* Another memory barrier */
            asm volatile("" ::: "memory");
        }
        
        /* Break condition based on computed value */
        if (d > 0x10000000) {
            d = d >> 4;
        }
    }
    
    /* Mix all values to create final result */
    unsigned long result = (a << 32) | (b & 0xFFFFFFFF);
    result = result ^ c ^ d;
    
    /* Volatile write to prevent elimination */
    sink = result;
    return result;
}

/* Another function with different pattern */
unsigned long nested_loops(unsigned start, int outer, int inner) {
    unsigned long acc = start;
    
    for (int i = 0; i < outer; i++) {
        int limit = inner + (i % 5);
        for (int j = 0; j < limit; j++) {
            /* Mixed operations with different data types */
            acc = acc * 1664525 + 1013904223;
            int temp = (int)(acc & 0xFF);
            
            /* Switch statement for control flow complexity */
            switch (temp % 4) {
                case 0:
                    acc = acc + (temp << 16);
                    break;
                case 1:
                    acc = acc - (temp * 0xABCD);
                    break;
                case 2:
                    acc = acc ^ (0xDEADBEEF * temp);
                    break;
                case 3:
                    acc = acc | (0xCAFEBABE >> temp);
                    break;
            }
            
            /* Early exit condition */
            if (acc > 0x8000000000000000UL) {
                acc = acc >> 1;
                if (j > 10) break;
            }
        }
        
        /* Function call to prevent inlining optimization */
        acc = complex_loop(acc & 0xFFFFFFFF, 5);
    }
    
    return acc;
}

/* Main driver with runtime input */
int main(int argc, char *argv[]) {
    unsigned seed = 42;
    int iterations = 1000;
    
    /* Use command line arguments to prevent compile-time computation */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    /* Call complex functions multiple times */
    unsigned long result1 = complex_loop(seed, iterations);
    unsigned long result2 = nested_loops(result1, 10, 50);
    unsigned long result3 = complex_loop(result2, iterations / 2);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %lu %lu %lu\n", result1, result2, result3);
    
    /* Additional computation with volatile reads */
    volatile unsigned long final = result1 ^ result2 ^ result3;
    printf("Final: %lu\n", (unsigned long)final);
    
    return (int)(final & 0x7FFFFFFF);
}

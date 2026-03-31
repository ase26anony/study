/* test_sel_sched.c - Test case for GCC selective scheduling debug coverage */

#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and operations */
unsigned int complex_loop(unsigned int seed, int iterations) {
    volatile unsigned int sink; /* Prevent dead code elimination */
    unsigned int a = seed;
    unsigned int b = 0x9e3779b9; /* Golden ratio constant */
    unsigned int c = 0;
    unsigned int d = 1;
    
    /* Create artificial dependencies with inline asm */
    asm volatile("" : "+r" (a), "+r" (b) : : "memory");
    
    for (int i = 0; i < iterations; i++) {
        /* Multiple arithmetic operations creating dependencies */
        a = a * 1664525 + 1013904223;
        b = b ^ (b >> 13);
        b = b ^ (b << 17);
        b = b ^ (b >> 5);
        
        /* Data-dependent branching */
        if (i % 3 == 0) {
            c = (c + a) * 1103515245 + 12345;
            d = d ^ (d << 13);
        } else if (i % 3 == 1) {
            c = (c ^ b) * 69069 + 1;
            d = d + (d >> 17);
        } else {
            c = (c * a) % 2147483647;
            d = d | (d << 5);
        }
        
        /* Bitwise operations mixed with arithmetic */
        unsigned int temp = (a & 0x55555555) + (b & 0xAAAAAAAA);
        temp = (temp << 1) | (temp >> 31); /* Rotate left */
        
        /* More complex operations with different data types */
        long long big_val = (long long)a * (long long)b;
        c = c ^ (unsigned int)(big_val >> 32);
        d = d + (unsigned int)(big_val & 0xFFFFFFFF);
        
        /* Conditional break based on computation */
        if (c > 0x80000000 && i > iterations / 2) {
            d = d * 3;
            break;
        }
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < 2; j++) {
            d = (d + j) * 16807 % 2147483647;
        }
        
        /* Switch statement for control flow variety */
        switch (i % 4) {
            case 0: a = a + b; break;
            case 1: a = a - b; break;
            case 2: a = a ^ b; break;
            case 3: a = a * b; break;
        }
    }
    
    /* Final mixing */
    unsigned int result = a ^ b ^ c ^ d;
    sink = result; /* Volatile write to prevent elimination */
    
    return result;
}

/* Another function with different pattern */
unsigned int hash_chain(unsigned int start, int steps) {
    unsigned int h = start;
    
    for (int i = 0; i < steps; i++) {
        /* Multiple dependent operations */
        h = h * 31 + (h >> 16);
        h = h ^ (h << 7);
        h = h + (h << 15);
        h = h % 1000000007;
        
        /* Memory barrier via asm */
        asm volatile("" : : : "memory");
        
        /* Conditional with side effect */
        if (h % 1000 < 500) {
            h = h | 0x0000FFFF;
        } else {
            h = h & 0xFFFF0000;
        }
    }
    
    return h;
}

/* Main driver with unpredictable inputs */
int main(int argc, char *argv[]) {
    unsigned int seed1 = 12345;
    unsigned int seed2 = 67890;
    int iterations = 100;
    
    /* Use command line arguments to prevent compile-time computation */
    if (argc > 1) {
        seed1 = atoi(argv[1]);
    }
    if (argc > 2) {
        seed2 = atoi(argv[2]);
    }
    if (argc > 3) {
        iterations = atoi(argv[3]);
        if (iterations > 1000) iterations = 1000;
        if (iterations < 10) iterations = 10;
    }
    
    /* Call complex functions multiple times */
    unsigned int result1 = 0;
    unsigned int result2 = 0;
    
    for (int run = 0; run < 3; run++) {
        result1 ^= complex_loop(seed1 + run, iterations);
        result2 ^= hash_chain(seed2 + run, iterations / 2);
        
        /* Mix results */
        result1 = result1 ^ result2;
        result2 = result2 + result1;
    }
    
    /* Final output prevents elimination */
    printf("Result: %u %u\n", result1, result2);
    
    return 0;
}

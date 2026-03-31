/* test_sel_sched.c - Test case for GCC selective scheduling debug coverage */

#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and operations */
unsigned int complex_loop(unsigned int seed, int iterations) {
    volatile unsigned int sink; /* Prevent dead code elimination */
    unsigned int a = seed;
    unsigned int b = 0x9e3779b9; /* Golden ratio */
    unsigned int c = 0;
    unsigned int d = 0xdeadbeef;
    
    for (int i = 0; i < iterations; i++) {
        /* Multiple arithmetic operations creating dependencies */
        a = (a ^ (a >> 13)) * 0x85ebca6b;
        b = (b + (b << 7)) ^ 0x165667b1;
        c = (c * 1664525) + 1013904223;
        
        /* Data-dependent branching */
        if (i % 3 == 0) {
            d = d ^ (a + b);
            /* Inline asm to create memory barrier */
            asm volatile("" ::: "memory");
        } else if (i % 5 == 0) {
            d = d * 0x5bd1e995;
            d = d ^ (d >> 15);
        } else {
            d = (d + c) | 0x80000000;
        }
        
        /* More operations with different data types */
        int temp = (int)d;
        if (temp < 0) {
            temp = -temp;
            d = (unsigned int)temp;
        }
        
        /* Bitwise operations mixing types */
        long long big = (long long)a * (long long)b;
        d ^= (unsigned int)(big >> 32);
        
        /* Conditional break based on computation */
        if (d == 0) break;
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < 2; j++) {
            d = (d << 1) | (d >> 31); /* Rotate left */
            if (j == 1) {
                d ^= 0xaaaaaaaa;
            }
        }
    }
    
    sink = a ^ b ^ c ^ d;
    return sink;
}

/* Another function with switch statement */
int control_flow_test(int x, int y) {
    int result = 0;
    
    for (int i = 0; i < 100; i++) {
        switch (i % 4) {
            case 0:
                result += x * y;
                x = (x << 1) | (x >> 31);
                break;
            case 1:
                result -= x / (y ? y : 1);
                y = y ^ result;
                break;
            case 2:
                result *= (x & 0xff) + 1;
                /* Memory clobber to prevent reordering */
                asm volatile("" ::: "memory");
                break;
            case 3:
                result = result % 10007;
                x = x + y;
                y = y - x;
                break;
        }
        
        /* Additional conditional */
        if (result > 1000000) {
            result = result >> 2;
        }
    }
    
    return result;
}

/* Main driver with input-dependent computation */
int main(int argc, char *argv[]) {
    unsigned int seed = 12345;
    int iterations = 1000;
    
    /* Use command line arguments to prevent compile-time computation */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations > 10000) iterations = 10000;
        if (iterations < 10) iterations = 10;
    }
    
    /* Call complex functions multiple times */
    unsigned int hash = 0;
    for (int k = 0; k < 3; k++) {
        hash ^= complex_loop(seed + k, iterations);
        hash = (hash << 13) | (hash >> 19);
    }
    
    int flow_result = control_flow_test((int)hash, iterations % 256);
    
    /* Final result depends on all computations */
    printf("Result: %u (flow: %d)\n", hash, flow_result);
    
    return (int)(hash ^ flow_result);
}

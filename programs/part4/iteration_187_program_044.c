/* test_sel_sched.c - Test case for GCC selective scheduling debug coverage */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and operations */
unsigned int complex_loop(unsigned int seed, int iterations) {
    volatile unsigned int sink; /* Prevent dead code elimination */
    unsigned int a = seed;
    unsigned int b = 0x9e3779b9; /* Golden ratio constant */
    unsigned int c = 0;
    unsigned int d = 0xdeadbeef;
    
    for (int i = 0; i < iterations; i++) {
        /* Multiple arithmetic operations creating dependencies */
        a = (a ^ (a >> 13)) * 0x85ebca6b;
        b = (b ^ (b << 17)) ^ 0xc2b2ae35;
        c = (c + a) * 1103515245 + 12345;
        d = (d ^ (d >> 16)) + (b << 3);
        
        /* Data-dependent branching with different operations */
        if ((i % 7) == 0) {
            a = (a * 3) / 2;
            b = b ^ (c << 1);
            /* Inline asm to create memory barrier */
            asm volatile("" ::: "memory");
        } else if ((i % 5) == 0) {
            c = (c | d) & 0x7fffffff;
            d = d - (a >> 2);
        } else {
            a = a + (b % 7919); /* Prime number modulus */
            c = c ^ (d << 4);
        }
        
        /* Nested conditional with bitwise operations */
        if ((a & 0xff) > 128) {
            for (int j = 0; j < 3; j++) {
                d = (d << 1) | (d >> 31); /* Rotate left */
                c = c ^ (a << j);
            }
            if (d > 0x80000000) {
                b = b - 1;
            }
        }
        
        /* Mix different data types */
        long long temp = (long long)a * (long long)b;
        a = (unsigned int)(temp >> 32);
        b = (unsigned int)(temp & 0xffffffff);
        
        /* Volatile write to prevent optimization */
        sink = a;
    }
    
    /* Create loop-carried dependency */
    unsigned int result = a ^ b ^ c ^ d;
    
    /* More complex control flow */
    switch (result % 8) {
        case 0: result = result << 1; break;
        case 1: result = result >> 1; break;
        case 2: result = result * 3; break;
        case 3: result = result / 2; break;
        case 4: result = result | 0x55555555; break;
        case 5: result = result & 0xaaaaaaaa; break;
        case 6: result = result ^ 0x33333333; break;
        default: result = ~result; break;
    }
    
    return result;
}

/* Second function with different pattern */
int hash_computation(char *data, int len) {
    int hash = 5381;
    volatile int barrier;
    
    for (int i = 0; i < len; i++) {
        /* Classic DJB2 hash algorithm with modifications */
        hash = ((hash << 5) + hash) + data[i];
        
        /* Conditional operations based on hash value */
        if (hash & 1) {
            hash = hash ^ 0x5a827999;
            /* Memory clobber to force scheduling constraints */
            asm volatile("" ::: "memory");
        }
        
        if (i % 4 == 0) {
            hash = (hash * 16777619) ^ 0x811c9dc5;
        }
        
        /* Prevent loop unrolling */
        barrier = hash;
        if (barrier == 0) break; /* Unlikely, but creates CFG edge */
    }
    
    /* Additional processing */
    for (int i = 0; i < 3; i++) {
        hash = (hash >> (i * 3)) | (hash << (32 - i * 3));
        hash = hash + i * 0x9e3779b9;
    }
    
    return hash;
}

/* Main driver with external input */
int main(int argc, char *argv[]) {
    unsigned int result1, result2;
    int final_result;
    
    /* Use command line argument or fixed value */
    unsigned int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call complex function multiple times */
    result1 = complex_loop(seed, 100);
    result2 = complex_loop(result1, 50);
    
    /* Process some data */
    char test_data[] = "This is test data for hash computation with selective scheduling";
    int hash = hash_computation(test_data, sizeof(test_data) - 1);
    
    /* Combine results in non-trivial way */
    final_result = (result1 ^ result2) + hash;
    
    /* Use result to prevent elimination */
    printf("Result: %d (0x%08x)\n", final_result, final_result);
    
    /* Additional loop with break condition */
    int temp = final_result;
    for (int i = 0; i < 100; i++) {
        temp = (temp * 1103515245 + 12345) & 0x7fffffff;
        if (temp > 0x70000000) {
            temp = temp / 3;
            break;
        }
        temp = temp ^ 0x12345678;
    }
    
    printf("Final: %d\n", temp);
    
    return temp != 0 ? 0 : 1;
}

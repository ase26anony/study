/* test_sel_sched.c - Test case for GCC selective scheduling debug coverage */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and operations */
unsigned long complex_loop(unsigned seed, int iterations) {
    volatile unsigned sink; /* Prevent dead code elimination */
    unsigned long a = seed * 1103515245 + 12345;
    unsigned long b = seed * 1664525 + 1013904223;
    unsigned long c = 0;
    unsigned long d = 1;
    
    for (int i = 0; i < iterations; i++) {
        /* Multiple arithmetic operations creating dependencies */
        a = (a * 1103515245 + 12345) & 0x7fffffff;
        b = (b * 1664525 + 1013904223) & 0xffffffff;
        
        /* Data-dependent branching */
        if (a % 3 == 0) {
            c += (a >> 16) & 0xff;
            d ^= (b << 8) | (a & 0xff);
        } else if (a % 7 == 0) {
            c -= (b >> 8) & 0xffff;
            d |= (a << 16) | (b & 0xff00);
        } else {
            c *= (a & 0xfff) + 1;
            d &= (b | 0x55555555);
        }
        
        /* More operations with different data types */
        int temp = (int)(a % 1000);
        if (temp < 100) {
            c = (c << 3) | (c >> 61); /* 64-bit rotate */
            d = d % (temp + 1000);
        }
        
        /* Bitwise operations mixing types */
        unsigned short s = (unsigned short)(a ^ b);
        c += s * 17;
        d -= (unsigned long)s * 31;
        
        /* Memory barrier via inline asm */
        asm volatile("" ::: "memory");
        
        /* Nested conditional */
        if (c > 1000000) {
            unsigned long t = c;
            c = d;
            d = t;
            if (d % 11 == 0) {
                c = c / 13;
            }
        }
        
        /* Break condition based on computation */
        if (c > 2000000 && i > iterations/2) {
            c = c % 1000000;
            break;
        }
    }
    
    /* Final mixing */
    unsigned long result = (c ^ d) + (a << 32 | b);
    sink = (unsigned)result; /* Force side effect */
    
    return result;
}

/* Another function with different pattern */
unsigned long hash_array(const unsigned char *data, int len) {
    unsigned long hash1 = 5381;
    unsigned long hash2 = 0;
    
    for (int i = 0; i < len; i++) {
        /* Complex hash computation */
        hash1 = ((hash1 << 5) + hash1) ^ data[i];
        
        /* Conditional with modulo */
        if (i % 4 == 0) {
            hash2 = (hash2 * 31 + data[i]) & 0xffffffff;
        } else if (i % 4 == 1) {
            hash2 = (hash2 ^ (data[i] << 8)) | 0x80000000;
        } else if (i % 4 == 2) {
            hash2 = (hash2 + (data[i] * 17)) % 1000000007;
        } else {
            hash2 = (hash2 - data[i]) ^ 0x5a827999;
        }
        
        /* Switch statement for variety */
        switch (data[i] % 5) {
            case 0: hash1 = hash1 >> 1; break;
            case 1: hash1 = hash1 << 2; break;
            case 2: hash1 = hash1 ^ hash2; break;
            case 3: hash1 = hash1 * 3; break;
            case 4: hash1 = hash1 % 997; break;
        }
        
        /* Prevent optimization */
        asm volatile("" : "+r" (hash1), "+r" (hash2) :: "memory");
    }
    
    return hash1 ^ hash2;
}

/* Main driver with external input */
int main(int argc, char *argv[]) {
    unsigned seed = 42;
    int iterations = 1000;
    
    /* Use command line input if available */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations > 10000) iterations = 10000;
        if (iterations < 10) iterations = 10;
    }
    
    /* Call complex functions multiple times */
    unsigned long total = 0;
    for (int j = 0; j < 5; j++) {
        total += complex_loop(seed + j, iterations);
    }
    
    /* Process some data */
    unsigned char data[256];
    for (int i = 0; i < 256; i++) {
        data[i] = (unsigned char)((i * 37 + seed) & 0xff);
    }
    
    total ^= hash_array(data, 256);
    
    /* Use result to prevent elimination */
    printf("Result: %lu\n", total);
    
    return (int)(total % 256);
}

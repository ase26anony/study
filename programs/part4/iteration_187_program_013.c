/* test_sel_sched.c - Test case for GCC selective scheduling debug coverage */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and multiple operations */
unsigned int complex_loop(unsigned int seed, int iterations) {
    volatile unsigned int sink; /* Prevent dead code elimination */
    unsigned int a = seed;
    unsigned int b = 0x9e3779b9; /* Golden ratio */
    unsigned int c = 0;
    unsigned int d = 0xdeadbeef;
    
    for (int i = 0; i < iterations; i++) {
        /* Multiple arithmetic operations creating dependencies */
        a = (a ^ (a >> 13)) * 0x5bd1e995;
        b = (b + (b << 7)) ^ (b >> 11);
        c = (c * 1664525) + 1013904223; /* Linear congruential generator */
        d = d ^ (d << 13);
        d = d ^ (d >> 17);
        d = d ^ (d << 5);
        
        /* Data-dependent branching with complex conditions */
        if ((i % 7) == 0) {
            a = a + (b << 3);
            c = c ^ d;
        } else if ((i % 5) == 0) {
            b = b | (a >> 2);
            d = d * 0xcc9e2d51;
        } else if ((i % 3) == 0) {
            a = a - (c & 0xff);
            b = b ^ (d | 0x55555555);
        } else {
            c = c + (d % 137);
            a = a * 3;
        }
        
        /* Bitwise operations mixing results */
        unsigned int temp = (a & b) | (c ^ d);
        temp = (temp << 1) | (temp >> 31); /* Rotate left */
        
        /* More arithmetic with different data types */
        int signed_temp = (int)temp;
        if (signed_temp < 0) {
            signed_temp = -signed_temp;
            d = d + (unsigned int)signed_temp;
        }
        
        /* Memory barrier via inline asm to prevent reordering */
        asm volatile("" ::: "memory");
        
        /* Nested conditional with loop-carried dependency */
        if (temp > 0x80000000) {
            for (int j = 0; j < 2; j++) {
                c = c + (temp >> j);
                if (j == 1) break;
            }
        }
    }
    
    /* Mix all results with complex final computation */
    unsigned int result = (a ^ b) + (c * d);
    result = (result % 10007) + (result / 10007);
    
    /* Volatile write to prevent elimination */
    sink = result;
    
    return result;
}

/* Another function with different pattern to increase scheduling complexity */
int branching_pattern(int x, int y) {
    int r = 0;
    
    /* Switch statement creates control flow */
    switch (x % 8) {
        case 0: r = y * 2; break;
        case 1: r = y + (y >> 3); break;
        case 2: r = y ^ 0x7f; break;
        case 3: r = y % 17; break;
        case 4: r = (y << 4) | (y >> 4); break;
        case 5: r = y - (x * 3); break;
        case 6: r = y & 0x0f0f0f0f; break;
        case 7: r = y | 0x80808080; break;
    }
    
    /* Complex conditional chain */
    if (r > 1000) {
        r = r / 2;
    } else if (r < -1000) {
        r = r * 3;
    } else if (r == 0) {
        r = x + y;
    } else {
        r = r ^ x;
    }
    
    return r;
}

/* Main function that calls everything with runtime values */
int main(int argc, char *argv[]) {
    unsigned int seed = 12345;
    int iterations = 100;
    
    /* Use command line or stdin to prevent compile-time computation */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    /* Multiple calls with different parameters */
    unsigned int total = 0;
    for (int i = 0; i < 5; i++) {
        unsigned int r1 = complex_loop(seed + i, iterations + i * 10);
        int r2 = branching_pattern(i * 7, r1 % 1000);
        total += r1 + r2;
        
        /* Additional computation to increase scheduling opportunities */
        total = (total << 3) | (total >> 29); /* Rotate right */
    }
    
    printf("Result: %u\n", total);
    return 0;
}

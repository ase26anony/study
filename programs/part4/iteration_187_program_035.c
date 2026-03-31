/* test_sel_sched_dump.c
 * This test is designed to trigger GCC's selective scheduling debug output
 * when compiled with flags like -fsel-sched-dump or -fdump-rtl-sched*
 * The goal is to cover the debug dumping routines in sel-sched-dump.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile sink to prevent dead code elimination */
volatile int global_sink = 0;

/* Complex function with data-dependent branching and operations */
unsigned int complex_loop(unsigned int seed, int iterations) {
    unsigned int a = seed;
    unsigned int b = 0x9e3779b9; /* Golden ratio */
    unsigned int c = 0;
    unsigned int d = 0xdeadbeef;
    
    /* Create artificial dependencies with volatile */
    volatile unsigned int dep = seed;
    
    for (int i = 0; i < iterations; i++) {
        /* Multiple arithmetic operations creating dependencies */
        a = a * 1664525 + 1013904223;
        b = b ^ (b >> 13);
        b = b ^ (b << 17);
        b = b ^ (b >> 5);
        
        /* Data-dependent branching */
        if (i % 3 == 0) {
            c = c + (a & 0xFF);
            d = d ^ (b << 3);
        } else if (i % 3 == 1) {
            c = c - (b & 0xFF);
            d = d | (a >> 5);
        } else {
            c = c * 3;
            d = d & (a | b);
        }
        
        /* More operations with different types */
        int temp = (int)(c % 256);
        if (temp < 0) temp = -temp;
        
        /* Bitwise operations mixing types */
        d = d ^ ((unsigned int)temp << (i % 16));
        
        /* Memory barrier via inline asm */
        asm volatile("" ::: "memory");
        
        /* Use volatile dependency */
        a = a ^ dep;
        dep = dep + 1;
        
        /* Complex condition with early exit possibility */
        if (c > 0x7FFFFFFF) {
            c = c / 2;
            if (d > 0xFFFFFFFF / 2) {
                break; /* Early exit creates interesting CFG */
            }
        }
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < 2; j++) {
            d = (d << 1) | (d >> 31); /* Rotate right */
            c = c + j;
        }
    }
    
    /* Final mixing */
    unsigned int result = a ^ b ^ c ^ d;
    global_sink = result; /* Prevent elimination */
    return result;
}

/* Another function with different pattern */
unsigned int hash_chain(unsigned int start, int steps) {
    unsigned int h = start;
    
    for (int i = 0; i < steps; i++) {
        /* Switch statement for control flow variety */
        switch (i % 4) {
            case 0:
                h = h * 31 + 17;
                break;
            case 1:
                h = (h << 7) | (h >> 25);
                h = h ^ 0x5A827999;
                break;
            case 2:
                h = h + (h >> 16);
                h = h * 0x85EBCA6B;
                break;
            case 3:
                h = h - (h << 13);
                h = h ^ (h >> 17);
                break;
        }
        
        /* Conditional with side effect */
        if (h & 1) {
            h = h ^ 0xEDB88320;
        } else {
            h = h + 0x82F63B78;
        }
        
        /* Prevent simple analysis */
        volatile unsigned int barrier = h;
        h = h ^ barrier;
    }
    
    return h;
}

/* Main driver that uses both functions */
int main(int argc, char *argv[]) {
    unsigned int seed = 12345;
    
    /* Use command line or stdin for variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        printf("Enter seed: ");
        scanf("%u", &seed);
    }
    
    /* Call complex functions multiple times */
    unsigned int result1 = complex_loop(seed, 100);
    unsigned int result2 = hash_chain(seed, 50);
    unsigned int result3 = complex_loop(result1, 30);
    unsigned int result4 = hash_chain(result2, 40);
    
    /* Mix results */
    unsigned int final = result1 ^ result2 ^ result3 ^ result4;
    
    /* Use result to prevent elimination */
    global_sink = final;
    printf("Result: %u (sink: %d)\n", final, global_sink);
    
    return (final & 255); /* Non-zero exit code based on result */
}

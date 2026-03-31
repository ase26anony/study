/* test_sel_sched.c - Complex loop to trigger selective scheduling debug dumps */
#include <stdio.h>
#include <stdlib.h>

/* Volatile sink to prevent dead code elimination */
volatile int global_sink;

/* Complex function with data-dependent branching and mixed operations */
unsigned int complex_loop(unsigned int seed, int iterations) {
    unsigned int a = seed;
    unsigned int b = 0x9e3779b9; /* Golden ratio */
    unsigned int c = 0;
    unsigned int d = 1;
    
    for (int i = 0; i < iterations; i++) {
        /* Multiple arithmetic operations creating dependencies */
        a = (a ^ (a >> 13)) * 0x85ebca6b;
        b = (b + (b << 7)) ^ 0x165667b1;
        
        /* Data-dependent branching */
        if (i % 3 == 0) {
            c = (c + a) * 0xc2b2ae35;
            d = d ^ (d << 13);
        } else if (i % 3 == 1) {
            c = (c ^ b) + 0x27d4eb2f;
            d = (d >> 7) | (d << 25);
        } else {
            c = (c * 0x165667b1) - a;
            d = (d + b) ^ 0x85ebca6b;
        }
        
        /* Bitwise operations mixing results */
        a = a ^ (c & 0x55555555);
        b = b + (d | 0xaaaaaaaa);
        
        /* Modulo operation creating longer latency */
        if (i % 7 == 0) {
            a = a % 9973;  /* Prime number */
        }
        
        /* Inline asm to create memory barrier and prevent reordering */
        asm volatile("" ::: "memory");
        
        /* Nested conditional with loop-carried dependency */
        if (c > 0x80000000) {
            for (int j = 0; j < 2; j++) {
                b = b ^ (a << j);
            }
            if (d % 1024 == 0) {
                break;  /* Early exit possibility */
            }
        }
    }
    
    /* Final mixing */
    unsigned int result = (a ^ b) + (c ^ d);
    result = (result >> 16) | (result << 16);
    
    return result;
}

/* Second function with different pattern to increase scheduling complexity */
int alternating_operations(int base, unsigned int mask) {
    int x = base;
    unsigned int y = mask;
    
    for (int i = 0; i < 100; i++) {
        /* Switch statement creating control flow */
        switch (i % 4) {
            case 0:
                x = (x * 3) / 2;
                y = y ^ (y >> 2);
                break;
            case 1:
                x = x + (x << 1);
                y = y | 0x0f0f0f0f;
                break;
            case 2:
                x = x - (x >> 3);
                y = y & 0x33333333;
                break;
            case 3:
                x = x ^ 0x55aa55aa;
                y = y + (y << 4);
                break;
        }
        
        /* Cross-dependency between variables */
        if (x > 0) {
            y = y + x;
        } else {
            y = y - (-x);
        }
        
        /* Volatile write to prevent optimization */
        global_sink = x;
    }
    
    return x + (int)y;
}

/* Main driver with external input to prevent compile-time computation */
int main(int argc, char *argv[]) {
    unsigned int seed = 12345;
    int iterations = 1000;
    
    /* Use command line argument if provided */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    /* Call complex functions multiple times */
    unsigned int result1 = 0;
    int result2 = 0;
    
    for (int k = 0; k < 3; k++) {
        result1 ^= complex_loop(seed + k, iterations / (k + 1));
        result2 += alternating_operations(k * 1000, result1);
        
        /* Read from stdin to prevent optimization */
        if (k == 0 && argc == 1) {
            printf("Enter a number: ");
            scanf("%u", &seed);
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result1: %u\n", result1);
    printf("Result2: %d\n", result2);
    
    return (result1 + result2) % 256;
}

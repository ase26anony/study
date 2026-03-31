/* test_sel_sched.c - Test case for GCC selective scheduling debug coverage */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and mixed operations */
unsigned long complex_loop(unsigned seed, int iterations) {
    volatile unsigned long sink; /* Prevent dead code elimination */
    unsigned long a = seed;
    unsigned long b = 0x9e3779b97f4a7c15UL;
    unsigned long c = 0;
    unsigned long d = 1;
    
    for (int i = 0; i < iterations; i++) {
        /* Multiple arithmetic operations creating dependencies */
        a = (a ^ (a >> 30)) * 0xbf58476d1ce4e5b9UL;
        a = (a ^ (a >> 27)) * 0x94d049bb133111ebUL;
        a = a ^ (a >> 31);
        
        /* Data-dependent branching */
        if (a % 3 == 0) {
            b = (b * 1103515245UL + 12345) % 0x7fffffff;
            c += a * b;
        } else if (a % 5 == 0) {
            b = b ^ (b << 13);
            b = b ^ (b >> 17);
            b = b ^ (b << 5);
            d *= (b % 1024) + 1;
        } else {
            /* Mixed bitwise and arithmetic */
            c = c ^ (c << 13);
            c = c + a * d;
            d = (d << 1) | (d >> 63);
        }
        
        /* More operations with different data types */
        unsigned short s = (a >> 16) & 0xFFFF;
        int shift = (s % 16) + 1;
        c = (c << shift) | (c >> (64 - shift));
        
        /* Memory barrier via inline asm */
        asm volatile("" ::: "memory");
        
        /* Complex condition with early exit possibility */
        if (c > 0xFFFFFFFF00000000UL && i > iterations / 2) {
            d = d ^ c;
            break;
        }
    }
    
    /* Final computation with all accumulated values */
    unsigned long result = (a ^ b) + (c * d);
    
    /* Use volatile to ensure computation isn't optimized away */
    sink = result;
    return result + sink;
}

/* Another function with nested loops and switch statement */
int nested_operations(int base, int count) {
    volatile int vsink;
    int total = 0;
    int mod = base % 256;
    
    for (int i = 0; i < count; i++) {
        int val = i * mod;
        
        /* Switch with multiple cases */
        switch (val % 7) {
            case 0:
                total += val * 2;
                break;
            case 1:
                total -= val / 3;
                break;
            case 2:
                total ^= val;
                break;
            case 3:
                total = (total << 3) | (total >> 29);
                break;
            case 4:
                total = total % 1001;
                break;
            case 5:
                total = total * 7 + 11;
                break;
            case 6:
                total = ~total;
                break;
        }
        
        /* Inner loop with dependency */
        for (int j = 0; j < 3; j++) {
            total += (i * j) % 19;
            asm volatile("" ::: "memory"); /* Another memory barrier */
        }
        
        /* Division/modulo operations (expensive) */
        if (total != 0) {
            mod = (mod % (total & 0xFF)) + 1;
        }
    }
    
    vsink = total;
    return total + vsink;
}

/* Main driver with external input to prevent constant folding */
int main(int argc, char *argv[]) {
    unsigned seed = 42;
    int iterations = 1000;
    
    /* Use command line arguments to vary input */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    /* Call complex functions multiple times */
    unsigned long result1 = 0;
    int result2 = 0;
    
    for (int run = 0; run < 3; run++) {
        result1 ^= complex_loop(seed + run, iterations / (run + 1) + 10);
        result2 += nested_operations(seed + run * 17, 50);
        
        /* Mix results to create cross-dependencies */
        seed = (seed * 1664525U + 1013904223U) % 0xFFFFFFFF;
    }
    
    /* Print final result (prevents dead code elimination) */
    printf("Final results: %lu, %d\n", result1, result2);
    
    return (result1 + result2) % 256;
}

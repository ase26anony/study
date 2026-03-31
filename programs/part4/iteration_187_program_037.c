/* test_sel_sched.c - Complex loop to trigger selective scheduling debug output */

#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and multiple operations */
unsigned long complex_loop(unsigned long seed, int iterations) {
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
            c += b >> 16;
        } else if (a % 5 == 0) {
            b = b ^ (b << 13);
            b = b ^ (b >> 17);
            b = b ^ (b << 5);
            c -= b & 0xFF;
        } else {
            b = (b + 0x9e3779b9UL + (b << 6) + (b >> 2)) ^ a;
            c ^= b;
        }
        
        /* More operations with mixed types */
        d = d * 6364136223846793005UL + 1442695040888963407UL;
        
        /* Bitwise operations */
        unsigned long temp = (a & 0xF0F0F0F0F0F0F0F0UL) | 
                            (b & 0x0F0F0F0F0F0F0F0FUL);
        c = c ^ temp;
        
        /* Nested conditional with break */
        if (i > 100 && (c & 0xFFF) == 0xABC) {
            d = d >> 4;
            break;
        }
        
        /* Division/modulo operations (expensive, creates dependencies) */
        if (d != 0) {
            a = a % d;
            b = b / (d & 0xFF + 1);
        }
        
        /* Inline asm to create memory barrier and prevent reordering */
        asm volatile("" ::: "memory");
    }
    
    /* Final mixing */
    a = a ^ (a >> 33);
    a = a * 0xff51afd7ed558ccdUL;
    a = a ^ (a >> 33);
    a = a * 0xc4ceb9fe1a85ec53UL;
    a = a ^ (a >> 33);
    
    sink = a + b + c + d; /* Use volatile to prevent elimination */
    return a + b + c + d;
}

/* Another function with different pattern */
unsigned long nested_loops(unsigned long start, int outer) {
    unsigned long result = start;
    
    for (int i = 0; i < outer; i++) {
        int inner = (i % 7) + 3;
        for (int j = 0; j < inner; j++) {
            /* Switch statement for control flow complexity */
            switch ((i + j) % 4) {
                case 0:
                    result = (result << 1) | (result >> 63);
                    break;
                case 1:
                    result = result * 0x2545F4914F6CDD1DUL;
                    break;
                case 2:
                    result = result ^ (result * 0x9E3779B97F4A7C15UL);
                    break;
                case 3:
                    result = result + (result & 0xAAAAAAAAAAAAAAAAUL);
                    break;
            }
            
            /* Conditional with arithmetic */
            if (result % 2 == 0) {
                result = result / 2;
            } else {
                result = result * 3 + 1;
            }
        }
        
        /* Early exit condition */
        if (result < 1000) {
            result = result * 1000 + i;
            break;
        }
    }
    
    return result;
}

/* Main driver that uses input to prevent compile-time computation */
int main(int argc, char *argv[]) {
    unsigned long seed1 = 123456789;
    unsigned long seed2 = 987654321;
    int iterations = 1000;
    
    /* Use command line arguments to prevent constant propagation */
    if (argc > 1) {
        seed1 = strtoul(argv[1], NULL, 0);
    }
    if (argc > 2) {
        seed2 = strtoul(argv[2], NULL, 0);
    }
    if (argc > 3) {
        iterations = atoi(argv[3]);
        if (iterations < 100) iterations = 100;
        if (iterations > 10000) iterations = 10000;
    }
    
    /* Call complex functions multiple times */
    unsigned long result1 = 0;
    unsigned long result2 = 0;
    
    for (int i = 0; i < 5; i++) {
        result1 ^= complex_loop(seed1 + i, iterations / 5);
        result2 += nested_loops(seed2 + i * 100, 10 + (i % 5));
    }
    
    /* Mix results */
    unsigned long final_result = result1 ^ result2;
    
    /* Use result to prevent elimination */
    printf("Result: 0x%016lx\n", final_result);
    
    return (final_result & 0xFF) == 0 ? 0 : 1;
}

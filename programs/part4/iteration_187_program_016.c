/* test_sel_sched.c - Test case for GCC selective scheduling debug coverage */

#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and mixed operations */
unsigned long complex_loop(unsigned int seed, int iterations) {
    volatile unsigned long sink; /* Prevent dead code elimination */
    unsigned long a = seed * 1103515245UL + 12345;
    unsigned long b = seed * 1664525UL + 1013904223UL;
    unsigned long c = 0;
    unsigned long d = 1;
    
    for (int i = 0; i < iterations; i++) {
        /* Data-dependent branching to create complex CFG */
        if (i % 7 == 0) {
            a = (a * 6364136223846793005UL) + 1442695040888963407UL;
            b = b ^ (b >> 29);
            c = c + (a & 0xFFFFFFFF);
        } else if (i % 5 == 0) {
            a = a ^ (a << 13);
            b = (b * 1103515245UL) + 12345;
            c = c | (b & 0xFFFF);
        } else if (i % 3 == 0) {
            a = a + (b >> 17);
            b = b - (a << 5);
            c = c ^ (a * b);
        } else {
            a = a + i;
            b = b - (i * 3);
            c = c & (a | b);
        }
        
        /* Mixed arithmetic operations to create dependencies */
        d = d * 3 + (a % 17);
        d = d / 2 + (b % 13);
        d = d - (c % 11);
        d = d ^ (i << 3);
        
        /* Bitwise operations with shifts */
        a = (a << 3) | (a >> 61);
        b = (b >> 7) ^ (b << 57);
        c = (c & 0xAAAAAAAAAAAAAAAA) | (c & 0x5555555555555555);
        
        /* Artificial memory barrier with inline asm */
        asm volatile("" ::: "memory");
        
        /* Loop-carried dependency */
        c = c + d;
        
        /* Conditional break based on computed value */
        if ((c & 0xFFF) == 0xABC) {
            d = d * 2;
            break;
        }
    }
    
    /* Use volatile to ensure computation isn't optimized away */
    sink = a + b + c + d;
    return sink;
}

/* Another function with nested loops and switch statement */
int nested_operations(int start, int count) {
    volatile int result = 0;
    int temp = start;
    
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < 10; j++) {
            /* Switch with multiple cases for control flow complexity */
            switch ((i + j) % 6) {
                case 0:
                    temp = temp * 3 + j;
                    result += temp & 1;
                    break;
                case 1:
                    temp = temp / 2 - i;
                    result += temp & 2;
                    break;
                case 2:
                    temp = temp ^ (i * j);
                    result += temp & 4;
                    break;
                case 3:
                    temp = temp | (0xFF << (j % 8));
                    result += temp & 8;
                    break;
                case 4:
                    temp = temp & ~(0x7F >> (i % 7));
                    result += temp & 16;
                    break;
                case 5:
                    temp = temp + (i << j);
                    result += temp & 32;
                    break;
            }
            
            /* More arithmetic with different data types */
            long ltemp = (long)temp * 16807L;
            unsigned int utemp = (unsigned int)ltemp % 2147483647U;
            temp = (int)(utemp ^ (utemp >> 16));
            
            /* Another memory barrier */
            asm volatile("" ::: "memory");
        }
        
        /* Conditional with modulo operation */
        if (i % 100 == 0) {
            result = result ^ (temp << 3);
        }
    }
    
    return result;
}

/* Main function that calls the complex functions */
int main(int argc, char *argv[]) {
    unsigned int seed = 42;
    int iterations = 1000;
    
    /* Use command line arguments to prevent compile-time computation */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    /* Call complex functions multiple times */
    unsigned long total = 0;
    for (int i = 0; i < 5; i++) {
        total += complex_loop(seed + i, iterations / 5);
    }
    
    int nested_result = nested_operations(seed, iterations / 10);
    
    /* Mix results to create final output */
    unsigned long final_result = total ^ (nested_result * 0x5A827999UL);
    
    printf("Result: %lu\n", final_result);
    
    return (int)(final_result & 0x7FFFFFFF);
}

/* test_sel_sched.c - Test case for GCC selective scheduling debug coverage */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and operations */
unsigned long complex_loop(unsigned int seed, int iterations) {
    volatile unsigned long state = seed;  /* Prevent optimization */
    unsigned long result = 0;
    int i, j;
    
    for (i = 0; i < iterations; i++) {
        /* Multiple arithmetic operations creating dependencies */
        state = state * 1103515245 + 12345;
        unsigned long temp = state;
        
        /* Bitwise operations mixed with arithmetic */
        temp = (temp >> 16) & 0x7FFF;
        temp = temp * 16807 % 2147483647;
        
        /* Data-dependent branching */
        if (temp % 3 == 0) {
            result += temp * 3;
            /* Nested loop with break condition */
            for (j = 0; j < 5; j++) {
                if (j > temp % 5) break;
                result ^= (temp << j);
            }
        } else if (temp % 5 == 0) {
            result -= temp / 2;
            /* More operations with different types */
            result |= (unsigned long)(temp * 0x9E3779B9);
        } else {
            result = (result * 1664525 + 1013904223) ^ temp;
        }
        
        /* Switch statement for additional control flow */
        switch (temp % 4) {
            case 0:
                result = result << 1;
                break;
            case 1:
                result = result >> 1;
                break;
            case 2:
                result = result + (temp & 0xFF);
                break;
            case 3:
                result = result - (temp % 256);
                break;
        }
        
        /* Memory barrier via inline asm */
        asm volatile("" ::: "memory");
        
        /* Division/modulo operations (expensive, creates longer latency) */
        if (state % 7 == 0) {
            result = result / 3;
        } else {
            result = result % 0x7FFFFFFF;
        }
    }
    
    return result;
}

/* Another function with different pattern to increase scheduling complexity */
int mixed_operations(int a, int b, int c) {
    int x = a * b;
    int y = b + c;
    int z = c - a;
    
    volatile int barrier = 0;  /* Prevent reordering */
    
    /* Complex expression with multiple dependencies */
    int r = (x * y) / (z + 1);
    r = r ^ (a << 3);
    r = r | (b >> 2);
    r = r & 0x0F0F0F0F;
    
    /* Conditional with side effect */
    if (r > 1000) {
        r = r % 1000;
        asm volatile("" ::: "memory");
    } else {
        r = r * 2 + 1;
    }
    
    return r;
}

/* Main function that provides varying inputs */
int main(int argc, char *argv[]) {
    unsigned long total = 0;
    int i;
    
    /* Use command line argument or default */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    
    /* Call complex function multiple times with different seeds */
    for (i = 0; i < 10; i++) {
        total += complex_loop(i * 137, iterations);
        
        /* Call mixed operations to create different scheduling patterns */
        int mix = mixed_operations(i, i * 2, i * 3);
        total += mix;
        
        /* Additional volatile write to prevent dead code elimination */
        volatile unsigned long sink = total;
        (void)sink;
    }
    
    printf("Result: %lu\n", total);
    return (int)(total % 256);
}

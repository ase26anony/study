/* test_sel_sched.c - Test case for GCC selective scheduling debug coverage */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and multiple operations */
static unsigned long complex_loop(unsigned int seed, int iterations) {
    volatile unsigned long state = seed;  /* Prevent optimization */
    unsigned long result = 0;
    int i, j;
    
    /* Outer loop with loop-carried dependency */
    for (i = 0; i < iterations; i++) {
        /* Multiple arithmetic operations creating dependencies */
        state = (state * 1103515245 + 12345) & 0x7fffffff;
        
        /* Data-dependent branching */
        if (state % 3 == 0) {
            result += (state >> 16) & 0xFF;
        } else if (state % 5 == 0) {
            result ^= (state << 8) | (state >> 24);
        } else {
            result |= state & 0xABCDEF;
        }
        
        /* Nested loop with break condition */
        for (j = 0; j < 3; j++) {
            unsigned long temp = state ^ (j * 0x1234);
            temp = (temp * 1664525 + 1013904223) & 0xFFFFFFFF;
            
            /* Mixed operations on different data types */
            int small = (int)(temp & 0xFFFF);
            unsigned long large = temp >> 16;
            
            if (small % 7 == 0) {
                result += large / (small + 1);
            } else {
                result -= large % (small | 1);
            }
            
            /* Conditional break based on computed value */
            if (temp > 0x80000000) {
                result ^= 0xDEADBEEF;
                break;
            }
        }
        
        /* Bitwise operations */
        result = (result << 3) | (result >> 29);
        result ^= ~state;
        
        /* Memory barrier using inline asm */
        asm volatile("" ::: "memory");
    }
    
    return result;
}

/* Another function with switch statement for control flow complexity */
static int switch_based_computation(int x, int y) {
    int result = 0;
    volatile int counter = x;  /* Prevent optimization */
    
    while (counter < y) {
        switch (counter % 8) {
            case 0: result += x * y; break;
            case 1: result -= x / (y ? y : 1); break;
            case 2: result ^= (x << 4) | (y >> 4); break;
            case 3: result |= x & y; break;
            case 4: result = (result * 3) / 2; break;
            case 5: result = result % 0x1000 + 1; break;
            case 6: result = ~result; break;
            case 7: result = result >> 1; break;
        }
        
        /* Mixed floating point to engage different execution units */
        float f = (float)result / 1000.0f;
        result += (int)(f * 100.0f);
        
        counter++;
        
        /* Artificial dependency chain */
        asm volatile("" : "+r"(result) : : "memory");
    }
    
    return result;
}

/* Main driver that prevents compile-time computation */
int main(int argc, char **argv) {
    unsigned long total = 0;
    int i;
    
    /* Use command line arguments to prevent constant folding */
    int base_iterations = (argc > 1) ? atoi(argv[1]) : 100;
    int seed = (argc > 2) ? atoi(argv[2]) : 42;
    
    /* Call complex functions multiple times */
    for (i = 0; i < 5; i++) {
        total += complex_loop(seed + i, base_iterations + i * 10);
        total += switch_based_computation(i * 10, base_iterations);
        
        /* Additional computation to increase scheduling opportunities */
        total = (total * 6364136223846793005UL) + 1442695040888963407UL;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %lu\n", total);
    
    return (int)(total % 256);
}

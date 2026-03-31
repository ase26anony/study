/* test_sel_sched_dump.c
 * 
 * This test is designed to trigger selective scheduling debug output
 * in GCC's sel-sched-dump.cc, specifically lines that dump instruction
 * RTL representations during scheduling.
 *
 * Compile with instrumented GCC using flags like:
 *   gcc -O3 -fselective-scheduling2 -fsel-sched-dump -c test_sel_sched_dump.c -o test.o
 *   gcc -O3 -fselective-scheduling2 -fdump-rtl-sched1 -fdump-rtl-sched2 -c test_sel_sched_dump.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and multiple operations
 * to create interesting scheduling opportunities */
static unsigned long complex_loop(unsigned iterations, unsigned seed) {
    volatile unsigned sink; /* Prevent dead code elimination */
    unsigned long state = seed;
    unsigned i;
    
    for (i = 0; i < iterations; i++) {
        /* Create loop-carried dependency */
        unsigned long prev = state;
        
        /* Multiple arithmetic operations creating data dependencies */
        state = state * 1103515245 + 12345;
        
        /* Bitwise operations mixed with arithmetic */
        state = (state ^ (state >> 16)) * 0x45d9f3b;
        state = (state ^ (state >> 16)) * 0x45d9f3b;
        state = state ^ (state >> 16);
        
        /* Conditional operations based on data values */
        if (state % 3 == 0) {
            state = state + (prev << 2);
        } else if (state % 5 == 0) {
            state = state ^ (prev >> 3);
        } else {
            state = state | (prev & 0xFF00FF);
        }
        
        /* More operations with different data types */
        int temp = (int)(state & 0xFFFF);
        state = state + (unsigned long)(temp * temp);
        
        /* Artificial memory barrier using inline asm */
        asm volatile("" ::: "memory");
        
        /* Volatile write to prevent optimization */
        sink = (unsigned)(state & 0xFFFFFFFF);
        
        /* Nested conditional with early exit possibility */
        if (state > 0x80000000UL && i > iterations/2) {
            state = state % 0x7FFFFFFF;
            if (state < 1000) {
                /* Early break with data-dependent condition */
                break;
            }
        }
    }
    
    return state;
}

/* Another function with different control flow pattern */
static unsigned hash_array(const unsigned *data, unsigned len) {
    unsigned hash = 0xDEADBEEF;
    unsigned i, j;
    
    for (i = 0; i < len; i++) {
        /* Switch statement creates different basic blocks */
        switch (i % 4) {
            case 0:
                hash = hash ^ (data[i] * 0xCC9E2D51);
                hash = (hash << 15) | (hash >> 17);
                break;
            case 1:
                hash = hash + (data[i] ^ 0x1B873593);
                hash = hash * 5 + 0xE6546B64;
                break;
            case 2:
                hash = hash ^ (data[i] >> 2);
                hash = hash * 0x1B873593;
                break;
            case 3:
                hash = hash | (data[i] & 0xF0F0F0F0);
                hash = ~hash;
                break;
        }
        
        /* Nested loop with simple operation */
        for (j = 0; j < 2; j++) {
            hash = hash ^ (j * 0x85EBCA6B);
        }
        
        /* Division/modulo operations are expensive and create dependencies */
        if (hash % 7 == 0) {
            hash = hash / 3;
        } else {
            hash = hash % 0x7FFF;
        }
    }
    
    return hash;
}

/* Main driver that calls the complex functions */
int main(int argc, char **argv) {
    unsigned iterations = 1000;
    unsigned seed = 42;
    
    /* Use command line arguments to prevent compile-time computation */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations > 10000) iterations = 10000;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Starting selective scheduling test with iterations=%u, seed=%u\n", 
           iterations, seed);
    
    /* Call the complex loop function */
    unsigned long result1 = complex_loop(iterations, seed);
    printf("Result from complex_loop: %lu\n", result1);
    
    /* Create and process an array */
    unsigned data[50];
    for (int i = 0; i < 50; i++) {
        data[i] = (i * seed) ^ 0x12345678;
    }
    
    unsigned result2 = hash_array(data, 50);
    printf("Result from hash_array: %u\n", result2);
    
    /* Mix results to produce final output */
    unsigned long final = result1 ^ ((unsigned long)result2 << 32);
    printf("Final result: %lu\n", final);
    
    return (int)(final % 256);
}

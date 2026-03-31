/* test_sel_sched_dump.c
 * 
 * This test is designed to trigger GCC's selective scheduling debug output
 * machinery, specifically targeting the uncovered lines in sel-sched-dump.cc
 * that dump scheduled instruction details.
 *
 * Compile with instrumented GCC using flags like:
 *   gcc -O3 -fselective-scheduling2 -fsel-sched-dump -c test_sel_sched_dump.c -o test.o
 *   gcc -O3 -fselective-scheduling2 -fdump-rtl-sched1 -fdump-rtl-sched2 -c test_sel_sched_dump.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

/* Complex loop with data-dependent branching and mixed operations */
static unsigned long complex_loop(unsigned iterations, unsigned seed) {
    volatile unsigned sink; /* Prevent dead code elimination */
    unsigned long state = seed;
    unsigned long result = 0;
    int i;
    
    /* Loop with multiple arithmetic and bitwise operations */
    for (i = 0; i < iterations; i++) {
        /* Data-dependent branching */
        if (state & 0x1) {
            /* Branch 1: Mix of operations */
            state = (state * 1103515245 + 12345) & 0x7fffffff;
            result += (state >> 16) & 0xFF;
        } else {
            /* Branch 2: Different mix of operations */
            state = (state * 1664525 + 1013904223) & 0x7fffffff;
            result ^= (state >> 8) & 0xFF;
        }
        
        /* More operations outside branches */
        if (i % 7 == 0) {
            result = (result << 3) | (result >> 29); /* Rotate */
        } else if (i % 13 == 0) {
            result = (result * 13) % 9973; /* Modulo operation */
        }
        
        /* Bitwise operations with shifting */
        result = (result ^ (state & 0xFFFF)) + i;
        
        /* Artificial memory barrier to create dependencies */
        asm volatile("" : : : "memory");
    }
    
    sink = result; /* Volatile write to prevent elimination */
    return result;
}

/* Another function with nested loops and switch statement */
static unsigned long nested_loop_computation(unsigned outer, unsigned inner) {
    unsigned long acc = 0;
    unsigned i, j;
    
    for (i = 0; i < outer; i++) {
        unsigned temp = i * 3;
        
        for (j = 0; j < inner; j++) {
            /* Switch with multiple cases */
            switch ((temp + j) % 5) {
                case 0:
                    acc += (temp << j);
                    break;
                case 1:
                    acc ^= (temp >> (j % 16));
                    break;
                case 2:
                    acc = (acc * 17) + j;
                    break;
                case 3:
                    acc = (acc & 0xAAAAAAAA) | (temp & 0x55555555);
                    break;
                case 4:
                    acc = (acc % 1023) + temp;
                    break;
            }
            
            /* Break condition based on computed value */
            if (acc > 1000000) {
                acc %= 1000000;
                if (j > inner / 2) break;
            }
        }
        
        /* Mix data types */
        long long big_val = (long long)acc * (long long)i;
        acc = (unsigned long)(big_val & 0xFFFFFFFF);
    }
    
    return acc;
}

/* Function with pointer arithmetic and memory operations */
static unsigned long memory_intensive(unsigned size, int* data) {
    unsigned long sum = 0;
    unsigned i;
    
    for (i = 0; i < size; i++) {
        /* Complex addressing with multiple operations */
        int idx = (i * 37) % size;
        int val = data[idx];
        
        /* Conditional update with arithmetic */
        if (val > 0) {
            sum += val * 3;
        } else {
            sum -= (-val) / 2;
        }
        
        /* More operations to increase instruction count */
        sum = (sum << 1) | (sum >> 31);
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
    }
    
    return sum;
}

int main(int argc, char** argv) {
    unsigned iterations = 1000;
    unsigned seed = 123456;
    
    /* Use command line arguments to prevent compile-time computation */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations > 10000) iterations = 10000;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    printf("Starting selective scheduling test...\n");
    
    /* Call all computational functions to ensure they're compiled */
    unsigned long result1 = complex_loop(iterations, seed);
    printf("Result 1: %lu\n", result1);
    
    unsigned long result2 = nested_loop_computation(iterations / 10, 50);
    printf("Result 2: %lu\n", result2);
    
    /* Allocate and initialize array for memory-intensive function */
    int* data = (int*)malloc(sizeof(int) * 1000);
    unsigned i;
    for (i = 0; i < 1000; i++) {
        data[i] = (i * 3 - 1500) ^ seed;
    }
    
    unsigned long result3 = memory_intensive(1000, data);
    printf("Result 3: %lu\n", result3);
    
    free(data);
    
    /* Final result depends on all computations */
    unsigned long final_result = result1 ^ result2 ^ result3;
    printf("Final result: %lu\n", final_result);
    
    return (int)(final_result % 256);
}

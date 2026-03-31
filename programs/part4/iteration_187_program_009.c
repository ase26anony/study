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

/* A non-trivial function with complex control flow and data dependencies
 * to engage the selective scheduler and its debug output routines.
 */
unsigned long complex_loop(unsigned int seed, int iterations) {
    volatile unsigned int v_seed = seed; /* volatile to prevent optimization */
    unsigned long state = v_seed;
    unsigned long hash = 0;
    int i, j;
    
    /* Outer loop with loop-carried dependency */
    for (i = 0; i < iterations; i++) {
        /* Mix arithmetic and bitwise operations to create many instructions */
        state = (state * 1103515245 + 12345) & 0x7fffffff;
        
        /* Data-dependent branching */
        if (state % 3 == 0) {
            hash += (state << 2) | (hash >> 5);
        } else if (state % 5 == 0) {
            hash ^= (state * 16807) % 2147483647;
        } else {
            hash = (hash + state) * 6364136223846793005ULL;
        }
        
        /* Nested loop with different operation types */
        for (j = 0; j < 3; j++) {
            /* Mixed-type operations inhibit simplifications */
            unsigned short temp = (hash >> (j * 8)) & 0xFF;
            if (temp % 2 == 0) {
                hash += temp * 17;
            } else {
                hash -= temp * 23;
            }
            
            /* Artificial memory barrier using inline asm */
            asm volatile("" ::: "memory");
        }
        
        /* More arithmetic with division/modulo (expensive operations) */
        if (hash % 7 == 0) {
            hash = hash / 3;
        } else {
            hash = hash % 1000000007;
        }
        
        /* Bitwise rotation simulation */
        hash = (hash << 13) | (hash >> (64 - 13));
    }
    
    return hash;
}

/* Another function with different patterns to increase scheduling complexity */
int branching_pattern(int start, int limit) {
    int result = start;
    int i;
    
    for (i = 0; i < limit; i++) {
        /* Switch-like branching */
        switch (i % 4) {
            case 0:
                result += i * 2;
                break;
            case 1:
                result ^= i * 3;
                /* Fall through */
            case 2:
                result |= i * 5;
                break;
            default:
                result &= ~i;
                break;
        }
        
        /* Conditional break based on computed value */
        if (result > 1000000) {
            result /= 2;
            if (result < 1000) break;
        }
        
        /* More operations with volatile to force dependencies */
        volatile int barrier = result;
        result = barrier + i;
    }
    
    return result;
}

/* Main function that provides non-constant inputs and uses results */
int main(int argc, char **argv) {
    unsigned int seed;
    int iterations;
    
    /* Use command line arguments to prevent compile-time computation */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        seed = 12345;
    }
    
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations < 10) iterations = 10;
        if (iterations > 1000) iterations = 1000;
    } else {
        iterations = 100;
    }
    
    /* Call the complex functions multiple times with different parameters */
    unsigned long hash1 = complex_loop(seed, iterations);
    unsigned long hash2 = complex_loop(hash1 & 0xFFFFFFFF, iterations / 2);
    
    int pattern_result1 = branching_pattern(hash1 & 0xFFFF, iterations);
    int pattern_result2 = branching_pattern(pattern_result1, iterations / 3);
    
    /* Combine results in a non-trivial way */
    unsigned long final_result = hash1 ^ hash2;
    final_result += (unsigned long)pattern_result1 * pattern_result2;
    
    /* Ensure result is used (prevents dead code elimination) */
    printf("Result: %lu\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}

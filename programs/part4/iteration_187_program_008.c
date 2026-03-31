/* test_sel_sched_dump.c
 * 
 * This test is designed to trigger GCC's selective scheduling debug output
 * machinery, specifically targeting the uncovered lines in sel-sched-dump.cc
 * that dump instruction RTL representations during scheduling.
 *
 * Compile with instrumented GCC using flags like:
 *   gcc -O3 -fselective-scheduling2 -fsel-sched-dump -c test_sel_sched_dump.c -o test.o
 *   gcc -O3 -fselective-scheduling2 -fdump-rtl-sched1 -fdump-rtl-sched2 -c test_sel_sched_dump.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile sink to prevent dead code elimination */
volatile int global_sink = 0;

/* Function with complex loop to engage selective scheduler */
unsigned int complex_hash_loop(unsigned int seed, int iterations) {
    unsigned int a = seed;
    unsigned int b = 0x9e3779b9; /* golden ratio */
    unsigned int c = 0xdeadbeef;
    unsigned int d = 0xcafebabe;
    int i;
    
    /* Complex loop with data-dependent branching and multiple operations */
    for (i = 0; i < iterations; i++) {
        /* Mix operations to create various dependencies */
        unsigned int old_a = a;
        
        /* Multiple arithmetic operations */
        a = a * 1664525 + 1013904223;
        b = b ^ (b >> 13);
        c = c + (c << 3);
        d = d - (d >> 7);
        
        /* Bitwise operations mixing variables */
        a = a ^ (b << 1);
        b = b + (c ^ d);
        c = c ^ (a >> 3);
        d = d | (b & c);
        
        /* Conditional operations based on loop index and computed values */
        if (i % 3 == 0) {
            a = a + (d << 2);
            /* Inline asm to create memory barrier and prevent reordering */
            asm volatile("" ::: "memory");
        } else if (i % 5 == 0) {
            b = b * 1103515245 + 12345;
            c = c ^ (a * b);
        } else {
            d = (d * 48271) % 2147483647;
        }
        
        /* More operations with different data types */
        if ((a & 0xFF) > 128) {
            short temp = (short)(a & 0xFFFF);
            b = b + (unsigned int)temp * 17;
        }
        
        /* Nested conditional with bitwise operations */
        switch (i % 4) {
            case 0: a = a ^ 0xAAAAAAAA; break;
            case 1: b = b | 0x55555555; break;
            case 2: c = c & 0x33333333; break;
            case 3: d = d ^ c ^ b ^ a; break;
        }
        
        /* Loop-carried dependency */
        a = a + old_a;
        
        /* Early exit condition based on computed value */
        if ((a & 0xFFF) == 0 && i > iterations/2) {
            /* Break creates more complex CFG for scheduler */
            break;
        }
    }
    
    /* Final mixing */
    a = a ^ b ^ c ^ d;
    return a;
}

/* Another function with different pattern to increase scheduling complexity */
int nested_loop_computation(int start, int limit) {
    int i, j;
    int result = start;
    
    for (i = 0; i < limit; i++) {
        int inner_result = 0;
        
        /* Nested loop with varying bounds */
        for (j = 0; j < (i % 8) + 2; j++) {
            /* Mix of operations */
            inner_result += (i * j) % 256;
            inner_result = inner_result ^ (i << (j % 4));
            inner_result = (inner_result * 6364136223846793005ULL) >> 32;
        }
        
        /* Conditional update */
        if (inner_result > 100) {
            result += inner_result;
        } else {
            result -= (inner_result * 3) / 2;
        }
        
        /* Modulo operation creates dependency chain */
        result = result % 1000000007;
        
        /* Memory barrier to force scheduler to work harder */
        asm volatile("" ::: "memory");
    }
    
    return result;
}

/* Main driver that uses volatile and external input to prevent optimization */
int main(int argc, char *argv[]) {
    unsigned int hash_result = 0;
    int compute_result = 0;
    int iterations = 1000;
    
    /* Use command line argument to prevent compile-time computation */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 100) iterations = 100;
        if (iterations > 10000) iterations = 10000;
    }
    
    /* Call complex functions multiple times with different parameters */
    for (int i = 0; i < 5; i++) {
        hash_result ^= complex_hash_loop(hash_result + i * 123, iterations + i * 50);
        
        /* Feed result into next computation */
        compute_result += nested_loop_computation(hash_result & 0xFFF, 50 + (i * 10));
        
        /* Use volatile write to prevent elimination */
        global_sink = hash_result + compute_result;
    }
    
    /* Final mixing and output */
    unsigned int final_result = hash_result ^ compute_result;
    printf("Result: %u (sink: %d)\n", final_result, global_sink);
    
    return (int)(final_result % 256);
}

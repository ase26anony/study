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

/* Complex loop with data-dependent branching and multiple operations */
unsigned int compute_hash(unsigned int seed, int iterations) {
    volatile unsigned int state = seed; /* volatile to prevent optimization */
    unsigned int a, b, c;
    int i;
    
    /* Initialize with some values */
    a = state ^ 0xDEADBEEF;
    b = state * 1664525U + 1013904223U;
    c = 0;
    
    /* Main computational loop with control flow and mixed operations */
    for (i = 0; i < iterations; i++) {
        /* Data-dependent branching */
        if (i % 7 == 0) {
            a = (a << 3) | (a >> 29);  /* rotate left */
            b = b ^ (b << 13);
            c = a + b;
        } else if (i % 5 == 0) {
            a = a * 1103515245U + 12345;
            b = (b & 0x55555555) | (c & 0xAAAAAAAA);
            c = c % 9973;  /* prime modulus operation */
        } else if (i % 3 == 0) {
            a = a ^ c;
            b = b * 3 + 1;  /* Collatz-like step */
            c = c >> 1;
        } else {
            a = a + b + c;
            b = b - a;
            c = c ^ (a * b);
        }
        
        /* More arithmetic operations */
        a = (a + i) * 0x9E3779B9U;
        b = (b ^ 0x85EBCA6B) + (c << 7);
        c = (c * 0x1B873593) | (a & b);
        
        /* Artificial memory barrier to create dependencies */
        asm volatile("" : "+r" (a), "+r" (b), "+r" (c) : : "memory");
        
        /* Conditional break based on computed value */
        if ((a & 0xFFF) == 0x123) {
            c = c ^ 0xFFFFFFFF;
            break;
        }
    }
    
    /* Final mixing */
    a = a ^ (a >> 16);
    b = b * 0x5BD1E995;
    c = c + (c << 3) + (c >> 5);
    
    return a ^ b ^ c;
}

/* Another function with different pattern to increase scheduling complexity */
int process_array(int *arr, int n) {
    int sum = 0;
    int prod = 1;
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Nested loop for additional complexity */
        for (j = 0; j < 3; j++) {
            int val = arr[i] + j;
            
            /* Switch statement for control flow variety */
            switch (val % 4) {
                case 0:
                    sum += val * 2;
                    prod *= (val & 0xF);
                    break;
                case 1:
                    sum -= val / 2;
                    prod |= val;
                    break;
                case 2:
                    sum ^= val;
                    prod = prod % 256;
                    break;
                default:
                    sum = sum << 1;
                    prod = prod >> 1;
                    break;
            }
            
            /* Bitwise operations mixed with arithmetic */
            val = (val << (i % 8)) | (val >> (8 - (i % 8)));
            sum = sum + (val & 0xFF);
            
            /* Prevent loop unrolling from simplifying too much */
            asm volatile("" : "+r" (sum), "+r" (prod) : : "memory");
        }
        
        /* Loop-carried dependency */
        arr[i] = sum ^ prod;
    }
    
    return sum + prod;
}

/* Main driver that provides varying inputs */
int main(int argc, char **argv) {
    int i;
    unsigned int hash_result = 0;
    int array[100];
    int array_result;
    
    /* Initialize array with pseudo-random values */
    for (i = 0; i < 100; i++) {
        array[i] = (i * 1103515245U + 12345) & 0x7FFFFFFF;
    }
    
    /* Vary the iteration count based on argument to prevent constant folding */
    int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 10000) iterations = 10000;
    }
    
    /* Call the complex functions multiple times with different seeds */
    for (i = 0; i < 5; i++) {
        hash_result ^= compute_hash(i * 0x1234567, iterations + i * 100);
    }
    
    /* Process the array */
    array_result = process_array(array, 100);
    
    /* Use results to prevent dead code elimination */
    printf("Hash result: 0x%08X\n", hash_result);
    printf("Array result: %d\n", array_result);
    printf("Final combined: %lu\n", 
           (unsigned long)hash_result * (unsigned long)array_result);
    
    return (hash_result + array_result) & 0xFF;
}

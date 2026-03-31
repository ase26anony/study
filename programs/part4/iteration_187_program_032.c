/* test_sel_sched.c - Test case for GCC selective scheduling debug coverage */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and operations */
unsigned int compute_hash(unsigned int seed, int iterations) {
    volatile unsigned int sink; /* Prevent dead code elimination */
    unsigned int hash = seed;
    int i, j;
    
    /* Outer loop with loop-carried dependency */
    for (i = 0; i < iterations; i++) {
        /* Multiple arithmetic operations creating dependencies */
        hash = (hash * 1103515245U + 12345U) & 0x7fffffffU;
        
        /* Data-dependent branching */
        if (hash % 3 == 0) {
            /* Bitwise operations */
            hash ^= (hash << 13);
            hash ^= (hash >> 17);
            hash ^= (hash << 5);
        } else if (hash % 5 == 0) {
            /* Different arithmetic mix */
            hash = (hash * 1664525U + 1013904223U);
            hash = (hash % 997) + (hash / 17);
        } else {
            /* Mixed operations */
            hash = ((hash & 0x55555555) << 1) | ((hash & 0xAAAAAAAA) >> 1);
            hash = hash * 3 + 1;
        }
        
        /* Nested loop with break condition */
        for (j = 0; j < 3; j++) {
            if ((hash >> (j * 8)) & 0x80) {
                hash += j * 0x01010101;
                if (j == 2) break;
            }
        }
        
        /* Switch statement for control flow complexity */
        switch (hash % 4) {
            case 0: hash = hash * 2; break;
            case 1: hash = hash / 3; break;
            case 2: hash = hash | 0xF0F0F0F0; break;
            case 3: hash = hash & 0x0F0F0F0F; break;
        }
        
        /* Memory barrier via inline asm */
        asm volatile("" ::: "memory");
    }
    
    /* Volatile write to ensure computation isn't eliminated */
    sink = hash;
    return hash;
}

/* Another function with different pattern */
long process_array(int *arr, int len) {
    volatile long result = 0;
    int i;
    
    for (i = 0; i < len; i++) {
        int val = arr[i];
        
        /* Complex conditional operations */
        if (val > 0) {
            result += val * val;
            if (val % 2 == 0) {
                result -= val / 2;
            } else {
                result |= (val << 3);
            }
        } else if (val < 0) {
            result -= (-val) * (-val);
            result = (result << 1) | (result >> 31);
        } else {
            result ^= 0x12345678;
        }
        
        /* More operations with different data types */
        unsigned short us = (unsigned short)val;
        result += us * 257;  /* Mix 8-bit and 16-bit */
        
        /* Prevent loop unrolling from simplifying too much */
        if (i % 7 == 0) {
            asm volatile("" : "+r"(result) : : "memory");
        }
    }
    
    return result;
}

/* Main function with input-dependent execution */
int main(int argc, char **argv) {
    int iterations = 1000;
    int array_size = 50;
    int i;
    
    /* Use command line argument to prevent compile-time computation */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 10000) iterations = 10000;
    }
    
    /* Initialize array with pseudo-random values */
    int *array = (int*)malloc(array_size * sizeof(int));
    if (!array) return 1;
    
    unsigned int seed = 123456789;
    for (i = 0; i < array_size; i++) {
        seed = seed * 1103515245U + 12345U;
        array[i] = (int)(seed % 1000) - 500;
    }
    
    /* Call both complex functions */
    unsigned int hash_result = compute_hash(seed, iterations);
    long array_result = process_array(array, array_size);
    
    /* Combine results in a non-trivial way */
    long final_result = (long)hash_result * array_result;
    final_result = (final_result % 1000000) + (final_result / 1000000);
    
    /* Output result to prevent elimination */
    printf("Result: %ld (hash: %u, array: %ld)\n", 
           final_result, hash_result, array_result);
    
    free(array);
    return 0;
}

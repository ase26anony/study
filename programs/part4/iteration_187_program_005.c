/* test_sel_sched_dump.c
 * 
 * This test is designed to trigger GCC's selective scheduling debug dump
 * functionality, specifically covering lines in sel-sched-dump.cc that
 * print scheduled instruction details.
 *
 * Compile with instrumented GCC using flags like:
 *   gcc -O3 -fselective-scheduling2 -fsel-sched-dump -fdump-rtl-all -c test_sel_sched_dump.c -o test.o
 *
 * The actual execution of the compiled program is secondary; coverage
 * occurs during compilation when the instrumented GCC writes debug dumps.
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile sink to prevent dead code elimination */
static volatile int global_sink = 0;

/* Function with complex control flow and data dependencies
 * to engage the selective scheduler */
int compute_hash(int seed, int iterations) {
    int hash = seed;
    int i, j;
    
    /* Outer loop with loop-carried dependency */
    for (i = 0; i < iterations; i++) {
        int temp = hash;
        
        /* Inner loop with data-dependent branching */
        for (j = 0; j < 5; j++) {
            /* Multiple arithmetic operations creating dependencies */
            temp = temp * 1103515245 + 12345;
            
            /* Bitwise operations mixed with arithmetic */
            temp = (temp ^ (temp >> 16)) & 0x7FFF;
            
            /* Conditional operations based on computed values */
            if (temp % 3 == 0) {
                temp = temp + (temp << 2);
            } else if (temp % 7 == 0) {
                temp = temp | 0x5555;
            } else {
                temp = temp ^ 0xAAAA;
            }
            
            /* More operations to increase instruction count */
            temp = (temp * 13) % 0x10000;
            temp = temp & ~(1 << (j % 15));
        }
        
        /* Mix inner loop result with hash */
        hash = hash ^ temp;
        
        /* Complex conditional with multiple operations */
        if (hash % 2 == 0) {
            hash = (hash >> 1) | ((hash & 1) << 31);
        } else {
            hash = (hash * 3) % 0x7FFFFFFF;
        }
        
        /* Memory barrier via inline asm to create scheduling constraints */
        asm volatile("" ::: "memory");
        
        /* Additional branching based on loop index */
        switch (i % 4) {
            case 0:
                hash = hash + i;
                break;
            case 1:
                hash = hash - (i * 2);
                break;
            case 2:
                hash = hash ^ (i << 3);
                break;
            case 3:
                hash = hash | 0xFF;
                break;
        }
    }
    
    return hash;
}

/* Another function with different pattern to increase scheduling complexity */
int process_array(int *arr, int size) {
    int sum = 0;
    int i;
    
    for (i = 0; i < size; i++) {
        volatile int load = arr[i];  /* Volatile load creates memory dependency */
        int val = load;
        
        /* Data-dependent computation chain */
        if (val > 0) {
            val = val * 2 - 1;
        } else {
            val = (val + 1000) * 3;
        }
        
        /* Nested conditional */
        if (i % 2 == 0) {
            for (int k = 0; k < 3; k++) {
                val = (val << k) | (val >> (32 - k));
            }
        } else {
            val = val % 997;
        }
        
        sum += val;
        
        /* Early exit condition that's data-dependent */
        if (sum > 1000000) {
            sum = sum / 2;
        }
    }
    
    return sum;
}

/* Main function that provides varying inputs and uses results */
int main(int argc, char **argv) {
    int iterations = 100;
    int array_size = 50;
    int i, result;
    
    /* Use command line argument to vary input, preventing compile-time computation */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 1000) iterations = 1000;
    }
    
    /* Initialize array with pseudo-random values */
    int *array = (int *)malloc(array_size * sizeof(int));
    for (i = 0; i < array_size; i++) {
        array[i] = (i * 37 + 123) % 1000;
    }
    
    /* Call both complex functions, mixing their results */
    int hash1 = compute_hash(42, iterations);
    int hash2 = compute_hash(hash1, iterations / 2);
    int sum = process_array(array, array_size);
    
    /* Combine results in a non-trivial way */
    result = (hash1 ^ hash2) + sum;
    
    /* Use volatile write to prevent elimination */
    global_sink = result;
    
    /* Also print to ensure side effect */
    printf("Result: %d (global_sink: %d)\n", result, global_sink);
    
    free(array);
    return 0;
}

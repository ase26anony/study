/* sel-sched-test.c
 * Program to trigger selective scheduler debugging output
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -o test sel-sched-test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
static volatile int g_volatile_counter = 0;

/* Function with memory aliasing through restrict and non-restrict pointers */
static inline uint64_t compute_loop(int *restrict arr1, int *arr2, 
                                   float *restrict farr, double *darr,
                                   int start, int end, int seed) {
    uint64_t acc1 = seed;
    float acc2 = start * 0.5f;
    double acc3 = start * 0.25;
    int temp = g_volatile_counter;
    
    /* Hot loop with multiple dependencies and operations */
    for (int i = start; i < end; i++) {
        /* Integer operations with carried dependency */
        acc1 = (acc1 * 1103515245 + 12345) & 0x7fffffff;
        arr1[i] = (int)(acc1 % 1000);
        
        /* Floating-point operations mixing types */
        acc2 = acc2 * 1.1f + farr[i % 16] * 0.9f;
        acc3 = acc3 / 1.01 + darr[i % 16] * 0.99;
        
        /* Memory operation with potential aliasing */
        arr2[i] = arr1[i] + (int)(acc2 * 100);
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            /* Additional operations in conditional path */
            temp += arr1[i] * 3;
            acc3 = acc3 * 2.0 - 1.0;
        } else if (i % 13 == 0) {
            /* Another conditional path */
            temp -= arr2[i] / 2;
            acc2 = acc2 / 1.5f + 0.5f;
        } else {
            /* Default path with different operations */
            temp ^= (arr1[i] | arr2[i]);
            acc1 = acc1 ^ (temp << 3);
        }
        
        /* More arithmetic diversity */
        if (i % 5 == 0) {
            farr[i % 16] = acc2 * 0.8f;
            darr[i % 16] = acc3 * 0.7;
        }
        
        /* Inline assembly with memory clobber to prevent optimization */
        asm volatile("" : : "r"(temp) : "memory");
    }
    
    /* Mix results to create final checksum */
    return (uint64_t)acc1 ^ ((uint64_t)(*(int*)&acc2) << 32) ^ (uint64_t)(*(int64_t*)&acc3);
}

/* Secondary hot function to increase scheduling complexity */
static inline uint64_t process_block(int *restrict buf1, int *buf2,
                                    float *restrict fbuf, double *dbuf,
                                    int size, int iter) {
    uint64_t total = 0;
    
    for (int j = 0; j < iter; j++) {
        /* Call compute_loop multiple times with different parameters */
        uint64_t res = compute_loop(buf1, buf2, fbuf, dbuf, 
                                   j % 4, size - (j % 3), j);
        
        /* Additional computation on result */
        total = total * 6364136223846793005ULL + res + j;
        
        /* Memory store that might alias */
        if (j % 11 == 0) {
            buf2[size - 1] = (int)(total & 0xFFFFFFFF);
        }
    }
    
    return total;
}

int main(void) {
    const int SIZE = 256;
    const int ITERS = 1000;
    
    /* Allocate and initialize arrays with different patterns */
    int *arr1 = (int*)aligned_alloc(64, SIZE * sizeof(int));
    int *arr2 = (int*)aligned_alloc(64, SIZE * sizeof(int));
    float *farr = (float*)aligned_alloc(64, 16 * sizeof(float));
    double *darr = (double*)aligned_alloc(64, 16 * sizeof(double));
    
    if (!arr1 || !arr2 || !farr || !darr) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (i * 13 + 7) % 97;
        arr2[i] = (i * 17 + 11) % 101;
    }
    
    for (int i = 0; i < 16; i++) {
        farr[i] = (i * 0.3f + 0.7f) * (i % 3 + 1);
        darr[i] = (i * 0.7 + 0.3) * (i % 5 + 1);
    }
    
    /* Perform computation in a way that creates hot loops */
    uint64_t checksum = 0;
    
    for (int phase = 0; phase < 3; phase++) {
        /* Vary parameters to avoid pattern recognition */
        int start_offset = phase * 10;
        int iter_count = ITERS + phase * 100;
        
        checksum ^= process_block(arr1 + start_offset, 
                                 arr2 + start_offset,
                                 farr, darr,
                                 SIZE - start_offset * 2,
                                 iter_count);
        
        /* Additional computation between phases */
        for (int i = 0; i < 16; i++) {
            farr[i] = farr[i] * 0.95f + 0.05f;
            darr[i] = darr[i] * 0.98 + 0.02;
        }
    }
    
    /* Final reduction to ensure all computations are used */
    uint64_t final_result = 0;
    for (int i = 0; i < SIZE; i++) {
        final_result = final_result * 31 + arr1[i];
        final_result = final_result * 17 + arr2[i];
    }
    
    for (int i = 0; i < 16; i++) {
        final_result ^= *(uint32_t*)&farr[i];
        final_result ^= *(uint64_t*)&darr[i];
    }
    
    final_result ^= checksum;
    
    /* Print result to prevent dead code elimination */
    printf("Result: 0x%016llx\n", (unsigned long long)final_result);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr);
    free(darr);
    
    return 0;
}

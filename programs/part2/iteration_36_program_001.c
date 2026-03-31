/* sel-sched-coverage.c
 * Designed to trigger selective scheduler debugging output
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-all sel-sched-coverage.c -o sel-sched-coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 100000

/* Force memory dependencies and prevent optimization */
static volatile int force_memory_barrier;

/* Function with memory aliasing - both restrict and non-restrict pointers */
static inline void compute_loop(float *restrict dest, float *src1, float *src2, 
                                int *counter, double *fp_acc, int start, int end) {
    int i;
    float local_acc = 0.0f;
    double double_acc = *fp_acc;
    
    /* Hot loop with multiple dependencies and operations */
    for (i = start; i < end; i++) {
        /* Integer operations with carried dependency */
        *counter += i * 3;
        
        /* Floating-point operations - creates FPU instructions */
        float temp1 = src1[i] * 1.5f;
        float temp2 = src2[i] / 2.0f;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            temp1 += src2[i % 256] * 0.75f;
            *counter ^= (i << 3);
        } else if (i % 13 == 0) {
            temp2 -= src1[i % 128] * 0.25f;
            *counter |= (i >> 2);
        }
        
        /* Memory operations with potential aliasing */
        dest[i] = temp1 + temp2;
        
        /* Mixed integer/float operations */
        local_acc += dest[i];
        double_acc += (double)dest[i] * 0.5;
        
        /* Additional arithmetic diversity */
        if (i % 5 == 0) {
            double_acc /= 1.1;
            *counter *= 2;
        }
        
        /* Force memory barrier to prevent reordering */
        if (force_memory_barrier) {
            asm volatile("" ::: "memory");
        }
    }
    
    /* Store results back */
    *fp_acc = double_acc;
    
    /* Additional computation to extend basic block */
    for (int j = 0; j < 4; j++) {
        local_acc *= 1.01f;
    }
    
    /* Final store with memory clobber */
    asm volatile("" : "+m" (*dest) : : "memory");
}

/* Wrapper function to encourage inlining */
static inline uint64_t process_data(float *restrict arr1, float *arr2, float *arr3) {
    int counter = 0;
    double fp_accumulator = 0.0;
    uint64_t checksum = 0;
    
    /* Call compute_loop multiple times with different parameters */
    compute_loop(arr1, arr2, arr3, &counter, &fp_accumulator, 0, SIZE/2);
    compute_loop(arr1 + SIZE/2, arr2 + SIZE/2, arr3 + SIZE/2, 
                 &counter, &fp_accumulator, 0, SIZE/2);
    
    /* Additional loop with different stride */
    for (int i = 0; i < SIZE; i += 8) {
        compute_loop(arr1 + i, arr2 + i, arr3 + i, 
                     &counter, &fp_accumulator, 0, 8);
    }
    
    /* Create checksum from results */
    for (int i = 0; i < SIZE; i++) {
        checksum ^= *(uint32_t*)&arr1[i];
        checksum += (uint64_t)counter * i;
    }
    
    checksum ^= *(uint64_t*)&fp_accumulator;
    return checksum;
}

int main(void) {
    /* Allocate and initialize arrays */
    float *array1 = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *array2 = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *array3 = (float*)aligned_alloc(64, SIZE * sizeof(float));
    
    if (!array1 || !array2 || !array3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern to create dependencies */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (float)(i % 97) * 0.3f;
        array2[i] = (float)(i % 113) * 0.7f;
        array3[i] = (float)(i % 71) * 1.3f;
    }
    
    uint64_t final_checksum = 0;
    
    /* Multiple iterations to create hot loop */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Vary the force_memory_barrier to create different scheduling scenarios */
        force_memory_barrier = (iter % 100 == 0);
        
        /* Process data - this should trigger selective scheduling */
        uint64_t iter_checksum = process_data(array1, array2, array3);
        
        /* Mix results to prevent dead code elimination */
        final_checksum ^= iter_checksum;
        final_checksum += iter * 0x5A827999;
        
        /* Modify inputs slightly each iteration */
        array1[iter % SIZE] += 0.1f;
        array2[iter % SIZE] -= 0.05f;
    }
    
    /* Print result to prevent optimization */
    printf("Final checksum: 0x%016llx\n", (unsigned long long)final_checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}

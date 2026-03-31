/* sel-sched-dump-coverage.c
 * Designed to trigger debug_insn_rtx in GCC's selective scheduler
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-all -o test test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 100000

/* Force memory dependencies and prevent optimization */
static volatile int global_seed = 42;

/* Function with potential aliasing */
static inline void compute_loop(int *restrict arr_a, int *arr_b, 
                               float *restrict arr_f, double *arr_d,
                               int start, int end, int modifier) {
    int i;
    int local_sum = global_seed;
    float f_acc = 1.0f;
    double d_acc = 1.0;
    
    /* Hot loop with multiple dependencies and operations */
    for (i = start; i < end; i++) {
        /* Integer operations with carried dependency */
        local_sum = (local_sum * 1103515245 + 12345) & 0x7fffffff;
        
        /* Floating-point operations */
        f_acc = f_acc * 1.01f + (float)local_sum * 0.0001f;
        d_acc = d_acc / 1.001 + (double)local_sum * 0.00001;
        
        /* Memory operations with potential aliasing */
        arr_a[i] = local_sum + modifier;
        arr_b[i % 256] = arr_b[i % 256] + local_sum;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            /* Additional operations in taken branch */
            f_acc = f_acc - (float)(i % 13) * 0.5f;
            arr_f[i % 128] = f_acc;
        } else if (i % 11 == 0) {
            /* Another basic block */
            d_acc = d_acc + (double)(i % 17) * 0.25;
            arr_d[i % 64] = d_acc;
        }
        
        /* More arithmetic diversity */
        if (i % 3 == 0) {
            arr_a[i] = arr_a[i] ^ (local_sum >> 16);
        }
        
        /* Additional floating-point operation */
        f_acc = f_acc + (float)(arr_b[i % 256] & 0xFF) * 0.01f;
        
        /* Complex expression with multiple operations */
        arr_a[i] = arr_a[i] + (int)(f_acc * 100.0f) - (int)(d_acc * 50.0);
    }
    
    /* Store results to prevent dead code elimination */
    arr_a[end-1] = local_sum;
    arr_f[127] = f_acc;
    arr_d[63] = d_acc;
}

/* Another hot function to encourage inlining and scheduling */
static inline void process_chunk(int *restrict a, int *b, 
                                float *restrict f, double *d,
                                int chunk_size, int offset) {
    int chunk_start = offset * chunk_size;
    int chunk_end = chunk_start + chunk_size;
    
    if (chunk_end > SIZE) chunk_end = SIZE;
    
    /* Call the compute loop with different modifiers */
    for (int repeat = 0; repeat < 3; repeat++) {
        compute_loop(a, b, f, d, chunk_start, chunk_end, 
                    offset * 100 + repeat * 10);
    }
}

int main(void) {
    /* Allocate and initialize arrays */
    int *array_a = (int*)malloc(SIZE * sizeof(int));
    int *array_b = (int*)malloc(256 * sizeof(int));
    float *array_f = (float*)malloc(128 * sizeof(float));
    double *array_d = (double*)malloc(64 * sizeof(double));
    
    if (!array_a || !array_b || !array_f || !array_d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = i;
    }
    for (int i = 0; i < 256; i++) {
        array_b[i] = i * 2;
    }
    for (int i = 0; i < 128; i++) {
        array_f[i] = (float)i * 0.5f;
    }
    for (int i = 0; i < 64; i++) {
        array_d[i] = (double)i * 0.25;
    }
    
    /* Perform multiple iterations to create hot loop */
    uint64_t checksum = 0;
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Update global seed to vary computation */
        global_seed = (global_seed * 1664525 + 1013904223) & 0x7fffffff;
        
        /* Process in chunks to create more scheduling opportunities */
        for (int chunk = 0; chunk < 4; chunk++) {
            process_chunk(array_a, array_b, array_f, array_d, 
                         SIZE / 4, chunk);
        }
        
        /* Simple checksum to prevent optimization */
        checksum ^= (uint64_t)array_a[iter % SIZE];
        checksum ^= (uint64_t)array_b[iter % 256] << 16;
        checksum ^= (uint64_t)(array_f[iter % 128] * 1000.0f);
    }
    
    /* Final computation and output to ensure side effects */
    int final_result = 0;
    for (int i = 0; i < SIZE; i++) {
        final_result ^= array_a[i];
    }
    for (int i = 0; i < 256; i++) {
        final_result ^= array_b[i];
    }
    
    printf("Result: %d (checksum: 0x%016llx)\n", 
           final_result, (unsigned long long)checksum);
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_f);
    free(array_d);
    
    return 0;
}

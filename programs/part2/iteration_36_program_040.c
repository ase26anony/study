/* sel-sched-coverage.c
 * Designed to trigger debug_insn_rtx() in GCC's selective scheduler
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-all sel-sched-coverage.c -o sel-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000000

/* Use volatile to prevent optimization of dependencies */
static volatile int global_counter = 0;

/* Function with memory aliasing to create complex dependencies */
static inline void compute_loop(int *restrict arr1, int *arr2, 
                               float *restrict farr, double *darr,
                               int start, int end, int seed) {
    int i;
    int local_sum = seed;
    float f_acc = (float)seed * 0.5f;
    double d_acc = (double)seed * 0.25;
    
    /* Hot loop with multiple dependencies and operations */
    for (i = start; i < end; i++) {
        /* Integer operations with carried dependency */
        local_sum = local_sum * 1103515245 + 12345;
        
        /* Floating-point operations */
        f_acc = f_acc * 1.5f + (float)local_sum * 0.01f;
        d_acc = d_acc * 1.25 + (double)f_acc * 0.02;
        
        /* Memory operations with potential aliasing */
        arr1[i] = local_sum + (int)f_acc;
        arr2[i % 256] = arr2[i % 256] * 3 + local_sum;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            f_acc = f_acc / 2.0f;
            arr1[i] = arr1[i] >> 1;
        } else if (i % 13 == 0) {
            d_acc = d_acc * 0.9;
            arr2[i % 256] = arr2[i % 256] + (int)d_acc;
        }
        
        /* More arithmetic diversity */
        if (i % 17 == 0) {
            f_acc = f_acc - (float)(i * 2);
            d_acc = d_acc + (double)(i % 19);
        }
        
        /* Additional integer operations */
        int temp = arr1[i] ^ arr2[i % 256];
        temp = temp * 6364136223846793005ULL;
        
        /* Mix in global volatile to prevent reordering */
        arr1[i] = temp + global_counter;
        
        /* Complex expression with multiple operations */
        farr[i % 128] = (f_acc * 0.33f) + (float)(temp % 100) * 0.01f;
        darr[i % 64] = d_acc * 0.44 + (double)(arr1[i] % 50) * 0.02;
        
        /* Another conditional with different operations */
        if (i % 23 == 0) {
            farr[i % 128] = farr[i % 128] / 3.14159f;
            darr[i % 64] = darr[i % 64] * 2.71828;
        }
    }
    
    /* Store results to prevent dead code elimination */
    arr1[end-1] = local_sum;
    farr[127] = f_acc;
    darr[63] = d_acc;
}

/* Another inline function to increase scheduling complexity */
static inline void process_chunk(int *restrict a, int *b, 
                                float *restrict f, double *d,
                                int chunk_size, int offset) {
    int i, j;
    
    for (j = 0; j < 4; j++) {
        compute_loop(a + offset, b, f, d, 
                     j * chunk_size / 4, 
                     (j + 1) * chunk_size / 4,
                     offset + j);
        
        /* Additional operations between loop calls */
        for (i = 0; i < chunk_size / 8; i++) {
            int idx = offset + i;
            a[idx] = a[idx] * 3 - b[idx % 256];
            f[i % 128] = f[i % 128] + (float)a[idx] * 0.001f;
            d[i % 64] = d[i % 64] - (double)b[idx % 256] * 0.002;
        }
    }
}

int main(void) {
    /* Allocate arrays with different alignments */
    int *arr1 = (int*)aligned_alloc(64, SIZE * sizeof(int));
    int *arr2 = (int*)aligned_alloc(32, 256 * sizeof(int));
    float *farr = (float*)aligned_alloc(64, 128 * sizeof(float));
    double *darr = (double*)aligned_alloc(64, 64 * sizeof(double));
    
    if (!arr1 || !arr2 || !farr || !darr) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i * 3 + 1;
    }
    for (int i = 0; i < 256; i++) {
        arr2[i] = i * 5 + 2;
    }
    for (int i = 0; i < 128; i++) {
        farr[i] = (float)i * 0.7f;
    }
    for (int i = 0; i < 64; i++) {
        darr[i] = (double)i * 1.3;
    }
    
    int checksum = 0;
    
    /* Call the hot function multiple times to create scheduling regions */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Update volatile to create memory barrier */
        global_counter = iter;
        
        /* Process in chunks to create multiple scheduling regions */
        for (int chunk = 0; chunk < 8; chunk++) {
            int offset = chunk * (SIZE / 8);
            process_chunk(arr1, arr2, farr, darr, SIZE / 8, offset);
        }
        
        /* Simple checksum to prevent optimization */
        checksum ^= arr1[iter % SIZE];
        checksum ^= arr2[iter % 256];
        checksum ^= (int)farr[iter % 128];
        checksum ^= (int)darr[iter % 64];
        
        /* Additional computation to increase loop body size */
        if (iter % 1000 == 0) {
            for (int i = 0; i < 16; i++) {
                arr1[i] = arr1[i] * 7 + arr2[i % 256];
                farr[i % 128] = farr[i % 128] * 1.1f - (float)i;
                darr[i % 64] = darr[i % 64] / 1.05 + (double)(iter % 100);
            }
        }
    }
    
    /* Final computation and output to ensure no dead code elimination */
    int final_result = 0;
    for (int i = 0; i < SIZE; i++) {
        final_result += arr1[i];
        final_result ^= arr2[i % 256];
    }
    for (int i = 0; i < 128; i++) {
        final_result += (int)farr[i];
    }
    for (int i = 0; i < 64; i++) {
        final_result ^= (int)darr[i];
    }
    
    final_result ^= checksum;
    
    printf("Result: %d\n", final_result);
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr);
    free(darr);
    
    return 0;
}

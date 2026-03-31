/* Program to trigger selective scheduler debug dumps in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000000

/* Volatile variables to prevent optimization */
static volatile int g_volatile_counter = 0;

/* Function with memory aliasing */
static inline void compute_loop(int *restrict arr1, int *arr2, 
                               float *restrict farr, double *darr,
                               int start, int end, int seed) {
    int i;
    int local_sum = seed;
    float fsum = seed * 0.5f;
    double dsum = seed * 0.25;
    
    /* Hot loop with multiple dependencies and operations */
    for (i = start; i < end; i++) {
        /* Integer operations with carried dependency */
        local_sum = local_sum * 1103515245 + 12345;
        
        /* Floating-point operations */
        fsum = fsum * 1.5f + arr1[i % SIZE] * 0.01f;
        dsum = dsum / 1.7 + darr[i % SIZE] * 0.02;
        
        /* Memory operations with potential aliasing */
        arr1[i % SIZE] = local_sum + i;
        arr2[i % SIZE] = arr1[i % SIZE] * 2;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            fsum = fsum - farr[i % SIZE];
            dsum = dsum + 1.0;
            /* Additional operations in this path */
            local_sum ^= (i << 3);
        } else if (i % 13 == 0) {
            /* Another basic block */
            farr[i % SIZE] = fsum * 0.3f;
            darr[i % SIZE] = dsum * 0.4;
        } else {
            /* Default path */
            farr[i % SIZE] = fsum;
            darr[i % SIZE] = dsum;
        }
        
        /* More arithmetic diversity */
        if (i % 17 == 0) {
            local_sum = local_sum / (abs(local_sum % 100) + 1);
        }
        
        /* Inline assembly with memory clobber to prevent optimization */
        asm volatile("" : : "r"(local_sum), "r"(fsum), "r"(dsum) : "memory");
        
        /* Update volatile to create side effect */
        g_volatile_counter += (i & 0xFF);
    }
    
    /* Store results back */
    arr1[0] = local_sum;
    farr[0] = fsum;
    darr[0] = dsum;
}

/* Another hot function to encourage inlining */
static inline int process_chunk(int *restrict a, int *b, 
                               float *restrict f, double *d,
                               int chunk_size, int offset) {
    int i;
    int checksum = 0;
    
    for (i = 0; i < chunk_size; i++) {
        /* Different computation pattern */
        int idx = (i + offset) % SIZE;
        a[idx] = a[idx] * 3 + b[idx];
        b[idx] = b[idx] ^ a[idx];
        f[idx] = f[idx] + a[idx] * 0.01f - b[idx] * 0.02f;
        d[idx] = d[idx] * 0.99 + f[idx];
        
        /* Complex conditional */
        if ((a[idx] + b[idx]) % 5 == 0) {
            checksum ^= a[idx];
            f[idx] = f[idx] * 2.0f;
        } else {
            checksum ^= b[idx];
            d[idx] = d[idx] / 1.5;
        }
    }
    
    return checksum;
}

int main(void) {
    /* Allocate and initialize arrays */
    int *arr1 = (int*)malloc(SIZE * sizeof(int));
    int *arr2 = (int*)malloc(SIZE * sizeof(int));
    float *farr = (float*)malloc(SIZE * sizeof(float));
    double *darr = (double*)malloc(SIZE * sizeof(double));
    
    int i;
    for (i = 0; i < SIZE; i++) {
        arr1[i] = i;
        arr2[i] = SIZE - i;
        farr[i] = i * 0.5f;
        darr[i] = i * 0.25;
    }
    
    int total_checksum = 0;
    
    /* Call hot functions multiple times to create scheduling regions */
    for (i = 0; i < ITERATIONS / 1000; i++) {
        compute_loop(arr1, arr2, farr, darr, 0, 1000, i);
        
        /* Process different chunks */
        total_checksum ^= process_chunk(arr1, arr2, farr, darr, 500, i * 37);
        total_checksum ^= process_chunk(arr1, arr2, farr, darr, 500, i * 73);
        
        /* Additional computation to increase register pressure */
        for (int j = 0; j < 100; j++) {
            int idx = (i * 97 + j) % SIZE;
            arr1[idx] = arr1[idx] + arr2[idx] - total_checksum;
            farr[idx] = farr[idx] * 1.1f + j * 0.01f;
            darr[idx] = darr[idx] - farr[idx] * 0.5;
        }
    }
    
    /* Final computation and output to prevent dead code elimination */
    int final_result = 0;
    for (i = 0; i < SIZE; i++) {
        final_result ^= arr1[i];
        final_result ^= arr2[i];
        final_result ^= (int)farr[i];
        final_result ^= (int)darr[i];
    }
    
    final_result ^= total_checksum;
    final_result ^= g_volatile_counter;
    
    printf("Result: %d\n", final_result);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr);
    free(darr);
    
    return 0;
}

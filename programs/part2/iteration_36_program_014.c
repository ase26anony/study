/* Program to trigger selective scheduler debugging output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000000

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;

/* Function with potential aliasing */
static inline void compute_loop(int *restrict arr1, int *arr2, 
                               float *restrict farr, double *darr,
                               int start, int end, int seed) {
    int i;
    int local_sum = seed;
    float fsum = seed * 0.5f;
    double dsum = seed * 0.25;
    
    /* Complex loop with multiple dependencies and operations */
    for (i = start; i < end; i++) {
        /* Integer operations with carried dependency */
        local_sum = local_sum * 1103515245 + 12345;
        
        /* Floating-point operations */
        fsum = fsum * 1.5f + arr1[i % SIZE] * 0.01f;
        dsum = dsum / 1.7 + darr[i % SIZE] * 0.02;
        
        /* Memory operations with potential aliasing */
        arr1[i % SIZE] = local_sum + arr2[i % SIZE];
        arr2[i % SIZE] = arr2[i % SIZE] ^ local_sum;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            farr[i % SIZE] = fsum + i * 0.1f;
            darr[i % SIZE] = dsum - i * 0.05;
            
            /* Additional operations in conditional path */
            local_sum += (i % 13) * 3;
            fsum -= (i % 17) * 0.2f;
        } else if (i % 11 == 0) {
            /* Another basic block */
            farr[i % SIZE] = fsum * 0.9f;
            local_sum -= (i % 19) * 2;
        } else {
            /* Default path */
            farr[i % SIZE] = fsum;
        }
        
        /* More arithmetic diversity */
        if (i % 23 == 0) {
            dsum = dsum * 0.8 + (i % 29) * 0.03;
        }
        
        /* Volatile access to prevent reordering */
        g_volatile_counter += (i & 1);
        
        /* Inline assembly with memory clobber to create barriers */
        asm volatile("" ::: "memory");
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
        a[idx] = a[idx] * 6364136223846793005ULL + 1442695040888963407ULL;
        b[idx] = b[idx] + a[idx] * 3 - b[idx] / 2;
        
        /* Floating point with type mixing */
        f[idx] = f[idx] * 1.1f + i * 0.01f;
        d[idx] = d[idx] + f[idx] * 0.5;
        
        /* Complex conditional */
        if ((a[idx] ^ b[idx]) % 31 == 0) {
            f[idx] = f[idx] / 1.3f;
            checksum ^= a[idx];
        }
        
        /* Prevent optimization */
        asm volatile("" : "+r" (checksum) : : "memory");
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
    int final_result = 0;
    
    /* Initialize with pseudo-random values */
    for (i = 0; i < SIZE; i++) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 5 + 2;
        farr[i] = i * 0.7f + 1.5f;
        darr[i] = i * 0.9 + 2.5;
    }
    
    /* Multiple calls to hot functions with different parameters */
    for (i = 0; i < ITERATIONS / 1000; i++) {
        compute_loop(arr1, arr2, farr, darr, 
                    0, SIZE, i * 7);
        
        /* Interleave with different computation */
        int chunk_result = process_chunk(arr1, arr2, farr, darr, 
                                        SIZE / 2, i * 11);
        final_result ^= chunk_result;
    }
    
    /* Additional complex loop in main */
    int sum = 0;
    for (i = 0; i < SIZE; i++) {
        /* Mixed operations creating complex RTL */
        arr1[i] = arr1[i] + arr2[i] * 2 - arr1[i] / 3;
        farr[i] = farr[i] * 2.0f - darr[i];
        darr[i] = darr[i] + arr1[i] * 0.01 - farr[i] * 0.02;
        
        /* Complex expression with multiple operators */
        sum += arr1[i] ^ (int)farr[i] ^ (int)darr[i];
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
    }
    
    final_result ^= sum;
    
    /* Use results to prevent dead code elimination */
    printf("Result checksum: %d\n", final_result);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr);
    free(darr);
    
    return final_result != 0 ? 0 : 1;
}

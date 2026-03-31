/* Program to trigger selective scheduler debugging output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000000

/* Force memory dependencies with aliasing */
static inline void compute_loop(int *restrict arr1, int *arr2, 
                                float *restrict farr, double *darr,
                                int start, int end, int seed) {
    volatile int dep1 = seed;  /* Prevent optimization */
    volatile float dep2 = seed * 0.5f;
    volatile double dep3 = seed * 0.25;
    
    /* Hot loop with mixed operations and dependencies */
    for (int i = start; i < end; i++) {
        /* Integer operations with carried dependency */
        dep1 = dep1 * 1103515245 + 12345;
        
        /* Floating-point operations */
        dep2 = dep2 * 1.5f + (float)i * 0.01f;
        dep3 = dep3 / 1.7 + (double)i * 0.001;
        
        /* Memory operations with potential aliasing */
        int idx = i % SIZE;
        arr1[idx] = dep1 + arr2[idx];
        farr[idx] = dep2 * farr[idx];
        darr[idx] = dep3 - darr[idx];
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            /* Additional operations in taken branch */
            dep1 ^= 0x55555555;
            farr[idx] += 1.0f;
            asm volatile("" ::: "memory");  /* Memory clobber */
        } else if (i % 13 == 0) {
            /* Another basic block */
            dep1 += i;
            darr[idx] *= 2.0;
        }
        
        /* More arithmetic diversity */
        if (i % 3 == 0) {
            dep2 = dep2 / 3.14159f;
        }
        
        /* Complex expression with multiple operations */
        arr2[(i + 1) % SIZE] = (dep1 >> 3) * (i & 0xFF) - (int)dep2;
    }
}

/* Another inline function to increase scheduling complexity */
static inline void process_chunk(int *restrict a, int *b, 
                                 float *restrict f, double *d,
                                 int chunk_size, int offset) {
    for (int j = 0; j < chunk_size; j++) {
        int idx = (offset + j) % SIZE;
        
        /* Mixed type computations */
        double temp = d[idx] * 0.9 + (double)a[idx];
        f[idx] = (float)temp + f[idx] * 1.1f;
        
        /* Pointer arithmetic with aliasing */
        int *ptr1 = &a[idx];
        int *ptr2 = &b[idx];
        *ptr1 = (*ptr1 + *ptr2) | 0x1;
        *ptr2 = (*ptr2 - *ptr1) & 0x7FFFFFFF;
        
        /* Prevent loop unrolling */
        if (j % 5 == 0) {
            asm volatile("" : "+r" (idx) : : "memory");
        }
    }
}

int main(void) {
    /* Allocate and initialize arrays */
    int *arr1 = (int*)malloc(SIZE * sizeof(int));
    int *arr2 = (int*)malloc(SIZE * sizeof(int));
    float *farr = (float*)malloc(SIZE * sizeof(float));
    double *darr = (double*)malloc(SIZE * sizeof(double));
    
    if (!arr1 || !arr2 || !farr || !darr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i;
        arr2[i] = SIZE - i;
        farr[i] = (float)i * 0.5f;
        darr[i] = (double)i * 0.25;
    }
    
    int checksum = 0;
    
    /* Multiple calls to create scheduling regions */
    for (int iter = 0; iter < ITERATIONS / 1000; iter++) {
        compute_loop(arr1, arr2, farr, darr, 
                    0, SIZE, iter);
        
        process_chunk(arr1, arr2, farr, darr, 
                     SIZE / 2, iter * 17);
        
        /* Alternate between different loop bounds */
        if (iter % 2 == 0) {
            compute_loop(arr1, arr2, farr, darr, 
                        SIZE / 4, 3 * SIZE / 4, iter * 3);
        }
    }
    
    /* Compute checksum to prevent optimization */
    for (int i = 0; i < SIZE; i++) {
        checksum ^= arr1[i];
        checksum ^= arr2[i];
        checksum ^= (int)farr[i];
        checksum ^= (int)darr[i];
    }
    
    printf("Result checksum: %d\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr);
    free(darr);
    
    return 0;
}

/* Program to trigger selective scheduler debugging output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000000

/* Use volatile and restrict to create complex dependencies */
static inline uint64_t compute_loop(volatile int* restrict arr1, 
                                   int* arr2, 
                                   float* restrict farr,
                                   double* darr,
                                   int start,
                                   int end) {
    volatile int temp = 0;
    float f_acc = 1.0f;
    double d_acc = 1.0;
    uint64_t checksum = 0;
    
    /* Hot loop with mixed operations and dependencies */
    for (int i = start; i < end; i++) {
        /* Integer operations with carried dependency */
        temp = arr1[i] + temp;
        
        /* Floating point operations */
        f_acc = f_acc * farr[i] + 0.5f;
        d_acc = d_acc / (darr[i] + 0.1);
        
        /* Memory operations with potential aliasing */
        arr2[i] = temp + i;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            /* Different operation path */
            f_acc = f_acc - farr[i];
            d_acc = d_acc * 2.0;
            arr2[i] = arr2[i] * 3;
        } else if (i % 13 == 0) {
            /* Another basic block */
            f_acc = f_acc / 2.0f;
            temp = temp >> 1;
        }
        
        /* More arithmetic diversity */
        if (i % 3 == 0) {
            d_acc = d_acc + (double)temp * 0.25;
        }
        
        /* Create checksum with XOR to prevent optimization */
        checksum ^= (uint64_t)temp;
        checksum ^= (uint64_t)(f_acc * 1000);
        checksum ^= (uint64_t)(d_acc * 1000);
    }
    
    return checksum;
}

/* Another inline function to increase scheduling complexity */
static inline void process_chunk(volatile int* restrict a,
                                int* b,
                                float* restrict c,
                                double* d,
                                int chunk_size) {
    for (int j = 0; j < chunk_size; j += 64) {
        int end = (j + 64 < chunk_size) ? j + 64 : chunk_size;
        
        /* Inline assembly to prevent optimization and add complexity */
        asm volatile("" : "+r"(j) : : "memory");
        
        uint64_t cs = compute_loop(a, b, c, d, j, end);
        
        /* Use the checksum to prevent dead code elimination */
        b[end-1] ^= (int)(cs & 0xFFFFFFFF);
    }
}

int main(void) {
    /* Allocate and initialize arrays with different patterns */
    volatile int* arr1 = (volatile int*)malloc(SIZE * sizeof(int));
    int* arr2 = (int*)malloc(SIZE * sizeof(int));
    float* farr = (float*)malloc(SIZE * sizeof(float));
    double* darr = (double*)malloc(SIZE * sizeof(double));
    
    if (!arr1 || !arr2 || !farr || !darr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with varying values to create diverse RTL */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 5 + 2;
        farr[i] = (float)i * 0.7f + 0.3f;
        darr[i] = (double)i * 1.3 + 0.7;
    }
    
    uint64_t final_checksum = 0;
    
    /* Call the hot function multiple times to create scheduling regions */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Process in chunks to create different scheduling contexts */
        for (int chunk = 0; chunk < SIZE; chunk += 128) {
            int chunk_end = (chunk + 128 < SIZE) ? chunk + 128 : SIZE;
            
            /* Vary parameters slightly each iteration */
            int offset = iter % 4;
            
            /* Inline assembly to create memory barriers */
            asm volatile("" ::: "memory");
            
            uint64_t cs = compute_loop(arr1 + chunk + offset,
                                      arr2 + chunk,
                                      farr + chunk,
                                      darr + chunk,
                                      0,
                                      chunk_end - chunk - offset);
            
            final_checksum ^= cs;
            
            /* Also call the chunk processor */
            if (iter % 3 == 0) {
                process_chunk(arr1 + chunk,
                             arr2 + chunk,
                             farr + chunk,
                             darr + chunk,
                             chunk_end - chunk);
            }
        }
        
        /* Modify data slightly to change dependencies */
        if (iter % 100 == 0) {
            for (int i = 0; i < SIZE; i += 17) {
                arr1[i] = arr1[i] * 2 + 1;
                farr[i] = farr[i] * 1.1f;
            }
        }
    }
    
    /* Final computation to ensure all code is used */
    uint64_t verify = 0;
    for (int i = 0; i < SIZE; i++) {
        verify ^= (uint64_t)arr1[i];
        verify ^= (uint64_t)arr2[i];
        verify ^= (uint64_t)(farr[i] * 1000);
        verify ^= (uint64_t)(darr[i] * 1000);
    }
    
    final_checksum ^= verify;
    
    printf("Final checksum: %llu\n", (unsigned long long)final_checksum);
    
    /* Cleanup */
    free((void*)arr1);
    free(arr2);
    free(farr);
    free(darr);
    
    return 0;
}

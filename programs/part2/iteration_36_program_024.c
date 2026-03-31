/* Program to trigger selective scheduler debugging output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000000

/* Use volatile to prevent optimization */
static volatile int global_counter = 0;

/* Function with potential aliasing */
static inline void compute_loop(int *restrict arr1, int *arr2, 
                                float *restrict farr1, double *darr,
                                int start, int end, int seed) {
    int i;
    float f_acc = seed * 0.5f;
    double d_acc = seed * 0.25;
    int int_acc = seed;
    
    /* Complex loop with multiple dependencies and operations */
    for (i = start; i < end; i++) {
        /* Integer operations with carried dependency */
        int_acc = int_acc * 1103515245 + 12345;
        
        /* Floating point operations */
        f_acc = f_acc * 1.5f + arr1[i % SIZE] * 0.01f;
        d_acc = d_acc / 1.7 + darr[i % SIZE] * 0.5;
        
        /* Memory operations with potential aliasing */
        arr1[i % SIZE] = int_acc + (int)f_acc;
        arr2[i % SIZE] = arr2[i % SIZE] * 2 + arr1[(i + 1) % SIZE];
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            f_acc = f_acc - d_acc;
            /* Inline asm to prevent optimization */
            asm volatile("" : "+r"(int_acc) : : "memory");
        } else if (i % 13 == 0) {
            d_acc = d_acc + f_acc * 2.0;
            int_acc = int_acc ^ (i << 3);
        } else {
            /* Mixed operations */
            f_acc = f_acc + (float)(int_acc % 100) * 0.01f;
            d_acc = d_acc - (double)((i * 17) % 50) * 0.02;
        }
        
        /* More arithmetic diversity */
        if (i % 3 == 0) {
            int_acc = int_acc / (abs((i % 10) - 5) + 1);
        }
        
        /* Volatile access to create memory barrier */
        global_counter++;
    }
    
    /* Store results back */
    arr1[0] = int_acc;
    farr1[0] = f_acc;
    darr[0] = d_acc;
}

/* Another hot function to encourage inlining */
static inline int process_chunk(int *restrict a, int *b, 
                               float *restrict f, double *d,
                               int chunk_size, int base) {
    int sum = 0;
    for (int i = 0; i < chunk_size; i++) {
        /* Complex expression with multiple dependencies */
        int idx = (base + i) % SIZE;
        a[idx] = (a[idx] * 3 + b[idx] * 2) / (abs(i % 8 - 4) + 1);
        b[idx] = b[idx] ^ (a[(idx + 1) % SIZE] << (i % 4));
        f[idx] = f[idx] * 1.1f + (float)a[idx] * 0.01f;
        d[idx] = d[idx] + (double)b[idx] * 0.005 - f[idx] * 0.1;
        
        /* Running sum with dependency */
        sum += a[idx] + (int)f[idx];
        
        /* Conditional with side effect */
        if ((sum & 255) == 0) {
            asm volatile("" : : : "memory");
            sum = sum >> 1;
        }
    }
    return sum;
}

int main(void) {
    /* Allocate and initialize arrays */
    int *arr1 = (int*)malloc(SIZE * sizeof(int));
    int *arr2 = (int*)malloc(SIZE * sizeof(int));
    float *farr1 = (float*)malloc(SIZE * sizeof(float));
    double *darr = (double*)malloc(SIZE * sizeof(double));
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 5 + 1;
        farr1[i] = i * 0.7f;
        darr[i] = i * 1.3;
    }
    
    int total_result = 0;
    
    /* Multiple calls to hot functions to create scheduling regions */
    for (int iter = 0; iter < 100; iter++) {
        /* Call the main compute loop */
        compute_loop(arr1, arr2, farr1, darr, 
                    0, ITERATIONS / 100, iter * 17);
        
        /* Process chunks with different parameters */
        int chunk_result = process_chunk(arr1, arr2, farr1, darr, 
                                        SIZE / 2, iter * 23);
        total_result ^= chunk_result;
        
        /* Additional mixed computation */
        for (int i = 0; i < SIZE; i += 8) {
            /* Vector-like operations (will be scalarized) */
            arr1[i] = arr1[i] + arr2[i + 1];
            arr2[i] = arr2[i] - arr1[i + 2];
            farr1[i] = farr1[i] * farr1[i + 3];
            darr[i] = darr[i] / (darr[i + 4] + 1.0);
            
            /* Dependency chain */
            if (i > 0) {
                arr1[i] = arr1[i] + arr1[i - 1];
            }
        }
    }
    
    /* Final computation to use results */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= arr1[i];
        checksum ^= (int)farr1[i];
        checksum += arr2[i];
        checksum ^= (int)darr[i];
    }
    
    checksum ^= total_result;
    checksum ^= global_counter;
    
    printf("Result checksum: %d\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr1);
    free(darr);
    
    return 0;
}

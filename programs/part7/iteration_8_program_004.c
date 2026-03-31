/*
 * Cache-sensitive test program to trigger GCC driver's cache detection logic.
 * This program performs operations that encourage the compiler to consider
 * cache parameters during optimization.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#define MATRIX_SIZE 1024
#define ITERATIONS 5

/* Large multi-dimensional arrays to stress cache */
static double matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_c[MATRIX_SIZE][MATRIX_SIZE];

/* Function with nested loops accessing arrays in different patterns */
void matrix_multiply_row_major() {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            double sum = 0.0;
            for (int k = 0; k < MATRIX_SIZE; k++) {
                sum += matrix_a[i][k] * matrix_b[k][j];
            }
            matrix_c[i][j] = sum;
        }
    }
}

/* Column-major access pattern */
void matrix_transpose() {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix_b[j][i] = matrix_a[i][j];
        }
    }
}

/* Function with cache-aware prefetching hints */
void cache_aware_traversal(double* array, size_t size) {
    const int prefetch_distance = 64; /* Should match cache line size */
    
    for (size_t i = 0; i < size; i++) {
        /* Prefetch ahead to hide memory latency */
        if (i + prefetch_distance < size) {
            __builtin_prefetch(&array[i + prefetch_distance], 0, 3);
        }
        array[i] = array[i] * 1.01;
    }
}

/* Different access pattern to trigger different cache considerations */
void strided_access(double* array, size_t size, int stride) {
    for (size_t i = 0; i < size; i += stride) {
        array[i] = array[i] * 0.99;
    }
}

/* Compute checksum to prevent dead code elimination */
double compute_checksum() {
    double checksum = 0.0;
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            checksum += matrix_c[i][j];
        }
    }
    return checksum;
}

/* Initialize matrices with deterministic values */
void initialize_matrices() {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = (i + j) * 0.001;
            matrix_b[i][j] = (i - j) * 0.001;
            matrix_c[i][j] = 0.0;
        }
    }
}

int main() {
    clock_t start, end;
    double cpu_time_used;
    
    printf("Starting cache-sensitive computation...\n");
    
    initialize_matrices();
    
    /* Perform multiple iterations with different access patterns */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        start = clock();
        
        /* Mix different access patterns */
        if (iter % 2 == 0) {
            matrix_multiply_row_major();
        } else {
            matrix_transpose();
            matrix_multiply_row_major();
        }
        
        /* Process arrays with cache-aware prefetching */
        cache_aware_traversal((double*)matrix_a, MATRIX_SIZE * MATRIX_SIZE);
        strided_access((double*)matrix_b, MATRIX_SIZE * MATRIX_SIZE, 8);
        
        end = clock();
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
        
        printf("Iteration %d: %.3f seconds\n", iter + 1, cpu_time_used);
    }
    
    /* Compute and print final checksum */
    double final_checksum = compute_checksum();
    printf("Final checksum: %.6f\n", final_checksum);
    
    /* Additional cache-sensitive operation */
    double* dynamic_array = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    if (dynamic_array) {
        for (size_t i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
            dynamic_array[i] = i * 0.0001;
        }
        
        /* Process with different strides to test various cache behaviors */
        for (int stride = 1; stride <= 16; stride *= 2) {
            strided_access(dynamic_array, MATRIX_SIZE * MATRIX_SIZE, stride);
        }
        
        free(dynamic_array);
    }
    
    printf("Computation completed successfully.\n");
    return 0;
}

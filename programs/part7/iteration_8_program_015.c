/*
 * Cache-sensitive test program to trigger GCC driver's cache detection logic.
 * This program performs operations that benefit from cache-aware optimizations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MATRIX_SIZE 1024
#define BLOCK_SIZE 32

/* Large multi-dimensional arrays to encourage cache-blocking optimizations */
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

/* Column-major access pattern to test different cache behavior */
void matrix_multiply_column_major() {
    for (int j = 0; j < MATRIX_SIZE; j++) {
        for (int i = 0; i < MATRIX_SIZE; i++) {
            double sum = 0.0;
            for (int k = 0; k < MATRIX_SIZE; k++) {
                sum += matrix_a[i][k] * matrix_b[k][j];
            }
            matrix_c[i][j] = sum;
        }
    }
}

/* Cache-blocked matrix multiplication */
void matrix_multiply_blocked() {
    for (int ii = 0; ii < MATRIX_SIZE; ii += BLOCK_SIZE) {
        for (int jj = 0; jj < MATRIX_SIZE; jj += BLOCK_SIZE) {
            for (int kk = 0; kk < MATRIX_SIZE; kk += BLOCK_SIZE) {
                for (int i = ii; i < ii + BLOCK_SIZE && i < MATRIX_SIZE; i++) {
                    for (int j = jj; j < jj + BLOCK_SIZE && j < MATRIX_SIZE; j++) {
                        double sum = matrix_c[i][j];
                        for (int k = kk; k < kk + BLOCK_SIZE && k < MATRIX_SIZE; k++) {
                            sum += matrix_a[i][k] * matrix_b[k][j];
                        }
                        matrix_c[i][j] = sum;
                    }
                }
            }
        }
    }
}

/* Function using __builtin_prefetch for cache-aware access */
void prefetch_test(double *arr, int size) {
    for (int i = 0; i < size - 8; i += 8) {
        /* Prefetch data that will be needed soon */
        __builtin_prefetch(&arr[i + 8], 0, 3);
        
        /* Perform some computation */
        arr[i] = arr[i] * 1.5 + arr[i + 1];
    }
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

/* Compute checksum to prevent dead code elimination */
double compute_checksum() {
    double checksum = 0.0;
    for (int i = 0; i < MATRIX_SIZE; i += 64) {
        for (int j = 0; j < MATRIX_SIZE; j += 64) {
            checksum += matrix_c[i][j];
        }
    }
    return checksum;
}

/* Test different access patterns */
void test_all_patterns() {
    printf("Testing row-major access pattern...\n");
    matrix_multiply_row_major();
    double checksum1 = compute_checksum();
    
    printf("Testing column-major access pattern...\n");
    initialize_matrices();
    matrix_multiply_column_major();
    double checksum2 = compute_checksum();
    
    printf("Testing cache-blocked access pattern...\n");
    initialize_matrices();
    matrix_multiply_blocked();
    double checksum3 = compute_checksum();
    
    /* Test prefetching */
    printf("Testing prefetch operations...\n");
    double test_array[1024];
    for (int i = 0; i < 1024; i++) {
        test_array[i] = i * 0.01;
    }
    prefetch_test(test_array, 1024);
    
    printf("Checksums: %.6f, %.6f, %.6f\n", checksum1, checksum2, checksum3);
    
    /* Use results to prevent optimization */
    if (checksum1 + checksum2 + checksum3 > 1000000.0) {
        printf("Result validation passed\n");
    }
}

int main() {
    printf("Starting cache-sensitive computation test...\n");
    
    /* Initialize with deterministic values for reproducibility */
    initialize_matrices();
    
    /* Run tests with different access patterns */
    test_all_patterns();
    
    /* Additional memory-intensive operations */
    printf("Performing additional memory operations...\n");
    
    /* Large 1D array traversal */
    double large_array[8192];
    for (int i = 0; i < 8192; i++) {
        large_array[i] = i * 0.001;
    }
    
    /* Reverse traversal */
    for (int i = 8191; i >= 0; i--) {
        large_array[i] = large_array[i] * 1.1;
    }
    
    /* Strided access pattern */
    for (int stride = 1; stride <= 16; stride *= 2) {
        for (int i = 0; i < 8192; i += stride) {
            large_array[i] += 0.5;
        }
    }
    
    printf("Test completed successfully.\n");
    return 0;
}

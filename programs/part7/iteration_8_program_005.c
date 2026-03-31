/**
 * Cache detection test program
 * This program performs cache-sensitive operations to encourage the GCC driver
 * to query CPU cache information during compilation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MATRIX_SIZE 1024
#define BLOCK_SIZE 32

/* Large multi-dimensional arrays to encourage cache-aware optimizations */
static double matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_c[MATRIX_SIZE][MATRIX_SIZE];

/* Function prototypes */
void initialize_matrices(void);
void matrix_multiply_naive(void);
void matrix_multiply_blocked(void);
void matrix_transpose_access(void);
double compute_checksum(void);
void cache_sensitive_operations(void);

/**
 * Initialize matrices with deterministic values
 */
void initialize_matrices(void) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = (i + j) * 0.001;
            matrix_b[i][j] = (i - j) * 0.001;
            matrix_c[i][j] = 0.0;
        }
    }
}

/**
 * Naive matrix multiplication (cache-inefficient)
 */
void matrix_multiply_naive(void) {
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

/**
 * Blocked matrix multiplication (cache-efficient)
 */
void matrix_multiply_blocked(void) {
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

/**
 * Access pattern that stresses cache with transposition
 */
void matrix_transpose_access(void) {
    double temp[MATRIX_SIZE][MATRIX_SIZE];
    
    /* Row-major access */
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            temp[i][j] = matrix_a[i][j];
        }
    }
    
    /* Column-major access (cache-unfriendly) */
    for (int j = 0; j < MATRIX_SIZE; j++) {
        for (int i = 0; i < MATRIX_SIZE; i++) {
            matrix_b[i][j] = temp[j][i];
        }
    }
}

/**
 * Compute checksum of matrix C for validation
 */
double compute_checksum(void) {
    double checksum = 0.0;
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            checksum += matrix_c[i][j] * (i + j + 1);
        }
    }
    return checksum;
}

/**
 * Perform various cache-sensitive operations
 * Uses __builtin_prefetch to hint at cache-aware access patterns
 */
void cache_sensitive_operations(void) {
    volatile double* volatile_ptr;
    double local_array[BLOCK_SIZE * BLOCK_SIZE];
    
    /* Use __builtin_prefetch with different access patterns */
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j += 16) {
            /* Prefetch ahead in the matrix */
            if (j + 32 < MATRIX_SIZE) {
                __builtin_prefetch(&matrix_a[i][j + 32], 0, 3);
                __builtin_prefetch(&matrix_b[j + 32][i], 0, 3);
            }
            
            /* Force compiler to consider cache effects */
            volatile_ptr = &matrix_a[i][j];
            (void)volatile_ptr;
        }
    }
    
    /* Strided access pattern */
    for (int stride = 1; stride <= 64; stride *= 2) {
        for (int i = 0; i < BLOCK_SIZE * BLOCK_SIZE; i += stride) {
            local_array[i] = i * 0.01;
        }
    }
}

/**
 * Main function that performs cache-sensitive computations
 */
int main(void) {
    clock_t start, end;
    double cpu_time_used;
    
    printf("Cache Detection Test Program\n");
    printf("Matrix size: %d x %d\n", MATRIX_SIZE, MATRIX_SIZE);
    
    /* Initialize matrices */
    start = clock();
    initialize_matrices();
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Initialization time: %.3f seconds\n", cpu_time_used);
    
    /* Perform cache-sensitive operations */
    cache_sensitive_operations();
    
    /* Matrix multiplication with different algorithms */
    printf("\nPerforming matrix multiplications...\n");
    
    /* Blocked multiplication (cache-friendly) */
    memset(matrix_c, 0, sizeof(matrix_c));
    start = clock();
    matrix_multiply_blocked();
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Blocked multiplication time: %.3f seconds\n", cpu_time_used);
    
    /* Naive multiplication (cache-unfriendly) */
    memset(matrix_c, 0, sizeof(matrix_c));
    start = clock();
    matrix_multiply_naive();
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Naive multiplication time: %.3f seconds\n", cpu_time_used);
    
    /* Transpose access pattern */
    start = clock();
    matrix_transpose_access();
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Transpose access time: %.3f seconds\n", cpu_time_used);
    
    /* Compute and print checksum for validation */
    double checksum = compute_checksum();
    printf("\nFinal checksum: %.6f\n", checksum);
    printf("Test completed successfully.\n");
    
    return 0;
}

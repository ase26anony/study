/*
 * Cache-sensitive test program to trigger GCC driver's cache detection logic
 * This program performs operations that benefit from cache-aware optimizations
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MATRIX_SIZE 1024
#define ITERATIONS 5

/* Large multi-dimensional arrays to stress cache */
static double matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_c[MATRIX_SIZE][MATRIX_SIZE];
static double vector[MATRIX_SIZE];
static double result[MATRIX_SIZE];

/* Initialize matrices with deterministic values */
void init_matrices() {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = (i + j) * 0.001;
            matrix_b[i][j] = (i - j) * 0.001;
            matrix_c[i][j] = 0.0;
        }
        vector[i] = i * 0.01;
        result[i] = 0.0;
    }
}

/* Row-major access pattern (cache-friendly) */
void row_major_multiply() {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix_c[i][j] = matrix_a[i][j] * matrix_b[i][j];
        }
    }
}

/* Column-major access pattern (cache-unfriendly) */
void column_major_multiply() {
    for (int j = 0; j < MATRIX_SIZE; j++) {
        for (int i = 0; i < MATRIX_SIZE; i++) {
            matrix_c[i][j] = matrix_a[i][j] * matrix_b[i][j];
        }
    }
}

/* Matrix-vector multiplication with prefetch hints */
void matrix_vector_multiply() {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        double sum = 0.0;
        for (int j = 0; j < MATRIX_SIZE; j++) {
            /* Use builtin prefetch for cache-aware access */
            if (j + 4 < MATRIX_SIZE) {
                __builtin_prefetch(&matrix_a[i][j + 4], 0, 3);
                __builtin_prefetch(&vector[j + 4], 0, 3);
            }
            sum += matrix_a[i][j] * vector[j];
        }
        result[i] = sum;
    }
}

/* Blocked matrix multiplication (cache-aware algorithm) */
void blocked_matrix_multiply(int block_size) {
    for (int ii = 0; ii < MATRIX_SIZE; ii += block_size) {
        for (int jj = 0; jj < MATRIX_SIZE; jj += block_size) {
            for (int kk = 0; kk < MATRIX_SIZE; kk += block_size) {
                for (int i = ii; i < ii + block_size && i < MATRIX_SIZE; i++) {
                    for (int j = jj; j < jj + block_size && j < MATRIX_SIZE; j++) {
                        double sum = matrix_c[i][j];
                        for (int k = kk; k < kk + block_size && k < MATRIX_SIZE; k++) {
                            sum += matrix_a[i][k] * matrix_b[k][j];
                        }
                        matrix_c[i][j] = sum;
                    }
                }
            }
        }
    }
}

/* Compute checksum to prevent dead code elimination */
double compute_checksum() {
    double checksum = 0.0;
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            checksum += matrix_c[i][j] * (i + j);
        }
        checksum += result[i] * i;
    }
    return checksum;
}

int main() {
    clock_t start, end;
    double cpu_time_used;
    double total_checksum = 0.0;
    
    printf("Cache detection test program\n");
    printf("Matrix size: %d x %d\n", MATRIX_SIZE, MATRIX_SIZE);
    printf("Iterations: %d\n\n", ITERATIONS);
    
    init_matrices();
    
    /* Test different access patterns to trigger cache-aware optimizations */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        printf("Iteration %d:\n", iter + 1);
        
        start = clock();
        row_major_multiply();
        end = clock();
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("  Row-major multiply: %.3f seconds\n", cpu_time_used);
        
        start = clock();
        column_major_multiply();
        end = clock();
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("  Column-major multiply: %.3f seconds\n", cpu_time_used);
        
        start = clock();
        matrix_vector_multiply();
        end = clock();
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("  Matrix-vector multiply with prefetch: %.3f seconds\n", cpu_time_used);
        
        /* Try different block sizes to explore cache effects */
        int block_size = 32 << (iter % 3);  /* 32, 64, or 128 */
        start = clock();
        blocked_matrix_multiply(block_size);
        end = clock();
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("  Blocked multiply (block=%d): %.3f seconds\n", block_size, cpu_time_used);
        
        double iter_checksum = compute_checksum();
        total_checksum += iter_checksum;
        printf("  Iteration checksum: %.6e\n\n", iter_checksum);
    }
    
    printf("Total checksum: %.6e\n", total_checksum);
    printf("Test completed successfully.\n");
    
    return 0;
}

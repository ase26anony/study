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
            for (int k = 0; k < MATRIX_SIZE; k++) {
                matrix_c[i][j] += matrix_a[i][k] * matrix_b[k][j];
            }
        }
    }
}

/* Column-major access pattern (cache-unfriendly) */
void column_major_multiply() {
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

/* Mixed access patterns with prefetch hints */
void mixed_access_with_prefetch() {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j += 8) {
            /* Prefetch next cache lines */
            if (j + 16 < MATRIX_SIZE) {
                __builtin_prefetch(&matrix_a[i][j + 16], 0, 3);
                __builtin_prefetch(&matrix_b[0][j + 16], 0, 3);
            }
            
            double sum = 0.0;
            for (int k = 0; k < MATRIX_SIZE; k++) {
                sum += matrix_a[i][k] * matrix_b[k][j];
            }
            matrix_c[i][j] = sum;
        }
    }
}

/* Blocked matrix multiplication for better cache utilization */
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

/* Main function with timing and validation */
int main() {
    clock_t start, end;
    double cpu_time_used;
    double final_checksum = 0.0;
    
    printf("Starting cache-sensitive computations...\n");
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        init_matrices();
        
        start = clock();
        
        /* Perform different access patterns to exercise various optimizations */
        if (iter % 3 == 0) {
            row_major_multiply();
        } else if (iter % 3 == 1) {
            column_major_multiply();
        } else {
            /* Try different block sizes to test cache blocking optimizations */
            int block_size = 32 + (iter * 16) % 64;
            blocked_matrix_multiply(block_size);
        }
        
        /* Use prefetch in some iterations */
        if (iter % 2 == 0) {
            mixed_access_with_prefetch();
        }
        
        end = clock();
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        
        double iter_checksum = compute_checksum();
        final_checksum += iter_checksum;
        
        printf("Iteration %d: checksum = %.6f, time = %.3f seconds\n", 
               iter, iter_checksum, cpu_time_used);
    }
    
    printf("\nFinal accumulated checksum: %.6f\n", final_checksum);
    printf("Test completed successfully.\n");
    
    /* Return deterministic exit code based on checksum */
    return ((int)final_checksum) % 2;
}

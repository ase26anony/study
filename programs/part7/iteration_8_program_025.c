/*
 * Cache-sensitive test program to trigger GCC driver's cache detection logic
 * This program performs operations that benefit from cache-aware optimizations
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MATRIX_SIZE 1024
#define ITERATIONS 5

/* Large multi-dimensional arrays to stress cache */
static double matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_c[MATRIX_SIZE][MATRIX_SIZE];

/* Initialize matrices with deterministic values */
void init_matrices(void) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = (i + j) * 0.001;
            matrix_b[i][j] = (i - j) * 0.001;
            matrix_c[i][j] = 0.0;
        }
    }
}

/* Row-major access pattern (cache-friendly) */
void matrix_multiply_row_major(void) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int k = 0; k < MATRIX_SIZE; k++) {
            double aik = matrix_a[i][k];
            // Use __builtin_prefetch to hint at cache behavior
            if (k + 4 < MATRIX_SIZE) {
                __builtin_prefetch(&matrix_a[i][k + 4], 0, 3);
                __builtin_prefetch(&matrix_b[k + 4][0], 0, 3);
            }
            for (int j = 0; j < MATRIX_SIZE; j++) {
                matrix_c[i][j] += aik * matrix_b[k][j];
            }
        }
    }
}

/* Column-major access pattern (cache-unfriendly) */
void matrix_multiply_col_major(void) {
    for (int j = 0; j < MATRIX_SIZE; j++) {
        for (int k = 0; k < MATRIX_SIZE; k++) {
            double bkj = matrix_b[k][j];
            // Prefetch hints for column-major access
            if (k + 4 < MATRIX_SIZE) {
                __builtin_prefetch(&matrix_b[k + 4][j], 0, 3);
                __builtin_prefetch(&matrix_a[0][k + 4], 0, 3);
            }
            for (int i = 0; i < MATRIX_SIZE; i++) {
                matrix_c[i][j] += matrix_a[i][k] * bkj;
            }
        }
    }
}

/* Blocked matrix multiplication (cache-aware algorithm) */
void matrix_multiply_blocked(int block_size) {
    for (int ii = 0; ii < MATRIX_SIZE; ii += block_size) {
        for (int jj = 0; jj < MATRIX_SIZE; jj += block_size) {
            for (int kk = 0; kk < MATRIX_SIZE; kk += block_size) {
                // Process block
                for (int i = ii; i < ii + block_size && i < MATRIX_SIZE; i++) {
                    for (int k = kk; k < kk + block_size && k < MATRIX_SIZE; k++) {
                        double aik = matrix_a[i][k];
                        // Prefetch next blocks
                        if (k + 2 < kk + block_size && k + 2 < MATRIX_SIZE) {
                            __builtin_prefetch(&matrix_a[i][k + 2], 0, 3);
                        }
                        for (int j = jj; j < jj + block_size && j < MATRIX_SIZE; j++) {
                            matrix_c[i][j] += aik * matrix_b[k][j];
                        }
                    }
                }
            }
        }
    }
}

/* Compute checksum to prevent dead code elimination */
double compute_checksum(void) {
    double checksum = 0.0;
    for (int i = 0; i < MATRIX_SIZE; i += 64) {  // Strided access
        for (int j = 0; j < MATRIX_SIZE; j += 64) {
            checksum += matrix_c[i][j];
        }
    }
    return checksum;
}

/* Array traversal with different access patterns */
void array_traversal_patterns(void) {
    // Large 1D array for various access patterns
    double* large_array = (double*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    
    if (!large_array) return;
    
    // Initialize
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        large_array[i] = i * 0.0001;
    }
    
    // Sequential access (cache-friendly)
    double sum_seq = 0.0;
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        sum_seq += large_array[i];
        if (i % 64 == 0) {
            __builtin_prefetch(&large_array[i + 64], 0, 3);
        }
    }
    
    // Strided access (less cache-friendly)
    double sum_stride = 0.0;
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            int idx = j * MATRIX_SIZE + i;  // Column-major indexing
            sum_stride += large_array[idx];
            if (j % 8 == 0) {
                __builtin_prefetch(&large_array[idx + 8 * MATRIX_SIZE], 0, 3);
            }
        }
    }
    
    // Random access (cache-unfriendly)
    double sum_random = 0.0;
    unsigned int seed = 42;
    for (int i = 0; i < 100000; i++) {
        int idx = rand_r(&seed) % (MATRIX_SIZE * MATRIX_SIZE);
        sum_random += large_array[idx];
    }
    
    // Use results to prevent optimization
    matrix_c[0][0] += sum_seq + sum_stride + sum_random;
    
    free(large_array);
}

int main(void) {
    clock_t start, end;
    double total_time = 0.0;
    double total_checksum = 0.0;
    
    printf("Starting cache-sensitive computations...\n");
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        init_matrices();
        
        start = clock();
        
        // Mix different access patterns
        matrix_multiply_row_major();
        array_traversal_patterns();
        matrix_multiply_col_major();
        
        // Try different block sizes for cache blocking
        matrix_multiply_blocked(32);   // Small block
        matrix_multiply_blocked(64);   // Medium block
        matrix_multiply_blocked(128);  // Large block
        
        end = clock();
        
        double checksum = compute_checksum();
        total_checksum += checksum;
        
        double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
        total_time += elapsed;
        
        printf("Iteration %d: checksum = %.6f, time = %.3f seconds\n", 
               iter + 1, checksum, elapsed);
    }
    
    printf("\nTotal time: %.3f seconds\n", total_time);
    printf("Final accumulated checksum: %.6f\n", total_checksum);
    
    // Use result to prevent dead code elimination
    if (total_checksum > 0.0) {
        printf("Test completed successfully.\n");
        return 0;
    } else {
        printf("Test failed - no computation performed.\n");
        return 1;
    }
}

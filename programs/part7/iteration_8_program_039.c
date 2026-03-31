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
static double vector[MATRIX_SIZE * MATRIX_SIZE];

/* Initialize matrices with deterministic values */
void init_matrices(void) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = (i + j) * 0.001;
            matrix_b[i][j] = (i - j) * 0.001;
            matrix_c[i][j] = 0.0;
        }
    }
    
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        vector[i] = i * 0.0001;
    }
}

/* Row-major access pattern (cache-friendly) */
void matrix_multiply_row_major(void) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int k = 0; k < MATRIX_SIZE; k++) {
            double aik = matrix_a[i][k];
            /* Prefetch hint for cache-aware access */
            if (k + 4 < MATRIX_SIZE) {
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
                int i_end = ii + block_size < MATRIX_SIZE ? ii + block_size : MATRIX_SIZE;
                int j_end = jj + block_size < MATRIX_SIZE ? jj + block_size : MATRIX_SIZE;
                int k_end = kk + block_size < MATRIX_SIZE ? kk + block_size : MATRIX_SIZE;
                
                for (int i = ii; i < i_end; i++) {
                    for (int k = kk; k < k_end; k++) {
                        double aik = matrix_a[i][k];
                        /* Prefetch for next block */
                        if (k + 2 < k_end) {
                            __builtin_prefetch(&matrix_a[i][k + 2], 0, 1);
                        }
                        for (int j = jj; j < j_end; j++) {
                            matrix_c[i][j] += aik * matrix_b[k][j];
                        }
                    }
                }
            }
        }
    }
}

/* Memory-intensive computation with different access patterns */
double compute_checksum(void) {
    double checksum = 0.0;
    
    /* Access in different strides to test various cache behaviors */
    for (int stride = 1; stride <= 16; stride *= 2) {
        for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i += stride) {
            checksum += vector[i] * 0.5;
            /* Prefetch ahead */
            if (i + stride * 4 < MATRIX_SIZE * MATRIX_SIZE) {
                __builtin_prefetch(&vector[i + stride * 4], 0, 2);
            }
        }
    }
    
    /* Process matrix with diagonal access pattern */
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            int idx = (i + j) % MATRIX_SIZE;
            checksum += matrix_c[i][idx] * 0.1;
        }
    }
    
    return checksum;
}

/* Test different cache blocking strategies */
void test_cache_blocking(void) {
    /* Try different block sizes that might be optimal for different cache sizes */
    int block_sizes[] = {16, 32, 64, 128, 256};
    int num_sizes = sizeof(block_sizes) / sizeof(block_sizes[0]);
    
    for (int b = 0; b < num_sizes; b++) {
        /* Clear result matrix */
        for (int i = 0; i < MATRIX_SIZE; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++) {
                matrix_c[i][j] = 0.0;
            }
        }
        
        matrix_multiply_blocked(block_sizes[b]);
    }
}

int main(void) {
    clock_t start, end;
    double cpu_time_used;
    double final_checksum = 0.0;
    
    printf("Starting cache-sensitive computation...\n");
    printf("Matrix size: %d x %d\n", MATRIX_SIZE, MATRIX_SIZE);
    printf("Total data: %.2f MB\n", 
           (3.0 * MATRIX_SIZE * MATRIX_SIZE * sizeof(double)) / (1024.0 * 1024.0));
    
    init_matrices();
    
    /* Perform multiple iterations with different access patterns */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        printf("\nIteration %d:\n", iter + 1);
        
        /* Clear result matrix */
        for (int i = 0; i < MATRIX_SIZE; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++) {
                matrix_c[i][j] = 0.0;
            }
        }
        
        start = clock();
        
        switch (iter % 3) {
            case 0:
                printf("  Row-major matrix multiplication\n");
                matrix_multiply_row_major();
                break;
            case 1:
                printf("  Column-major matrix multiplication\n");
                matrix_multiply_col_major();
                break;
            case 2:
                printf("  Cache-blocked multiplication\n");
                test_cache_blocking();
                break;
        }
        
        end = clock();
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("  Time: %.2f seconds\n", cpu_time_used);
        
        /* Update checksum */
        double iter_checksum = compute_checksum();
        final_checksum += iter_checksum;
        printf("  Iteration checksum: %.6f\n", iter_checksum);
    }
    
    printf("\nFinal checksum: %.6f\n", final_checksum);
    printf("Computation completed successfully.\n");
    
    /* Use the result to prevent dead code elimination */
    if (final_checksum != 0.0) {
        return 0;
    } else {
        return 1;
    }
}

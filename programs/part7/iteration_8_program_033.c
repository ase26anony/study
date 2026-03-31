/**
 * Test program to trigger GCC driver's cache detection logic.
 * This program performs cache-sensitive operations that encourage
 * the compiler to consider cache parameters during optimization.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MATRIX_SIZE 1024
#define BLOCK_SIZE 64
#define ITERATIONS 5

/* Large multi-dimensional arrays to stress cache */
static double matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_c[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_d[MATRIX_SIZE][MATRIX_SIZE];

/* Different access pattern arrays */
static double row_major[MATRIX_SIZE][MATRIX_SIZE];
static double col_major[MATRIX_SIZE][MATRIX_SIZE];

/**
 * Cache-blocked matrix multiplication
 * This pattern is highly sensitive to cache parameters
 */
void blocked_matrix_multiply(int block_size) {
    int i, j, k, ii, jj, kk;
    
    for (i = 0; i < MATRIX_SIZE; i += block_size) {
        for (j = 0; j < MATRIX_SIZE; j += block_size) {
            for (k = 0; k < MATRIX_SIZE; k += block_size) {
                /* Process block */
                for (ii = i; ii < i + block_size && ii < MATRIX_SIZE; ii++) {
                    for (jj = j; jj < j + block_size && jj < MATRIX_SIZE; jj++) {
                        double sum = matrix_c[ii][jj];
                        for (kk = k; kk < k + block_size && kk < MATRIX_SIZE; kk++) {
                            sum += matrix_a[ii][kk] * matrix_b[kk][jj];
                        }
                        matrix_c[ii][jj] = sum;
                    }
                }
            }
        }
    }
}

/**
 * Row-major vs column-major access patterns
 * Mixing patterns helps trigger different cache considerations
 */
void mixed_access_patterns(void) {
    int i, j;
    
    /* Row-major access (cache-friendly) */
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            row_major[i][j] = (double)(i * MATRIX_SIZE + j);
        }
    }
    
    /* Column-major access (cache-unfriendly) */
    for (j = 0; j < MATRIX_SIZE; j++) {
        for (i = 0; i < MATRIX_SIZE; i++) {
            col_major[i][j] = (double)(i * MATRIX_SIZE + j);
        }
    }
    
    /* Transpose operation - mixes access patterns */
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = i + 1; j < MATRIX_SIZE; j++) {
            double temp = row_major[i][j];
            row_major[i][j] = row_major[j][i];
            row_major[j][i] = temp;
        }
    }
}

/**
 * Function using __builtin_prefetch to hint at cache-aware access
 */
void prefetch_optimized_traversal(void) {
    int i, j;
    
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE - 4; j++) {
            /* Prefetch ahead in the row */
            __builtin_prefetch(&matrix_d[i][j + 4], 0, 3);
            matrix_d[i][j] = matrix_a[i][j] * matrix_b[j][i];
        }
    }
}

/**
 * Initialize matrices with deterministic values
 */
void initialize_matrices(void) {
    int i, j;
    
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = (double)((i + j) % 100) * 0.01;
            matrix_b[i][j] = (double)((i * j) % 100) * 0.01;
            matrix_c[i][j] = 0.0;
            matrix_d[i][j] = 0.0;
        }
    }
}

/**
 * Compute checksum to prevent dead code elimination
 */
double compute_checksum(void) {
    double checksum = 0.0;
    int i, j;
    
    for (i = 0; i < MATRIX_SIZE; i += 8) {
        for (j = 0; j < MATRIX_SIZE; j += 8) {
            checksum += matrix_c[i][j] + matrix_d[i][j];
        }
    }
    
    return checksum;
}

int main(void) {
    double total_checksum = 0.0;
    int iter;
    
    printf("Starting cache-sensitive computations...\n");
    
    for (iter = 0; iter < ITERATIONS; iter++) {
        initialize_matrices();
        
        /* Perform various cache-sensitive operations */
        blocked_matrix_multiply(BLOCK_SIZE);
        mixed_access_patterns();
        prefetch_optimized_traversal();
        
        /* Accumulate checksum to ensure computations aren't optimized away */
        total_checksum += compute_checksum();
        
        printf("Iteration %d complete, partial checksum: %.6f\n", 
               iter + 1, compute_checksum());
    }
    
    printf("Total checksum after %d iterations: %.6f\n", 
           ITERATIONS, total_checksum);
    
    /* Print a deterministic result for validation */
    printf("Test completed successfully. Final value: %.6f\n", total_checksum);
    
    return 0;
}

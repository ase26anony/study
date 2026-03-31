/*
 * Cache-sensitive test program to trigger GCC driver's cache detection logic
 * This program performs operations that benefit from cache-aware optimizations
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MATRIX_SIZE 1024
#define BLOCK_SIZE 64  // Common cache line size

/* Matrix multiplication with cache blocking */
void matrix_multiply_blocked(double A[MATRIX_SIZE][MATRIX_SIZE],
                             double B[MATRIX_SIZE][MATRIX_SIZE],
                             double C[MATRIX_SIZE][MATRIX_SIZE]) {
    int i, j, k, ii, jj, kk;
    
    // Zero out result matrix
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            C[i][j] = 0.0;
        }
    }
    
    // Blocked matrix multiplication for better cache utilization
    for (ii = 0; ii < MATRIX_SIZE; ii += BLOCK_SIZE) {
        for (jj = 0; jj < MATRIX_SIZE; jj += BLOCK_SIZE) {
            for (kk = 0; kk < MATRIX_SIZE; kk += BLOCK_SIZE) {
                // Process block
                for (i = ii; i < ii + BLOCK_SIZE && i < MATRIX_SIZE; i++) {
                    for (k = kk; k < kk + BLOCK_SIZE && k < MATRIX_SIZE; k++) {
                        double a = A[i][k];
                        // Prefetch hint for next cache line
                        if (k + 8 < MATRIX_SIZE) {
                            __builtin_prefetch(&B[0][k + 8], 0, 3);
                        }
                        for (j = jj; j < jj + BLOCK_SIZE && j < MATRIX_SIZE; j++) {
                            C[i][j] += a * B[k][j];
                        }
                    }
                }
            }
        }
    }
}

/* Cache-thrashing access pattern (column-major) */
double column_major_sum(double matrix[MATRIX_SIZE][MATRIX_SIZE]) {
    double sum = 0.0;
    int i, j;
    
    // Column-major access (poor cache locality)
    for (j = 0; j < MATRIX_SIZE; j++) {
        for (i = 0; i < MATRIX_SIZE; i++) {
            sum += matrix[i][j];
            // Prefetch hint for next column
            if (i + 4 < MATRIX_SIZE) {
                __builtin_prefetch(&matrix[i + 4][j], 0, 1);
            }
        }
    }
    
    return sum;
}

/* Row-major access pattern (good cache locality) */
double row_major_sum(double matrix[MATRIX_SIZE][MATRIX_SIZE]) {
    double sum = 0.0;
    int i, j;
    
    // Row-major access (good cache locality)
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            sum += matrix[i][j];
            // Prefetch hint for next row element
            if (j + 8 < MATRIX_SIZE) {
                __builtin_prefetch(&matrix[i][j + 8], 0, 3);
            }
        }
    }
    
    return sum;
}

/* Initialize matrices with deterministic values */
void initialize_matrices(double A[MATRIX_SIZE][MATRIX_SIZE],
                         double B[MATRIX_SIZE][MATRIX_SIZE]) {
    int i, j;
    
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            A[i][j] = (i + j) * 0.001;
            B[i][j] = (i - j) * 0.001;
        }
    }
}

/* Compute checksum to prevent dead code elimination */
double compute_checksum(double C[MATRIX_SIZE][MATRIX_SIZE]) {
    double checksum = 0.0;
    int i, j;
    
    // Use different access patterns to exercise various cache behaviors
    for (i = 0; i < MATRIX_SIZE; i += 2) {
        for (j = 0; j < MATRIX_SIZE; j += 2) {
            checksum += C[i][j];
        }
    }
    
    // Strided access pattern
    for (i = 1; i < MATRIX_SIZE; i += 4) {
        for (j = 1; j < MATRIX_SIZE; j += 4) {
            checksum += C[i][j] * 0.5;
        }
    }
    
    return checksum;
}

int main() {
    // Allocate matrices on heap to avoid stack overflow
    double (*A)[MATRIX_SIZE] = malloc(MATRIX_SIZE * sizeof(*A));
    double (*B)[MATRIX_SIZE] = malloc(MATRIX_SIZE * sizeof(*B));
    double (*C)[MATRIX_SIZE] = malloc(MATRIX_SIZE * sizeof(*C));
    
    if (!A || !B || !C) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    printf("Initializing matrices...\n");
    initialize_matrices(A, B);
    
    printf("Performing blocked matrix multiplication...\n");
    matrix_multiply_blocked(A, B, C);
    
    printf("Computing row-major sum...\n");
    double row_sum = row_major_sum(C);
    
    printf("Computing column-major sum...\n");
    double col_sum = column_major_sum(C);
    
    printf("Computing final checksum...\n");
    double checksum = compute_checksum(C);
    
    // Print results to prevent optimization
    printf("Results:\n");
    printf("  Row-major sum: %.6f\n", row_sum);
    printf("  Column-major sum: %.6f\n", col_sum);
    printf("  Final checksum: %.6f\n", checksum);
    printf("  Difference: %.6f\n", row_sum - col_sum);
    
    // Verify computation (not critical, just to use results)
    if (checksum > -1000000.0 && checksum < 1000000.0) {
        printf("Computation completed successfully.\n");
    }
    
    free(A);
    free(B);
    free(C);
    
    return 0;
}

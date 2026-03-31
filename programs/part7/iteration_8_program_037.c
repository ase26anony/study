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
        }
    }
    
    return sum;
}

/* Cache-friendly access pattern (row-major) */
double row_major_sum(double matrix[MATRIX_SIZE][MATRIX_SIZE]) {
    double sum = 0.0;
    int i, j;
    
    // Row-major access (good cache locality)
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            sum += matrix[i][j];
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

/* Compute checksum to prevent optimization */
double compute_checksum(double C[MATRIX_SIZE][MATRIX_SIZE]) {
    double checksum = 0.0;
    int i, j;
    
    for (i = 0; i < MATRIX_SIZE; i += 16) {
        for (j = 0; j < MATRIX_SIZE; j += 16) {
            checksum += C[i][j];
        }
    }
    
    return checksum;
}

int main() {
    // Static allocation to ensure large stack usage
    static double A[MATRIX_SIZE][MATRIX_SIZE];
    static double B[MATRIX_SIZE][MATRIX_SIZE];
    static double C[MATRIX_SIZE][MATRIX_SIZE];
    
    clock_t start, end;
    double cpu_time_used;
    
    printf("Cache Detection Test Program\n");
    printf("Matrix size: %d x %d\n", MATRIX_SIZE, MATRIX_SIZE);
    printf("Block size: %d\n", BLOCK_SIZE);
    
    // Initialize matrices
    initialize_matrices(A, B);
    
    // Time matrix multiplication
    start = clock();
    matrix_multiply_blocked(A, B, C);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Matrix multiplication time: %.2f seconds\n", cpu_time_used);
    
    // Compute and print checksum
    double checksum = compute_checksum(C);
    printf("Result checksum: %.6f\n", checksum);
    
    // Test different access patterns
    start = clock();
    double row_sum = row_major_sum(C);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Row-major sum: %.2f (time: %.4f s)\n", row_sum, cpu_time_used);
    
    start = clock();
    double col_sum = column_major_sum(C);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Column-major sum: %.2f (time: %.4f s)\n", col_sum, cpu_time_used);
    
    // Additional cache-sensitive operations
    printf("\nCache line size test:\n");
    
    // Test with different strides to detect cache effects
    double test_array[MATRIX_SIZE * MATRIX_SIZE];
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        test_array[i] = i * 0.01;
    }
    
    // Access with different strides
    double stride_sum = 0.0;
    for (int stride = 1; stride <= 64; stride *= 2) {
        start = clock();
        for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i += stride) {
            stride_sum += test_array[i];
        }
        end = clock();
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("Stride %2d: time = %.4f s\n", stride, cpu_time_used);
    }
    
    printf("\nTest completed successfully.\n");
    return 0;
}

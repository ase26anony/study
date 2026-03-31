/*
 * Cache-sensitive test program to trigger GCC driver's cache detection logic
 * This program performs operations that benefit from cache-aware optimizations
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MATRIX_SIZE 1024
#define BLOCK_SIZE 64
#define ITERATIONS 3

/* Large matrices to stress cache behavior */
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

/* Standard matrix multiplication - row-major access */
void matmul_standard(void) {
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

/* Cache-blocked matrix multiplication */
void matmul_blocked(void) {
    for (int ii = 0; ii < MATRIX_SIZE; ii += BLOCK_SIZE) {
        for (int jj = 0; jj < MATRIX_SIZE; jj += BLOCK_SIZE) {
            for (int kk = 0; kk < MATRIX_SIZE; kk += BLOCK_SIZE) {
                for (int i = ii; i < ii + BLOCK_SIZE && i < MATRIX_SIZE; i++) {
                    for (int j = jj; j < jj + BLOCK_SIZE && j < MATRIX_SIZE; j++) {
                        /* Use __builtin_prefetch to hint at cache behavior */
                        if (j + 1 < MATRIX_SIZE) {
                            __builtin_prefetch(&matrix_b[kk][j + 1], 0, 3);
                        }
                        if (i + 1 < MATRIX_SIZE && kk + 1 < MATRIX_SIZE) {
                            __builtin_prefetch(&matrix_a[i + 1][kk + 1], 0, 3);
                        }
                        
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

/* Column-major access pattern to test different cache behavior */
void transpose_access(void) {
    for (int j = 0; j < MATRIX_SIZE; j++) {
        for (int i = 0; i < MATRIX_SIZE; i++) {
            matrix_c[i][j] = matrix_a[j][i] + matrix_b[i][j];
        }
    }
}

/* Compute checksum to prevent dead code elimination */
double compute_checksum(void) {
    double checksum = 0.0;
    for (int i = 0; i < MATRIX_SIZE; i += 64) {
        for (int j = 0; j < MATRIX_SIZE; j += 64) {
            checksum += matrix_c[i][j];
        }
    }
    return checksum;
}

/* Large 1D array with strided access patterns */
void strided_access_pattern(void) {
    static double large_array[MATRIX_SIZE * MATRIX_SIZE];
    
    /* Different stride patterns to test cache associativity */
    for (int stride = 1; stride <= 16; stride *= 2) {
        for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i += stride) {
            large_array[i] = large_array[i] * 1.0001 + 0.001;
        }
    }
}

int main(void) {
    clock_t start, end;
    double cpu_time_used;
    double total_checksum = 0.0;
    
    printf("Starting cache-sensitive computation...\n");
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        init_matrices();
        
        start = clock();
        
        /* Mix different access patterns */
        matmul_standard();
        total_checksum += compute_checksum();
        
        matmul_blocked();
        total_checksum += compute_checksum();
        
        transpose_access();
        total_checksum += compute_checksum();
        
        strided_access_pattern();
        
        end = clock();
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
        
        printf("Iteration %d: %.2f seconds, checksum: %.6f\n", 
               iter + 1, cpu_time_used, total_checksum);
    }
    
    printf("Final checksum: %.12f\n", total_checksum);
    printf("Test completed successfully.\n");
    
    return 0;
}

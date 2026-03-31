/*
 * Cache-sensitive test program to trigger GCC driver's cache detection logic
 * This program performs operations that encourage cache-aware optimizations
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MATRIX_SIZE 1024
#define BLOCK_SIZE 32
#define ITERATIONS 5

/* Large multi-dimensional arrays to stress cache */
static double matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_c[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_d[MATRIX_SIZE][MATRIX_SIZE];

/* Different access patterns to test cache behavior */
void row_major_access(double mat[MATRIX_SIZE][MATRIX_SIZE]) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            mat[i][j] = i * MATRIX_SIZE + j;
        }
    }
}

void column_major_access(double mat[MATRIX_SIZE][MATRIX_SIZE]) {
    for (int j = 0; j < MATRIX_SIZE; j++) {
        for (int i = 0; i < MATRIX_SIZE; i++) {
            mat[i][j] = i * MATRIX_SIZE + j;
        }
    }
}

/* Blocked matrix multiplication - cache-aware algorithm */
void blocked_matrix_multiply(int block_size) {
    for (int i0 = 0; i0 < MATRIX_SIZE; i0 += block_size) {
        for (int j0 = 0; j0 < MATRIX_SIZE; j0 += block_size) {
            for (int k0 = 0; k0 < MATRIX_SIZE; k0 += block_size) {
                /* Process block */
                for (int i = i0; i < i0 + block_size && i < MATRIX_SIZE; i++) {
                    for (int k = k0; k < k0 + block_size && k < MATRIX_SIZE; k++) {
                        /* Use __builtin_prefetch to hint at cache behavior */
                        if (k + 1 < MATRIX_SIZE) {
                            __builtin_prefetch(&matrix_b[k + 1][j0], 0, 3);
                        }
                        for (int j = j0; j < j0 + block_size && j < MATRIX_SIZE; j++) {
                            matrix_c[i][j] += matrix_a[i][k] * matrix_b[k][j];
                        }
                    }
                }
            }
        }
    }
}

/* Transpose operation - poor cache locality */
void matrix_transpose(double src[MATRIX_SIZE][MATRIX_SIZE], 
                      double dst[MATRIX_SIZE][MATRIX_SIZE]) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            dst[j][i] = src[i][j];
        }
    }
}

/* Compute checksum to prevent optimization removal */
double compute_checksum(double mat[MATRIX_SIZE][MATRIX_SIZE]) {
    double sum = 0.0;
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            sum += mat[i][j];
        }
    }
    return sum;
}

/* Initialize matrices with deterministic values */
void initialize_matrices() {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = (i + j) % 7;
            matrix_b[i][j] = (i * j) % 11;
            matrix_c[i][j] = 0.0;
            matrix_d[i][j] = 0.0;
        }
    }
}

int main() {
    clock_t start, end;
    double cpu_time_used;
    
    printf("Cache Detection Test Program\n");
    printf("Matrix size: %d x %d\n", MATRIX_SIZE, MATRIX_SIZE);
    
    initialize_matrices();
    
    /* Test different access patterns */
    start = clock();
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Mix different access patterns */
        if (iter % 2 == 0) {
            row_major_access(matrix_d);
        } else {
            column_major_access(matrix_d);
        }
        
        /* Perform blocked matrix multiplication */
        blocked_matrix_multiply(BLOCK_SIZE);
        
        /* Transpose operation */
        matrix_transpose(matrix_a, matrix_d);
    }
    
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    /* Compute and print checksums to ensure computation isn't optimized away */
    double checksum_a = compute_checksum(matrix_a);
    double checksum_b = compute_checksum(matrix_b);
    double checksum_c = compute_checksum(matrix_c);
    
    printf("Checksums:\n");
    printf("  Matrix A: %f\n", checksum_a);
    printf("  Matrix B: %f\n", checksum_b);
    printf("  Matrix C: %f\n", checksum_c);
    printf("Total time: %f seconds\n", cpu_time_used);
    printf("Test completed successfully.\n");
    
    return 0;
}

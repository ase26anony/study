/*
 * Cache-sensitive test program to trigger GCC driver's cache detection logic.
 * This program performs operations that encourage the compiler to consider
 * cache parameters during optimization.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#define MATRIX_SIZE 1024
#define ITERATIONS 5

/* Large multi-dimensional arrays to stress cache considerations */
static double matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_c[MATRIX_SIZE][MATRIX_SIZE];

/* Different access patterns to trigger various cache optimizations */
void row_major_access(double mat[MATRIX_SIZE][MATRIX_SIZE]) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            mat[i][j] = (double)(i * MATRIX_SIZE + j);
        }
    }
}

void column_major_access(double mat[MATRIX_SIZE][MATRIX_SIZE]) {
    for (int j = 0; j < MATRIX_SIZE; j++) {
        for (int i = 0; i < MATRIX_SIZE; i++) {
            mat[i][j] = (double)(i * MATRIX_SIZE + j);
        }
    }
}

/* Matrix multiplication with potential for cache blocking optimization */
void matrix_multiply(double a[MATRIX_SIZE][MATRIX_SIZE],
                     double b[MATRIX_SIZE][MATRIX_SIZE],
                     double c[MATRIX_SIZE][MATRIX_SIZE]) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            c[i][j] = 0.0;
            for (int k = 0; k < MATRIX_SIZE; k++) {
                c[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

/* Function with __builtin_prefetch hints for cache-aware access */
void prefetch_optimized_access(double mat[MATRIX_SIZE][MATRIX_SIZE]) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j += 8) {
            /* Prefetch ahead in the row */
            if (j + 16 < MATRIX_SIZE) {
                __builtin_prefetch(&mat[i][j + 16], 0, 3);
            }
            /* Perform computation */
            mat[i][j] *= 1.0001;
        }
    }
}

/* Blocked matrix multiplication to encourage cache-aware optimization */
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
double compute_checksum(double mat[MATRIX_SIZE][MATRIX_SIZE]) {
    double checksum = 0.0;
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            checksum += mat[i][j];
        }
    }
    return checksum;
}

int main() {
    clock_t start, end;
    double cpu_time_used;
    
    printf("Cache Detection Test Program\n");
    printf("Matrix size: %d x %d\n", MATRIX_SIZE, MATRIX_SIZE);
    
    /* Initialize matrices with different patterns */
    start = clock();
    row_major_access(matrix_a);
    column_major_access(matrix_b);
    
    /* Perform various cache-sensitive operations */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        if (iter % 2 == 0) {
            matrix_multiply(matrix_a, matrix_b, matrix_c);
        } else {
            blocked_matrix_multiply(64); /* Try different block sizes */
        }
        
        prefetch_optimized_access(matrix_c);
    }
    
    /* Compute and print checksum to ensure computation isn't optimized away */
    double checksum = compute_checksum(matrix_c);
    end = clock();
    
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Final checksum: %.6f\n", checksum);
    printf("Total computation time: %.2f seconds\n", cpu_time_used);
    printf("Test completed successfully.\n");
    
    return 0;
}

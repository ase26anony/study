/*
 * Cache-sensitive test program to trigger GCC driver's cache detection logic.
 * This program performs operations that benefit from cache-aware optimizations.
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
static double matrix_d[MATRIX_SIZE][MATRIX_SIZE];

/* Function with nested loops accessing arrays in different patterns */
void matrix_multiply_row_major(double dest[MATRIX_SIZE][MATRIX_SIZE],
                               double a[MATRIX_SIZE][MATRIX_SIZE],
                               double b[MATRIX_SIZE][MATRIX_SIZE]) {
    int i, j, k;
    /* Row-major traversal - cache-friendly */
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            double sum = 0.0;
            for (k = 0; k < MATRIX_SIZE; k++) {
                sum += a[i][k] * b[k][j];
            }
            dest[i][j] = sum;
        }
    }
}

void matrix_multiply_column_major(double dest[MATRIX_SIZE][MATRIX_SIZE],
                                  double a[MATRIX_SIZE][MATRIX_SIZE],
                                  double b[MATRIX_SIZE][MATRIX_SIZE]) {
    int i, j, k;
    /* Column-major traversal - less cache-friendly */
    for (j = 0; j < MATRIX_SIZE; j++) {
        for (i = 0; i < MATRIX_SIZE; i++) {
            double sum = 0.0;
            for (k = 0; k < MATRIX_SIZE; k++) {
                sum += a[i][k] * b[k][j];
            }
            dest[i][j] = sum;
        }
    }
}

/* Function with __builtin_prefetch hints */
void matrix_transpose_with_prefetch(double dest[MATRIX_SIZE][MATRIX_SIZE],
                                    double src[MATRIX_SIZE][MATRIX_SIZE]) {
    int i, j;
    int prefetch_distance = 16; /* Cache line aware prefetch distance */
    
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            /* Prefetch ahead in source matrix */
            if (j + prefetch_distance < MATRIX_SIZE) {
                __builtin_prefetch(&src[i][j + prefetch_distance], 0, 3);
            }
            /* Prefetch ahead in destination matrix */
            if (i + prefetch_distance < MATRIX_SIZE && j < MATRIX_SIZE) {
                __builtin_prefetch(&dest[j][i + prefetch_distance], 1, 3);
            }
            dest[j][i] = src[i][j];
        }
    }
}

/* Blocked matrix multiplication for cache optimization */
void blocked_matrix_multiply(double dest[MATRIX_SIZE][MATRIX_SIZE],
                             double a[MATRIX_SIZE][MATRIX_SIZE],
                             double b[MATRIX_SIZE][MATRIX_SIZE],
                             int block_size) {
    int i, j, k, ii, jj, kk;
    
    for (ii = 0; ii < MATRIX_SIZE; ii += block_size) {
        for (jj = 0; jj < MATRIX_SIZE; jj += block_size) {
            for (kk = 0; kk < MATRIX_SIZE; kk += block_size) {
                /* Process block */
                for (i = ii; i < ii + block_size && i < MATRIX_SIZE; i++) {
                    for (j = jj; j < jj + block_size && j < MATRIX_SIZE; j++) {
                        double sum = dest[i][j];
                        for (k = kk; k < kk + block_size && k < MATRIX_SIZE; k++) {
                            sum += a[i][k] * b[k][j];
                        }
                        dest[i][j] = sum;
                    }
                }
            }
        }
    }
}

/* Compute checksum to prevent dead code elimination */
double compute_checksum(double matrix[MATRIX_SIZE][MATRIX_SIZE]) {
    double checksum = 0.0;
    int i, j;
    
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            checksum += matrix[i][j] * (i + j + 1);
        }
    }
    
    return checksum;
}

int main() {
    int i, j, iter;
    double total_checksum = 0.0;
    clock_t start, end;
    double cpu_time_used;
    
    printf("Cache Detection Test Program\n");
    printf("Matrix size: %d x %d\n", MATRIX_SIZE, MATRIX_SIZE);
    printf("Iterations: %d\n\n", ITERATIONS);
    
    /* Initialize matrices with deterministic values */
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = (i + j) * 0.001;
            matrix_b[i][j] = (i - j) * 0.001;
            matrix_c[i][j] = 0.0;
            matrix_d[i][j] = 0.0;
        }
    }
    
    start = clock();
    
    for (iter = 0; iter < ITERATIONS; iter++) {
        /* Perform various cache-sensitive operations */
        
        /* 1. Row-major matrix multiplication */
        matrix_multiply_row_major(matrix_c, matrix_a, matrix_b);
        total_checksum += compute_checksum(matrix_c);
        
        /* 2. Column-major matrix multiplication */
        matrix_multiply_column_major(matrix_d, matrix_a, matrix_b);
        total_checksum += compute_checksum(matrix_d);
        
        /* 3. Matrix transpose with prefetch hints */
        matrix_transpose_with_prefetch(matrix_c, matrix_a);
        total_checksum += compute_checksum(matrix_c);
        
        /* 4. Blocked matrix multiplication with different block sizes */
        for (i = 0; i < MATRIX_SIZE; i++) {
            for (j = 0; j < MATRIX_SIZE; j++) {
                matrix_c[i][j] = 0.0;
            }
        }
        
        /* Try different block sizes to test various cache configurations */
        int block_sizes[] = {32, 64, 128, 256};
        for (i = 0; i < 4; i++) {
            blocked_matrix_multiply(matrix_c, matrix_a, matrix_b, block_sizes[i]);
            total_checksum += compute_checksum(matrix_c);
        }
        
        /* Modify matrices slightly for next iteration */
        for (i = 0; i < MATRIX_SIZE; i += 64) {
            for (j = 0; j < MATRIX_SIZE; j += 64) {
                matrix_a[i][j] += 0.001;
                matrix_b[i][j] -= 0.001;
            }
        }
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Total checksum: %e\n", total_checksum);
    printf("Time elapsed: %.2f seconds\n", cpu_time_used);
    printf("Operations completed successfully.\n");
    
    /* Return deterministic result for validation */
    if (total_checksum != 0.0) {
        return 0; /* Success */
    } else {
        return 1; /* Error - all computations optimized away */
    }
}

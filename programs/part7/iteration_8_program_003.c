/*
 * Cache-sensitive test program to trigger GCC driver's cache detection logic
 * This program performs operations that benefit from cache-aware optimizations
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MATRIX_SIZE 1024
#define BLOCK_SIZE 64

/* Large multi-dimensional arrays to encourage cache-blocking optimizations */
static double matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_c[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_d[MATRIX_SIZE][MATRIX_SIZE];

/* Function with nested loops accessing arrays in row-major order */
void row_major_multiply(double dest[MATRIX_SIZE][MATRIX_SIZE],
                       double a[MATRIX_SIZE][MATRIX_SIZE],
                       double b[MATRIX_SIZE][MATRIX_SIZE]) {
    int i, j, k;
    
    /* Standard matrix multiplication - row-major access */
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

/* Function with mixed access patterns to test cache behavior */
void mixed_access_patterns(double dest[MATRIX_SIZE][MATRIX_SIZE],
                          double a[MATRIX_SIZE][MATRIX_SIZE],
                          double b[MATRIX_SIZE][MATRIX_SIZE]) {
    int i, j, k;
    
    /* Blocked matrix multiplication for better cache utilization */
    for (i = 0; i < MATRIX_SIZE; i += BLOCK_SIZE) {
        for (j = 0; j < MATRIX_SIZE; j += BLOCK_SIZE) {
            for (k = 0; k < MATRIX_SIZE; k += BLOCK_SIZE) {
                /* Process block */
                for (int ii = i; ii < i + BLOCK_SIZE && ii < MATRIX_SIZE; ii++) {
                    for (int jj = j; jj < j + BLOCK_SIZE && jj < MATRIX_SIZE; jj++) {
                        double sum = dest[ii][jj];
                        for (int kk = k; kk < k + BLOCK_SIZE && kk < MATRIX_SIZE; kk++) {
                            sum += a[ii][kk] * b[kk][jj];
                            
                            /* Use __builtin_prefetch to hint at cache behavior */
                            if (kk + 4 < MATRIX_SIZE) {
                                __builtin_prefetch(&a[ii][kk + 4], 0, 3);
                                __builtin_prefetch(&b[kk + 4][jj], 0, 3);
                            }
                        }
                        dest[ii][jj] = sum;
                    }
                }
            }
        }
    }
}

/* Function with column-major access pattern */
void column_major_transpose(double dest[MATRIX_SIZE][MATRIX_SIZE],
                           double src[MATRIX_SIZE][MATRIX_SIZE]) {
    int i, j;
    
    /* Transpose operation - forces column-major access */
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            dest[j][i] = src[i][j];
        }
    }
}

/* Initialize matrices with deterministic values */
void initialize_matrices(void) {
    int i, j;
    
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = (i + j) * 0.001;
            matrix_b[i][j] = (i - j) * 0.001;
            matrix_c[i][j] = 0.0;
            matrix_d[i][j] = 0.0;
        }
    }
}

/* Compute checksum to prevent dead code elimination */
double compute_checksum(double mat[MATRIX_SIZE][MATRIX_SIZE]) {
    double checksum = 0.0;
    int i, j;
    
    for (i = 0; i < MATRIX_SIZE; i += 8) {
        for (j = 0; j < MATRIX_SIZE; j += 8) {
            checksum += mat[i][j];
        }
    }
    
    return checksum;
}

int main(void) {
    clock_t start, end;
    double cpu_time_used;
    double checksum1, checksum2, checksum3;
    
    printf("Starting cache-sensitive matrix operations...\n");
    
    /* Initialize matrices */
    initialize_matrices();
    
    /* Perform row-major multiplication */
    start = clock();
    row_major_multiply(matrix_c, matrix_a, matrix_b);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Row-major multiplication: %.2f seconds\n", cpu_time_used);
    
    checksum1 = compute_checksum(matrix_c);
    printf("Checksum 1: %.6f\n", checksum1);
    
    /* Perform mixed pattern operations */
    start = clock();
    mixed_access_patterns(matrix_d, matrix_a, matrix_b);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Mixed pattern multiplication: %.2f seconds\n", cpu_time_used);
    
    checksum2 = compute_checksum(matrix_d);
    printf("Checksum 2: %.6f\n", checksum2);
    
    /* Perform column-major transpose */
    start = clock();
    column_major_transpose(matrix_c, matrix_a);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Column-major transpose: %.2f seconds\n", cpu_time_used);
    
    checksum3 = compute_checksum(matrix_c);
    printf("Checksum 3: %.6f\n", checksum3);
    
    /* Final validation */
    if (checksum1 != 0.0 && checksum2 != 0.0 && checksum3 != 0.0) {
        printf("All computations completed successfully.\n");
        return 0;
    } else {
        printf("Error: Some computations resulted in zero checksum.\n");
        return 1;
    }
}

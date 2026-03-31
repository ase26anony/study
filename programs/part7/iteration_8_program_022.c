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
#define BLOCK_SIZE 64

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
    
    /* Row-major traversal (cache-friendly) */
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            dest[i][j] = 0.0;
            for (k = 0; k < MATRIX_SIZE; k++) {
                dest[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

/* Function with column-major access pattern (cache-unfriendly) */
void matrix_multiply_col_major(double dest[MATRIX_SIZE][MATRIX_SIZE],
                               double a[MATRIX_SIZE][MATRIX_SIZE],
                               double b[MATRIX_SIZE][MATRIX_SIZE]) {
    int i, j, k;
    
    /* Column-major traversal (cache-unfriendly) */
    for (j = 0; j < MATRIX_SIZE; j++) {
        for (i = 0; i < MATRIX_SIZE; i++) {
            dest[i][j] = 0.0;
            for (k = 0; k < MATRIX_SIZE; k++) {
                dest[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

/* Cache-blocked matrix multiplication */
void matrix_multiply_blocked(double dest[MATRIX_SIZE][MATRIX_SIZE],
                             double a[MATRIX_SIZE][MATRIX_SIZE],
                             double b[MATRIX_SIZE][MATRIX_SIZE]) {
    int i, j, k, ii, jj, kk;
    
    /* Initialize destination matrix */
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            dest[i][j] = 0.0;
        }
    }
    
    /* Blocked matrix multiplication */
    for (ii = 0; ii < MATRIX_SIZE; ii += BLOCK_SIZE) {
        for (jj = 0; jj < MATRIX_SIZE; jj += BLOCK_SIZE) {
            for (kk = 0; kk < MATRIX_SIZE; kk += BLOCK_SIZE) {
                /* Process block */
                for (i = ii; i < ii + BLOCK_SIZE && i < MATRIX_SIZE; i++) {
                    for (j = jj; j < jj + BLOCK_SIZE && j < MATRIX_SIZE; j++) {
                        double sum = dest[i][j];
                        for (k = kk; k < kk + BLOCK_SIZE && k < MATRIX_SIZE; k++) {
                            sum += a[i][k] * b[k][j];
                        }
                        dest[i][j] = sum;
                    }
                }
            }
        }
    }
}

/* Function using __builtin_prefetch for cache hints */
void prefetch_aware_traversal(double arr[MATRIX_SIZE][MATRIX_SIZE]) {
    int i, j;
    
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE - 8; j += 8) {
            /* Prefetch next cache lines */
            __builtin_prefetch(&arr[i][j + 8], 0, 3);  /* Read, high temporal locality */
            __builtin_prefetch(&arr[i][j + 16], 0, 0); /* Read, low temporal locality */
            
            /* Perform some computation */
            arr[i][j] = arr[i][j] * 1.01 + 0.5;
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
double compute_checksum(double arr[MATRIX_SIZE][MATRIX_SIZE]) {
    double sum = 0.0;
    int i, j;
    
    for (i = 0; i < MATRIX_SIZE; i += 16) {
        for (j = 0; j < MATRIX_SIZE; j += 16) {
            sum += arr[i][j];
        }
    }
    
    return sum;
}

int main(void) {
    clock_t start, end;
    double checksum1, checksum2, checksum3;
    int iter;
    
    printf("Cache Detection Test Program\n");
    printf("Matrix size: %d x %d\n", MATRIX_SIZE, MATRIX_SIZE);
    printf("Block size: %d\n", BLOCK_SIZE);
    printf("Iterations: %d\n\n", ITERATIONS);
    
    /* Initialize matrices */
    initialize_matrices();
    
    /* Time different matrix multiplication approaches */
    start = clock();
    
    for (iter = 0; iter < ITERATIONS; iter++) {
        /* Alternate between different access patterns */
        if (iter % 3 == 0) {
            matrix_multiply_row_major(matrix_c, matrix_a, matrix_b);
        } else if (iter % 3 == 1) {
            matrix_multiply_col_major(matrix_d, matrix_a, matrix_b);
        } else {
            matrix_multiply_blocked(matrix_c, matrix_a, matrix_b);
        }
        
        /* Use prefetch-aware traversal */
        prefetch_aware_traversal(matrix_c);
    }
    
    end = clock();
    
    /* Compute checksums to ensure computations aren't optimized away */
    checksum1 = compute_checksum(matrix_a);
    checksum2 = compute_checksum(matrix_c);
    checksum3 = compute_checksum(matrix_d);
    
    printf("Execution time: %.2f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    printf("Checksums: A=%.6f, C=%.6f, D=%.6f\n", checksum1, checksum2, checksum3);
    printf("Total: %.6f\n", checksum1 + checksum2 + checksum3);
    
    /* Validate result (deterministic based on initialization) */
    if (checksum1 + checksum2 + checksum3 > 1000.0) {
        printf("Result validation: PASS\n");
        return 0;
    } else {
        printf("Result validation: FAIL\n");
        return 1;
    }
}

/*
 * Cache-sensitive test program to trigger GCC driver's cache detection logic
 * This program performs operations that benefit from cache-aware optimizations
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MATRIX_SIZE 1024
#define BLOCK_SIZE 32

/* Large multi-dimensional arrays to encourage cache-blocking optimizations */
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

/* Function with column-major access pattern */
void matrix_transpose(double dest[MATRIX_SIZE][MATRIX_SIZE],
                      double src[MATRIX_SIZE][MATRIX_SIZE]) {
    int i, j;
    
    /* Column-major access - less cache-friendly */
    for (j = 0; j < MATRIX_SIZE; j++) {
        for (i = 0; i < MATRIX_SIZE; i++) {
            dest[j][i] = src[i][j];
        }
    }
}

/* Cache-blocked matrix multiplication */
void matrix_multiply_blocked(double dest[MATRIX_SIZE][MATRIX_SIZE],
                            double a[MATRIX_SIZE][MATRIX_SIZE],
                            double b[MATRIX_SIZE][MATRIX_SIZE]) {
    int i, j, k, ii, jj, kk;
    
    /* Zero the destination matrix */
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

/* Function using __builtin_prefetch for cache-aware access */
void process_array_with_prefetch(double *arr, int size) {
    int i;
    const int prefetch_distance = 16;
    
    for (i = 0; i < size; i++) {
        /* Prefetch ahead to hide memory latency */
        if (i + prefetch_distance < size) {
            __builtin_prefetch(&arr[i + prefetch_distance], 0, 3);
        }
        
        /* Perform some computation */
        arr[i] = arr[i] * 1.01 + 0.5;
    }
}

/* Initialize matrices with deterministic values */
void initialize_matrices(void) {
    int i, j;
    
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = (i + j) * 0.001;
            matrix_b[i][j] = (i - j) * 0.001;
        }
    }
}

/* Compute checksum to prevent dead code elimination */
double compute_checksum(void) {
    double checksum = 0.0;
    int i, j;
    
    for (i = 0; i < MATRIX_SIZE; i += 64) {  /* Sample every 64th element */
        for (j = 0; j < MATRIX_SIZE; j += 64) {
            checksum += matrix_c[i][j] + matrix_d[i][j];
        }
    }
    
    return checksum;
}

int main(void) {
    clock_t start, end;
    double cpu_time_used;
    double checksum;
    
    printf("Starting cache-sensitive matrix operations...\n");
    
    /* Initialize matrices */
    initialize_matrices();
    
    /* Time row-major multiplication */
    start = clock();
    matrix_multiply_row_major(matrix_c, matrix_a, matrix_b);
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Row-major multiplication: %.2f seconds\n", cpu_time_used);
    
    /* Time blocked multiplication */
    start = clock();
    matrix_multiply_blocked(matrix_d, matrix_a, matrix_b);
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Blocked multiplication: %.2f seconds\n", cpu_time_used);
    
    /* Process arrays with prefetch */
    start = clock();
    for (int i = 0; i < MATRIX_SIZE; i++) {
        process_array_with_prefetch(matrix_a[i], MATRIX_SIZE);
    }
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Array processing with prefetch: %.2f seconds\n", cpu_time_used);
    
    /* Compute and print checksum to ensure computations aren't optimized away */
    checksum = compute_checksum();
    printf("Computed checksum: %.6f\n", checksum);
    
    /* Additional operations to increase optimization opportunities */
    {
        /* Transpose operation */
        matrix_transpose(matrix_c, matrix_a);
        
        /* Mixed access patterns */
        double temp[MATRIX_SIZE][MATRIX_SIZE];
        for (int i = 0; i < MATRIX_SIZE; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++) {
                temp[i][j] = matrix_a[j][i] + matrix_b[i][j];
            }
        }
        
        /* Vectorizable loop */
        double sum = 0.0;
        for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
            sum += ((double*)matrix_a)[i] * 0.5;
        }
        printf("Additional sum: %.6f\n", sum);
    }
    
    printf("Test completed successfully.\n");
    return 0;
}

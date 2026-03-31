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

/* Function with row-major access pattern */
void row_major_traversal(int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            sum += matrix_a[i][j];
            /* Hint compiler about prefetching */
            if (j % 16 == 0) {
                __builtin_prefetch(&matrix_a[i][j + 32], 0, 3);
            }
        }
    }
    printf("Row-major sum: %f\n", sum);
}

/* Function with column-major access pattern (cache-unfriendly) */
void column_major_traversal(int n) {
    double sum = 0.0;
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < n; i++) {
            sum += matrix_b[i][j];
            /* Different prefetch pattern */
            if (i % 16 == 0) {
                __builtin_prefetch(&matrix_b[i + 32][j], 0, 3);
            }
        }
    }
    printf("Column-major sum: %f\n", sum);
}

/* Matrix multiplication - highly cache-sensitive */
void matrix_multiply(int n) {
    for (int i = 0; i < n; i++) {
        for (int k = 0; k < n; k++) {
            double aik = matrix_a[i][k];
            for (int j = 0; j < n; j++) {
                matrix_c[i][j] += aik * matrix_b[k][j];
            }
        }
    }
}

/* Blocked matrix multiplication for better cache utilization */
void blocked_matrix_multiply(int n, int block_size) {
    for (int ii = 0; ii < n; ii += block_size) {
        for (int kk = 0; kk < n; kk += block_size) {
            for (int jj = 0; jj < n; jj += block_size) {
                for (int i = ii; i < ii + block_size && i < n; i++) {
                    for (int k = kk; k < kk + block_size && k < n; k++) {
                        double aik = matrix_a[i][k];
                        for (int j = jj; j < jj + block_size && j < n; j++) {
                            matrix_c[i][j] += aik * matrix_b[k][j];
                        }
                    }
                }
            }
        }
    }
}

/* Initialize matrices with deterministic values */
void init_matrices(int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matrix_a[i][j] = (i + j) * 0.001;
            matrix_b[i][j] = (i - j) * 0.001;
            matrix_c[i][j] = 0.0;
        }
    }
}

/* Compute checksum to prevent dead code elimination */
double compute_checksum(int n) {
    double checksum = 0.0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            checksum += matrix_c[i][j];
        }
    }
    return checksum;
}

int main() {
    int n = MATRIX_SIZE;
    
    printf("Initializing matrices...\n");
    init_matrices(n);
    
    printf("Performing cache-sensitive operations...\n");
    
    clock_t start, end;
    double cpu_time_used;
    
    /* Time different access patterns */
    start = clock();
    row_major_traversal(n);
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Row-major traversal time: %f seconds\n", cpu_time_used);
    
    start = clock();
    column_major_traversal(n);
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Column-major traversal time: %f seconds\n", cpu_time_used);
    
    /* Perform matrix multiplication multiple times */
    double total_checksum = 0.0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        init_matrices(n);
        
        start = clock();
        if (iter % 2 == 0) {
            matrix_multiply(n);
        } else {
            blocked_matrix_multiply(n, 64); /* 64-byte blocks for cache lines */
        }
        end = clock();
        
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        printf("Iteration %d time: %f seconds\n", iter, cpu_time_used);
        
        double checksum = compute_checksum(n);
        total_checksum += checksum;
        printf("Checksum: %f\n", checksum);
    }
    
    printf("Total checksum across iterations: %f\n", total_checksum);
    printf("Test completed successfully.\n");
    
    return 0;
}

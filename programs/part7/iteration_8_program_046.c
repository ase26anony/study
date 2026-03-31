/*
 * Cache-sensitive test program to trigger GCC driver's cache detection logic
 * This program uses patterns that encourage the compiler to consider cache
 * characteristics during optimization.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MATRIX_SIZE 1024
#define BLOCK_SIZE 64  /* Typical cache line size */

/* Large multi-dimensional arrays to stress cache considerations */
static double matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_c[MATRIX_SIZE][MATRIX_SIZE];

/* Function with cache-sensitive access patterns */
void matrix_multiply_blocked(int n, int block_size) {
    int i, j, k, ii, jj, kk;
    
    /* Blocked matrix multiplication - optimal block size depends on cache */
    for (i = 0; i < n; i += block_size) {
        for (j = 0; j < n; j += block_size) {
            for (k = 0; k < n; k += block_size) {
                /* Process block */
                for (ii = i; ii < i + block_size && ii < n; ii++) {
                    for (jj = j; jj < j + block_size && jj < n; jj++) {
                        double sum = matrix_c[ii][jj];
                        for (kk = k; kk < k + block_size && kk < n; kk++) {
                            sum += matrix_a[ii][kk] * matrix_b[kk][jj];
                        }
                        matrix_c[ii][jj] = sum;
                    }
                }
            }
        }
    }
}

/* Different access pattern: column-major traversal */
double column_sum(int n) {
    double total = 0.0;
    int i, j;
    
    /* Column-major access - poor for cache but tests detection */
    for (j = 0; j < n; j++) {
        for (i = 0; i < n; i++) {
            total += matrix_a[i][j];
        }
    }
    return total;
}

/* Row-major traversal with prefetch hints */
double row_sum_with_prefetch(int n) {
    double total = 0.0;
    int i, j;
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            /* Use builtin prefetch to hint at cache-aware access */
            if (j + 8 < n) {
                __builtin_prefetch(&matrix_a[i][j + 8], 0, 3);
            }
            total += matrix_a[i][j];
        }
    }
    return total;
}

/* Mixed access patterns to test various cache considerations */
void cache_stress_test(int n) {
    int i, j;
    double temp;
    
    /* Pattern 1: Sequential write */
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            matrix_a[i][j] = (double)(i * n + j);
        }
    }
    
    /* Pattern 2: Transpose (cache-unfriendly) */
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            matrix_b[j][i] = matrix_a[i][j];
        }
    }
    
    /* Pattern 3: Diagonal access */
    for (i = 0; i < n; i++) {
        matrix_c[i][i] = matrix_a[i][i] * 2.0;
    }
    
    /* Pattern 4: Strided access with different strides */
    for (i = 0; i < n; i += 2) {
        for (j = 0; j < n; j += 4) {
            matrix_c[i][j] += matrix_b[i][j];
        }
    }
}

/* Compute checksum to prevent dead code elimination */
double compute_checksum(int n) {
    double checksum = 0.0;
    int i, j;
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            checksum += matrix_c[i][j];
            /* Mix in some non-linear operation */
            checksum = checksum * 0.99 + matrix_a[i][j] * 0.01;
        }
    }
    
    return checksum;
}

int main() {
    int n = MATRIX_SIZE;
    clock_t start, end;
    double cpu_time_used;
    double checksum;
    
    printf("Cache detection test program\n");
    printf("Matrix size: %d x %d\n", n, n);
    printf("Total data: %.2f MB per matrix\n", 
           (double)(n * n * sizeof(double)) / (1024 * 1024));
    
    /* Initialize matrices with deterministic values */
    srand(42);  /* Fixed seed for reproducibility */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matrix_a[i][j] = (double)rand() / RAND_MAX;
            matrix_b[i][j] = (double)rand() / RAND_MAX;
            matrix_c[i][j] = 0.0;
        }
    }
    
    /* Perform cache-sensitive operations */
    start = clock();
    
    /* Test different access patterns */
    cache_stress_test(512);  /* Use smaller size for stress test */
    
    /* Blocked matrix multiplication */
    matrix_multiply_blocked(256, BLOCK_SIZE);
    
    /* Different traversal patterns */
    double col_sum = column_sum(512);
    double row_sum = row_sum_with_prefetch(512);
    
    /* Final computation */
    checksum = compute_checksum(256);
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Computation completed in %.3f seconds\n", cpu_time_used);
    printf("Column sum (512x512): %.6f\n", col_sum);
    printf("Row sum with prefetch (512x512): %.6f\n", row_sum);
    printf("Final checksum: %.12f\n", checksum);
    
    /* Print a deterministic result for validation */
    printf("Validation output: ");
    for (int i = 0; i < 10; i++) {
        printf("%.3f ", matrix_c[i][i]);
    }
    printf("\n");
    
    return 0;
}

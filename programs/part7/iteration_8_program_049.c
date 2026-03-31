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

/* Function with row-major access pattern */
void row_major_multiply(int n, double a[n][n], double b[n][n], double c[n][n]) {
    int i, j, k;
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            double sum = 0.0;
            for (k = 0; k < n; k++) {
                sum += a[i][k] * b[k][j];
            }
            c[i][j] = sum;
        }
    }
}

/* Function with column-major access pattern */
void column_major_multiply(int n, double a[n][n], double b[n][n], double c[n][n]) {
    int i, j, k;
    for (j = 0; j < n; j++) {
        for (i = 0; i < n; i++) {
            double sum = 0.0;
            for (k = 0; k < n; k++) {
                sum += a[i][k] * b[k][j];
            }
            c[i][j] = sum;
        }
    }
}

/* Cache-blocked matrix multiplication */
void blocked_multiply(int n, int block, double a[n][n], double b[n][n], double c[n][n]) {
    int i, j, k, ii, jj, kk;
    
    for (ii = 0; ii < n; ii += block) {
        for (jj = 0; jj < n; jj += block) {
            for (kk = 0; kk < n; kk += block) {
                for (i = ii; i < ii + block && i < n; i++) {
                    for (j = jj; j < jj + block && j < n; j++) {
                        double sum = c[i][j];
                        for (k = kk; k < kk + block && k < n; k++) {
                            sum += a[i][k] * b[k][j];
                        }
                        c[i][j] = sum;
                    }
                }
            }
        }
    }
}

/* Function using __builtin_prefetch for cache-aware access */
void prefetch_aware_transpose(int n, double src[n][n], double dst[n][n]) {
    int i, j;
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            /* Prefetch next row elements */
            if (j + 8 < n) {
                __builtin_prefetch(&src[i][j + 8], 0, 3);
            }
            if (i + 8 < n) {
                __builtin_prefetch(&src[i + 8][j], 0, 3);
            }
            dst[j][i] = src[i][j];
        }
    }
}

/* Initialize matrices with deterministic values */
void init_matrices(int n) {
    int i, j;
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            matrix_a[i][j] = (double)(i + j) / n;
            matrix_b[i][j] = (double)(i * j) / n;
            matrix_c[i][j] = 0.0;
            matrix_d[i][j] = 0.0;
        }
    }
}

/* Compute checksum to prevent dead code elimination */
double compute_checksum(int n, double mat[n][n]) {
    double sum = 0.0;
    int i, j;
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            sum += mat[i][j] * (i + j + 1);
        }
    }
    return sum;
}

int main() {
    const int test_size = 512;  /* Smaller size for reasonable runtime */
    clock_t start, end;
    double cpu_time_used;
    
    printf("Cache-sensitive test program for GCC driver coverage\n");
    printf("Matrix size: %d x %d\n\n", test_size, test_size);
    
    /* Initialize matrices */
    init_matrices(test_size);
    
    /* Test 1: Row-major multiplication */
    printf("Test 1: Row-major matrix multiplication...\n");
    start = clock();
    row_major_multiply(test_size, matrix_a, matrix_b, matrix_c);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Time: %.3f seconds\n", cpu_time_used);
    
    /* Test 2: Column-major multiplication */
    printf("Test 2: Column-major matrix multiplication...\n");
    start = clock();
    column_major_multiply(test_size, matrix_a, matrix_b, matrix_d);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Time: %.3f seconds\n", cpu_time_used);
    
    /* Test 3: Cache-blocked multiplication */
    printf("Test 3: Cache-blocked matrix multiplication...\n");
    start = clock();
    blocked_multiply(test_size, BLOCK_SIZE, matrix_a, matrix_b, matrix_c);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Time: %.3f seconds\n", cpu_time_used);
    
    /* Test 4: Prefetch-aware transpose */
    printf("Test 4: Prefetch-aware matrix transpose...\n");
    start = clock();
    prefetch_aware_transpose(test_size, matrix_a, matrix_d);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Time: %.3f seconds\n", cpu_time_used);
    
    /* Compute and print checksums to ensure computations aren't optimized away */
    double checksum1 = compute_checksum(test_size, matrix_c);
    double checksum2 = compute_checksum(test_size, matrix_d);
    
    printf("\nChecksum results:\n");
    printf("  Matrix C checksum: %.6f\n", checksum1);
    printf("  Matrix D checksum: %.6f\n", checksum2);
    
    /* Simple validation */
    if (checksum1 != 0.0 && checksum2 != 0.0) {
        printf("\nAll tests completed successfully!\n");
        return 0;
    } else {
        printf("\nError: Computations may have been optimized away\n");
        return 1;
    }
}

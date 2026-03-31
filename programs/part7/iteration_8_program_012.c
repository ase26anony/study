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

/* Function with row-major access pattern */
void row_major_multiply(int n, double src[n][n], double dst[n][n]) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                sum += src[i][k] * src[k][j];
            }
            dst[i][j] = sum;
        }
    }
}

/* Function with column-major access pattern */
void column_major_multiply(int n, double src[n][n], double dst[n][n]) {
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < n; i++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                sum += src[i][k] * src[k][j];
            }
            dst[i][j] = sum;
        }
    }
}

/* Cache-blocked matrix multiplication */
void blocked_multiply(int n, double a[n][n], double b[n][n], double c[n][n]) {
    for (int i = 0; i < n; i += BLOCK_SIZE) {
        for (int j = 0; j < n; j += BLOCK_SIZE) {
            for (int k = 0; k < n; k += BLOCK_SIZE) {
                /* Mini-block multiplication */
                for (int ii = i; ii < i + BLOCK_SIZE && ii < n; ii++) {
                    for (int jj = j; jj < j + BLOCK_SIZE && jj < n; jj++) {
                        double sum = c[ii][jj];
                        for (int kk = k; kk < k + BLOCK_SIZE && kk < n; kk++) {
                            sum += a[ii][kk] * b[kk][jj];
                        }
                        c[ii][jj] = sum;
                    }
                }
            }
        }
    }
}

/* Function using __builtin_prefetch for cache-aware access */
void prefetch_optimized_copy(int n, double src[n][n], double dst[n][n]) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j += 8) {
            /* Prefetch ahead in the source array */
            if (j + 16 < n) {
                __builtin_prefetch(&src[i][j + 16], 0, 3);
            }
            /* Prefetch ahead in the destination array */
            if (j + 16 < n) {
                __builtin_prefetch(&dst[i][j + 16], 1, 3);
            }
            /* Copy with potential vectorization */
            for (int k = 0; k < 8 && j + k < n; k++) {
                dst[i][j + k] = src[i][j + k] * 2.0;
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
            matrix_d[i][j] = 0.0;
        }
    }
}

/* Compute checksum to prevent dead code elimination */
double compute_checksum(int n, double mat[n][n]) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            sum += mat[i][j] * (i + j + 1);
        }
    }
    return sum;
}

int main() {
    const int test_size = 512;  /* Smaller size for faster execution */
    clock_t start, end;
    double cpu_time_used;
    
    printf("Cache Detection Test Program\n");
    printf("Matrix size: %d x %d\n", test_size, test_size);
    
    /* Initialize matrices */
    init_matrices(test_size);
    
    /* Test 1: Row-major multiplication */
    printf("\n1. Row-major matrix multiplication...\n");
    start = clock();
    row_major_multiply(test_size, matrix_a, matrix_c);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("   Time: %.3f seconds\n", cpu_time_used);
    
    /* Test 2: Column-major multiplication */
    printf("2. Column-major matrix multiplication...\n");
    start = clock();
    column_major_multiply(test_size, matrix_b, matrix_d);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("   Time: %.3f seconds\n", cpu_time_used);
    
    /* Test 3: Cache-blocked multiplication */
    printf("3. Cache-blocked matrix multiplication...\n");
    start = clock();
    blocked_multiply(test_size, matrix_a, matrix_b, matrix_c);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("   Time: %.3f seconds\n", cpu_time_used);
    
    /* Test 4: Prefetch-optimized copy */
    printf("4. Prefetch-optimized matrix copy...\n");
    start = clock();
    prefetch_optimized_copy(test_size, matrix_a, matrix_d);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("   Time: %.3f seconds\n", cpu_time_used);
    
    /* Compute and print checksums to ensure computations aren't optimized away */
    double checksum1 = compute_checksum(test_size, matrix_c);
    double checksum2 = compute_checksum(test_size, matrix_d);
    
    printf("\nValidation checksums:\n");
    printf("  Matrix C checksum: %.6f\n", checksum1);
    printf("  Matrix D checksum: %.6f\n", checksum2);
    
    /* Simple validation */
    if (checksum1 != 0.0 && checksum2 != 0.0) {
        printf("\nTest completed successfully.\n");
        return 0;
    } else {
        printf("\nError: Computations may have been optimized away.\n");
        return 1;
    }
}

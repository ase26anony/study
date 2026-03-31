/* test_cache_detect.c - Program with cache-sensitive operations
 * to trigger GCC driver's cache detection logic
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MATRIX_SIZE 1024
#define ITERATIONS 5

/* Cache-sensitive matrix operations */
void matrix_multiply(double A[MATRIX_SIZE][MATRIX_SIZE],
                     double B[MATRIX_SIZE][MATRIX_SIZE],
                     double C[MATRIX_SIZE][MATRIX_SIZE]) {
    int i, j, k;
    
    /* Row-major traversal (cache-friendly) */
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (k = 0; k < MATRIX_SIZE; k++) {
            double a = A[i][k];
            /* Prefetch hint for next cache line */
            if (k % 16 == 0) {
                __builtin_prefetch(&B[0][k + 8], 0, 3);
                __builtin_prefetch(&C[i][0], 1, 3);
            }
            for (j = 0; j < MATRIX_SIZE; j++) {
                C[i][j] += a * B[k][j];
            }
        }
    }
}

/* Column-major traversal (cache-unfriendly for comparison) */
void matrix_transpose(double src[MATRIX_SIZE][MATRIX_SIZE],
                      double dst[MATRIX_SIZE][MATRIX_SIZE]) {
    int i, j;
    
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            dst[j][i] = src[i][j];
            /* Prefetch for column access pattern */
            if (j % 8 == 0) {
                __builtin_prefetch(&src[i][j + 4], 0, 0);
                __builtin_prefetch(&dst[j + 4][i], 1, 0);
            }
        }
    }
}

/* Blocked matrix multiplication (cache-aware algorithm) */
void blocked_matrix_multiply(double A[MATRIX_SIZE][MATRIX_SIZE],
                             double B[MATRIX_SIZE][MATRIX_SIZE],
                             double C[MATRIX_SIZE][MATRIX_SIZE],
                             int block_size) {
    int i, j, k, ii, jj, kk;
    
    for (ii = 0; ii < MATRIX_SIZE; ii += block_size) {
        for (jj = 0; jj < MATRIX_SIZE; jj += block_size) {
            for (kk = 0; kk < MATRIX_SIZE; kk += block_size) {
                /* Process block */
                for (i = ii; i < ii + block_size && i < MATRIX_SIZE; i++) {
                    for (k = kk; k < kk + block_size && k < MATRIX_SIZE; k++) {
                        double a = A[i][k];
                        for (j = jj; j < jj + block_size && j < MATRIX_SIZE; j++) {
                            C[i][j] += a * B[k][j];
                        }
                    }
                }
            }
        }
    }
}

/* Initialize matrices with deterministic values */
void init_matrices(double A[MATRIX_SIZE][MATRIX_SIZE],
                   double B[MATRIX_SIZE][MATRIX_SIZE]) {
    int i, j;
    
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            A[i][j] = (i + j) * 0.001;
            B[i][j] = (i - j) * 0.001;
        }
    }
}

/* Compute checksum to prevent optimization */
double compute_checksum(double C[MATRIX_SIZE][MATRIX_SIZE]) {
    double sum = 0.0;
    int i, j;
    
    for (i = 0; i < MATRIX_SIZE; i += 64) {  /* Strided access */
        for (j = 0; j < MATRIX_SIZE; j += 64) {
            sum += C[i][j];
        }
    }
    return sum;
}

int main() {
    /* Static allocation to avoid heap allocation overhead */
    static double A[MATRIX_SIZE][MATRIX_SIZE];
    static double B[MATRIX_SIZE][MATRIX_SIZE];
    static double C1[MATRIX_SIZE][MATRIX_SIZE] = {{0}};
    static double C2[MATRIX_SIZE][MATRIX_SIZE] = {{0}};
    static double C3[MATRIX_SIZE][MATRIX_SIZE] = {{0}};
    
    clock_t start, end;
    double cpu_time_used;
    double checksum1, checksum2, checksum3;
    
    printf("Cache Detection Test Program\n");
    printf("Matrix size: %d x %d\n", MATRIX_SIZE, MATRIX_SIZE);
    
    /* Initialize matrices */
    init_matrices(A, B);
    
    /* Test 1: Standard matrix multiplication */
    printf("\n1. Standard matrix multiplication...\n");
    start = clock();
    matrix_multiply(A, B, C1);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("   Time: %.2f seconds\n", cpu_time_used);
    checksum1 = compute_checksum(C1);
    
    /* Test 2: Blocked matrix multiplication with different block sizes */
    printf("\n2. Blocked matrix multiplication...\n");
    start = clock();
    /* Try different block sizes to test cache sensitivity */
    blocked_matrix_multiply(A, B, C2, 32);   /* Small block */
    blocked_matrix_multiply(A, B, C2, 64);   /* Medium block */
    blocked_matrix_multiply(A, B, C2, 128);  /* Large block */
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("   Time: %.2f seconds\n", cpu_time_used);
    checksum2 = compute_checksum(C2);
    
    /* Test 3: Matrix transpose (cache-unfriendly) */
    printf("\n3. Matrix transpose...\n");
    start = clock();
    matrix_transpose(A, C3);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("   Time: %.2f seconds\n", cpu_time_used);
    checksum3 = compute_checksum(C3);
    
    /* Print results to prevent dead code elimination */
    printf("\nChecksums (for validation):\n");
    printf("  Standard multiply: %.6f\n", checksum1);
    printf("  Blocked multiply:  %.6f\n", checksum2);
    printf("  Transpose:         %.6f\n", checksum3);
    
    /* Additional cache-sensitive operations */
    printf("\nPerforming additional cache tests...\n");
    
    /* Array with different access patterns */
    double test_array[8192];
    double sum = 0.0;
    int i, stride;
    
    /* Sequential access (cache-friendly) */
    for (i = 0; i < 8192; i++) {
        test_array[i] = i * 0.01;
        sum += test_array[i];
    }
    
    /* Strided access with different strides */
    for (stride = 1; stride <= 32; stride *= 2) {
        double stride_sum = 0.0;
        for (i = 0; i < 8192; i += stride) {
            stride_sum += test_array[i];
            /* Prefetch for next stride */
            if (i + stride * 4 < 8192) {
                __builtin_prefetch(&test_array[i + stride * 4], 0, 0);
            }
        }
        sum += stride_sum;
    }
    
    printf("Final validation sum: %.2f\n", sum);
    
    return 0;
}

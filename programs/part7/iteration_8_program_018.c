/* test_cache_detect.c - Cache-sensitive program to trigger GCC driver cache detection */
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
            /* Hint to prefetch B[k][j] for better cache utilization */
            for (j = 0; j < MATRIX_SIZE; j++) {
                C[i][j] += a * B[k][j];
            }
        }
    }
}

/* Column-major traversal (cache-unfriendly for comparison) */
void matrix_transpose_multiply(double A[MATRIX_SIZE][MATRIX_SIZE],
                               double B[MATRIX_SIZE][MATRIX_SIZE],
                               double C[MATRIX_SIZE][MATRIX_SIZE]) {
    int i, j, k;
    
    /* Column-major traversal */
    for (j = 0; j < MATRIX_SIZE; j++) {
        for (k = 0; k < MATRIX_SIZE; k++) {
            double b = B[k][j];
            for (i = 0; i < MATRIX_SIZE; i++) {
                C[i][j] += A[i][k] * b;
            }
        }
    }
}

/* Blocked matrix multiplication for better cache utilization */
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
                        /* Use __builtin_prefetch for cache hints */
                        if (k + 1 < MATRIX_SIZE) {
                            __builtin_prefetch(&B[k+1][jj], 0, 3);
                        }
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
            A[i][j] = (i + j) % 7;
            B[i][j] = (i * j) % 11;
        }
    }
}

/* Compute checksum to prevent optimization */
double compute_checksum(double C[MATRIX_SIZE][MATRIX_SIZE]) {
    double sum = 0.0;
    int i, j;
    for (i = 0; i < MATRIX_SIZE; i += 16) {  /* Strided access */
        for (j = 0; j < MATRIX_SIZE; j += 16) {
            sum += C[i][j];
        }
    }
    return sum;
}

int main() {
    /* Large stack arrays to stress cache considerations */
    static double A[MATRIX_SIZE][MATRIX_SIZE];
    static double B[MATRIX_SIZE][MATRIX_SIZE];
    static double C1[MATRIX_SIZE][MATRIX_SIZE] = {{0}};
    static double C2[MATRIX_SIZE][MATRIX_SIZE] = {{0}};
    static double C3[MATRIX_SIZE][MATRIX_SIZE] = {{0}};
    
    double checksum1, checksum2, checksum3;
    int iter;
    
    /* Initialize matrices */
    init_matrices(A, B);
    
    /* Perform multiple iterations with different cache patterns */
    for (iter = 0; iter < ITERATIONS; iter++) {
        /* Method 1: Standard row-major */
        matrix_multiply(A, B, C1);
        
        /* Method 2: Column-major (different cache pattern) */
        matrix_transpose_multiply(A, B, C2);
        
        /* Method 3: Blocked with varying block sizes */
        int block_size = 32 + (iter % 4) * 16;  /* Vary block size */
        blocked_matrix_multiply(A, B, C3, block_size);
    }
    
    /* Compute checksums to ensure computations aren't optimized away */
    checksum1 = compute_checksum(C1);
    checksum2 = compute_checksum(C2);
    checksum3 = compute_checksum(C3);
    
    /* Print deterministic result */
    printf("Cache test results:\n");
    printf("Row-major checksum: %.2f\n", checksum1);
    printf("Column-major checksum: %.2f\n", checksum2);
    printf("Blocked checksum: %.2f\n", checksum3);
    printf("Total: %.2f\n", checksum1 + checksum2 + checksum3);
    
    return 0;
}

/* test_cache_detect.c - Program designed to trigger GCC driver cache detection */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MATRIX_SIZE 1024
#define ITERATIONS 5
#define BLOCK_SIZE 32

/* Cache-sensitive matrix operations */
void matrix_multiply_blocked(double A[MATRIX_SIZE][MATRIX_SIZE],
                             double B[MATRIX_SIZE][MATRIX_SIZE],
                             double C[MATRIX_SIZE][MATRIX_SIZE]) {
    int i, j, k, ii, jj, kk;
    
    /* Blocked matrix multiplication for better cache utilization */
    for (i = 0; i < MATRIX_SIZE; i += BLOCK_SIZE) {
        for (j = 0; j < MATRIX_SIZE; j += BLOCK_SIZE) {
            for (k = 0; k < MATRIX_SIZE; k += BLOCK_SIZE) {
                /* Process block */
                for (ii = i; ii < i + BLOCK_SIZE && ii < MATRIX_SIZE; ii++) {
                    for (jj = j; jj < j + BLOCK_SIZE && jj < MATRIX_SIZE; jj++) {
                        double sum = C[ii][jj];
                        for (kk = k; kk < k + BLOCK_SIZE && kk < MATRIX_SIZE; kk++) {
                            sum += A[ii][kk] * B[kk][jj];
                        }
                        C[ii][jj] = sum;
                    }
                }
            }
        }
    }
}

/* Different access patterns to stress cache */
double row_major_access(double matrix[MATRIX_SIZE][MATRIX_SIZE]) {
    double sum = 0.0;
    int i, j;
    
    /* Row-major access (cache-friendly) */
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            sum += matrix[i][j];
            /* Use __builtin_prefetch to hint at cache behavior */
            if (j + 4 < MATRIX_SIZE) {
                __builtin_prefetch(&matrix[i][j + 4], 0, 3);
            }
        }
    }
    return sum;
}

double column_major_access(double matrix[MATRIX_SIZE][MATRIX_SIZE]) {
    double sum = 0.0;
    int i, j;
    
    /* Column-major access (cache-unfriendly) */
    for (j = 0; j < MATRIX_SIZE; j++) {
        for (i = 0; i < MATRIX_SIZE; i++) {
            sum += matrix[i][j];
            /* Prefetch for column access */
            if (i + 4 < MATRIX_SIZE) {
                __builtin_prefetch(&matrix[i + 4][j], 0, 3);
            }
        }
    }
    return sum;
}

/* Initialize matrices with deterministic values */
void init_matrices(double A[MATRIX_SIZE][MATRIX_SIZE],
                   double B[MATRIX_SIZE][MATRIX_SIZE],
                   double C[MATRIX_SIZE][MATRIX_SIZE]) {
    int i, j;
    
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            A[i][j] = (i + j) * 0.001;
            B[i][j] = (i - j) * 0.001;
            C[i][j] = 0.0;
        }
    }
}

/* Compute checksum to prevent optimization */
double compute_checksum(double matrix[MATRIX_SIZE][MATRIX_SIZE]) {
    double checksum = 0.0;
    int i, j;
    
    for (i = 0; i < MATRIX_SIZE; i += 8) {
        for (j = 0; j < MATRIX_SIZE; j += 8) {
            checksum += matrix[i][j] * (i + j);
        }
    }
    return checksum;
}

int main() {
    static double A[MATRIX_SIZE][MATRIX_SIZE];
    static double B[MATRIX_SIZE][MATRIX_SIZE];
    static double C[MATRIX_SIZE][MATRIX_SIZE];
    double row_sum, col_sum, checksum;
    int iter;
    
    printf("Cache detection test program\n");
    printf("Matrix size: %d x %d\n", MATRIX_SIZE, MATRIX_SIZE);
    printf("Block size: %d\n", BLOCK_SIZE);
    
    /* Initialize with deterministic values */
    init_matrices(A, B, C);
    
    /* Perform cache-sensitive operations */
    for (iter = 0; iter < ITERATIONS; iter++) {
        /* Clear result matrix */
        memset(C, 0, sizeof(C));
        
        /* Blocked matrix multiplication */
        matrix_multiply_blocked(A, B, C);
        
        /* Different access patterns */
        row_sum = row_major_access(C);
        col_sum = column_major_access(C);
        
        /* Compute checksum to ensure computation isn't optimized away */
        checksum = compute_checksum(C);
        
        if (iter == ITERATIONS - 1) {
            printf("Iteration %d: row_sum=%.2f, col_sum=%.2f, checksum=%.6f\n",
                   iter, row_sum, col_sum, checksum);
        }
    }
    
    /* Final validation */
    printf("Test completed successfully.\n");
    printf("Final checksum: %.6f\n", checksum);
    
    return 0;
}

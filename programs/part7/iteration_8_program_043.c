/* test_cache_detect.c - Program with cache-sensitive operations
 * to trigger GCC driver's cache detection logic */
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
            if (k % 8 == 0 && k + 8 < MATRIX_SIZE) {
                __builtin_prefetch(&B[0][k+8], 0, 3);
            }
            for (j = 0; j < MATRIX_SIZE; j++) {
                C[i][j] += a * B[k][j];
            }
        }
    }
}

/* Column-major traversal (cache-unfriendly for comparison) */
double matrix_sum_column_major(double M[MATRIX_SIZE][MATRIX_SIZE]) {
    double sum = 0.0;
    int i, j;
    
    for (j = 0; j < MATRIX_SIZE; j++) {
        for (i = 0; i < MATRIX_SIZE; i++) {
            sum += M[i][j];
        }
    }
    return sum;
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
                        /* Prefetch for next iteration */
                        if ((k - kk) % 4 == 0 && k + 4 < MATRIX_SIZE) {
                            __builtin_prefetch(&B[0][k+4], 0, 1);
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
            A[i][j] = (i + j) * 0.001;
            B[i][j] = (i - j) * 0.001;
        }
    }
}

int main() {
    /* Large stack arrays to encourage cache-sensitive optimizations */
    static double A[MATRIX_SIZE][MATRIX_SIZE];
    static double B[MATRIX_SIZE][MATRIX_SIZE];
    static double C[MATRIX_SIZE][MATRIX_SIZE] = {{0.0}};
    static double D[MATRIX_SIZE][MATRIX_SIZE] = {{0.0}};
    
    clock_t start, end;
    double cpu_time_used;
    double checksum = 0.0;
    
    printf("Cache Detection Test Program\n");
    printf("Matrix size: %d x %d\n", MATRIX_SIZE, MATRIX_SIZE);
    
    /* Initialize matrices */
    init_matrices(A, B);
    
    /* Test 1: Standard matrix multiplication */
    printf("\nTest 1: Standard matrix multiplication\n");
    start = clock();
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        matrix_multiply(A, B, C);
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Time: %.2f seconds\n", cpu_time_used);
    
    /* Test 2: Blocked matrix multiplication with different block sizes */
    printf("\nTest 2: Blocked matrix multiplication\n");
    
    /* Try different block sizes to test cache-aware compilation */
    int block_sizes[] = {32, 64, 128, 256};
    for (int bs_idx = 0; bs_idx < 4; bs_idx++) {
        start = clock();
        
        /* Reset result matrix */
        for (int i = 0; i < MATRIX_SIZE; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++) {
                D[i][j] = 0.0;
            }
        }
        
        blocked_matrix_multiply(A, B, D, block_sizes[bs_idx]);
        
        end = clock();
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("Block size %d: %.2f seconds\n", 
               block_sizes[bs_idx], cpu_time_used);
    }
    
    /* Test 3: Column-major access pattern */
    printf("\nTest 3: Column-major traversal\n");
    start = clock();
    
    double sum = matrix_sum_column_major(C);
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Sum: %.2f, Time: %.2f seconds\n", sum, cpu_time_used);
    
    /* Compute final checksum to prevent dead code elimination */
    for (int i = 0; i < MATRIX_SIZE; i += 64) {
        for (int j = 0; j < MATRIX_SIZE; j += 64) {
            checksum += C[i][j] + D[i][j];
        }
    }
    
    printf("\nFinal checksum: %.6f\n", checksum);
    printf("Test completed successfully.\n");
    
    return 0;
}

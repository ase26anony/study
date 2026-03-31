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
#define CACHE_LINE_SIZE 64

/* Matrix multiplication with different access patterns */
void matrix_multiply(double A[MATRIX_SIZE][MATRIX_SIZE],
                     double B[MATRIX_SIZE][MATRIX_SIZE],
                     double C[MATRIX_SIZE][MATRIX_SIZE]) {
    int i, j, k;
    
    /* Row-major access pattern */
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            double sum = 0.0;
            for (k = 0; k < MATRIX_SIZE; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}

/* Cache-blocked matrix multiplication */
void blocked_matrix_multiply(double A[MATRIX_SIZE][MATRIX_SIZE],
                             double B[MATRIX_SIZE][MATRIX_SIZE],
                             double C[MATRIX_SIZE][MATRIX_SIZE],
                             int block_size) {
    int i, j, k, ii, jj, kk;
    
    for (i = 0; i < MATRIX_SIZE; i += block_size) {
        for (j = 0; j < MATRIX_SIZE; j += block_size) {
            for (k = 0; k < MATRIX_SIZE; k += block_size) {
                /* Process block */
                for (ii = i; ii < i + block_size && ii < MATRIX_SIZE; ii++) {
                    for (jj = j; jj < j + block_size && jj < MATRIX_SIZE; jj++) {
                        double sum = C[ii][jj];
                        for (kk = k; kk < k + block_size && kk < MATRIX_SIZE; kk++) {
                            sum += A[ii][kk] * B[kk][jj];
                        }
                        C[ii][jj] = sum;
                    }
                }
            }
        }
    }
}

/* Column-major access pattern */
void column_major_traversal(double matrix[MATRIX_SIZE][MATRIX_SIZE]) {
    int i, j;
    double sum = 0.0;
    
    for (j = 0; j < MATRIX_SIZE; j++) {
        for (i = 0; i < MATRIX_SIZE; i++) {
            sum += matrix[i][j];
        }
    }
    
    /* Use the sum to prevent optimization */
    if (sum < 0) {
        printf("Impossible negative sum\n");
    }
}

/* Use __builtin_prefetch for cache hints */
void prefetch_aware_access(double* data, int size) {
    int i;
    for (i = 0; i < size - CACHE_LINE_SIZE/sizeof(double); i += CACHE_LINE_SIZE/sizeof(double)) {
        /* Prefetch next cache line */
        __builtin_prefetch(&data[i + CACHE_LINE_SIZE/sizeof(double)], 0, 3);
        
        /* Process current cache line */
        double sum = 0.0;
        int j;
        for (j = 0; j < CACHE_LINE_SIZE/sizeof(double) && (i + j) < size; j++) {
            sum += data[i + j];
        }
        
        /* Store result back */
        if (i % (CACHE_LINE_SIZE * 4) == 0) {
            data[i] = sum;
        }
    }
}

/* Generate deterministic checksum for validation */
double compute_checksum(double A[MATRIX_SIZE][MATRIX_SIZE],
                        double B[MATRIX_SIZE][MATRIX_SIZE],
                        double C[MATRIX_SIZE][MATRIX_SIZE]) {
    double checksum = 0.0;
    int i, j;
    
    for (i = 0; i < MATRIX_SIZE; i += 16) {
        for (j = 0; j < MATRIX_SIZE; j += 16) {
            checksum += A[i][j] + B[j][i] + C[i][j];
        }
    }
    
    return checksum;
}

int main() {
    /* Allocate matrices on heap to avoid stack overflow */
    double (*A)[MATRIX_SIZE] = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    double (*B)[MATRIX_SIZE] = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    double (*C)[MATRIX_SIZE] = malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    
    if (!A || !B || !C) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize matrices with deterministic values */
    int i, j;
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            A[i][j] = (i + j) * 0.001;
            B[i][j] = (i - j) * 0.001;
            C[i][j] = 0.0;
        }
    }
    
    printf("Starting cache-sensitive computations...\n");
    
    /* Perform various cache-sensitive operations */
    clock_t start = clock();
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Standard matrix multiplication */
        matrix_multiply(A, B, C);
        
        /* Blocked matrix multiplication with different block sizes */
        blocked_matrix_multiply(A, B, C, 32);  /* Small block */
        blocked_matrix_multiply(A, B, C, 64);  /* Medium block */
        blocked_matrix_multiply(A, B, C, 128); /* Large block */
        
        /* Different access patterns */
        column_major_traversal(A);
        column_major_traversal(B);
        
        /* Prefetch-aware access */
        prefetch_aware_access((double*)A, MATRIX_SIZE * MATRIX_SIZE);
        prefetch_aware_access((double*)B, MATRIX_SIZE * MATRIX_SIZE);
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    /* Compute and print checksum for validation */
    double checksum = compute_checksum(A, B, C);
    printf("Computation completed in %.2f seconds\n", elapsed);
    printf("Checksum: %.6f\n", checksum);
    printf("Matrix size: %dx%d, Iterations: %d\n", 
           MATRIX_SIZE, MATRIX_SIZE, ITERATIONS);
    
    /* Clean up */
    free(A);
    free(B);
    free(C);
    
    return 0;
}

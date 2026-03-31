/* test_cache_detect.c - Program with cache-sensitive operations
 * to trigger GCC driver's cache detection logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

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

/* Blocked matrix transpose for cache optimization */
void blocked_transpose(double src[MATRIX_SIZE][MATRIX_SIZE],
                       double dst[MATRIX_SIZE][MATRIX_SIZE],
                       int block_size) {
    int i, j, ii, jj;
    
    for (i = 0; i < MATRIX_SIZE; i += block_size) {
        for (j = 0; j < MATRIX_SIZE; j += block_size) {
            /* Process block */
            for (ii = i; ii < i + block_size && ii < MATRIX_SIZE; ii++) {
                /* Prefetch next row */
                if (ii % 8 == 0) {
                    __builtin_prefetch(&src[ii + 4][0], 0, 3);
                }
                for (jj = j; jj < j + block_size && jj < MATRIX_SIZE; jj++) {
                    dst[jj][ii] = src[ii][jj];
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
            A[i][j] = (i * MATRIX_SIZE + j) * 0.0001;
            B[i][j] = (i * MATRIX_SIZE + j) * 0.0002;
        }
    }
}

int main() {
    /* Large stack arrays to encourage cache-sensitive optimizations */
    double A[MATRIX_SIZE][MATRIX_SIZE];
    double B[MATRIX_SIZE][MATRIX_SIZE];
    double C[MATRIX_SIZE][MATRIX_SIZE] = {{0.0}};
    double D[MATRIX_SIZE][MATRIX_SIZE];
    
    clock_t start, end;
    double cpu_time_used;
    double checksum = 0.0;
    
    printf("Cache Detection Test Program\n");
    printf("Matrix size: %d x %d\n", MATRIX_SIZE, MATRIX_SIZE);
    
    /* Initialize matrices */
    init_matrices(A, B);
    
    /* Perform matrix multiplication (triggers cache-aware optimizations) */
    start = clock();
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        matrix_multiply(A, B, C);
        
        /* Vary access patterns each iteration */
        if (iter % 2 == 0) {
            checksum += matrix_sum_column_major(C);
        } else {
            /* Blocked transpose with different block sizes */
            int block_size = 32 + (iter * 16) % 64;
            blocked_transpose(C, D, block_size);
            
            /* Sum elements from transposed matrix */
            for (int i = 0; i < MATRIX_SIZE; i += 8) {
                for (int j = 0; j < MATRIX_SIZE; j += 8) {
                    checksum += D[i][j];
                }
            }
        }
        
        /* Modify matrices slightly for next iteration */
        for (int i = 0; i < MATRIX_SIZE; i += 128) {
            A[i][i] += 0.001;
            B[i][i] += 0.002;
        }
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Computation completed in %.2f seconds\n", cpu_time_used);
    printf("Final checksum: %.12f\n", checksum);
    
    /* Additional cache-sensitive pattern: strided access */
    double stride_sum = 0.0;
    const int stride = 17; /* Prime stride to defeat prefetching */
    
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        int idx = (i * stride) % (MATRIX_SIZE * MATRIX_SIZE);
        int row = idx / MATRIX_SIZE;
        int col = idx % MATRIX_SIZE;
        stride_sum += C[row][col];
    }
    
    printf("Strided access sum: %.12f\n", stride_sum);
    
    return 0;
}

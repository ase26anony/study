/**
 * Cache detection test program
 * This program performs cache-sensitive operations that encourage the GCC driver
 * to query CPU cache information during compilation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MATRIX_SIZE 1024
#define BLOCK_SIZE 64
#define ITERATIONS 5

/* Large multi-dimensional arrays to stress cache considerations */
static double matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_c[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_d[MATRIX_SIZE][MATRIX_SIZE];

/* Function prototypes for different access patterns */
void row_major_traversal(double mat[MATRIX_SIZE][MATRIX_SIZE]);
void column_major_traversal(double mat[MATRIX_SIZE][MATRIX_SIZE]);
void block_matrix_multiply(void);
void cache_aware_transpose(void);
void mixed_access_pattern(void);
double compute_checksum(void);

/**
 * Row-major traversal (cache-friendly for C arrays)
 */
void row_major_traversal(double mat[MATRIX_SIZE][MATRIX_SIZE]) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            mat[i][j] = (double)(i * MATRIX_SIZE + j);
            /* Use __builtin_prefetch to hint at cache-aware access */
            if (j + 4 < MATRIX_SIZE) {
                __builtin_prefetch(&mat[i][j + 4], 0, 3);
            }
        }
    }
}

/**
 * Column-major traversal (cache-unfriendly for C arrays)
 * This pattern encourages the compiler to consider cache blocking
 */
void column_major_traversal(double mat[MATRIX_SIZE][MATRIX_SIZE]) {
    for (int j = 0; j < MATRIX_SIZE; j++) {
        for (int i = 0; i < MATRIX_SIZE; i++) {
            mat[i][j] = (double)(j * MATRIX_SIZE + i);
            /* Prefetch hint for column access */
            if (i + 8 < MATRIX_SIZE) {
                __builtin_prefetch(&mat[i + 8][j], 0, 2);
            }
        }
    }
}

/**
 * Blocked matrix multiplication - cache-optimized algorithm
 * This is the type of optimization that benefits from cache size knowledge
 */
void block_matrix_multiply(void) {
    int i, j, k, ii, jj, kk;
    
    for (i = 0; i < MATRIX_SIZE; i += BLOCK_SIZE) {
        for (j = 0; j < MATRIX_SIZE; j += BLOCK_SIZE) {
            for (k = 0; k < MATRIX_SIZE; k += BLOCK_SIZE) {
                /* Process block */
                for (ii = i; ii < i + BLOCK_SIZE && ii < MATRIX_SIZE; ii++) {
                    for (jj = j; jj < j + BLOCK_SIZE && jj < MATRIX_SIZE; jj++) {
                        double sum = matrix_c[ii][jj];
                        for (kk = k; kk < k + BLOCK_SIZE && kk < MATRIX_SIZE; kk++) {
                            sum += matrix_a[ii][kk] * matrix_b[kk][jj];
                        }
                        matrix_c[ii][jj] = sum;
                    }
                }
            }
        }
    }
}

/**
 * Cache-aware matrix transpose
 */
void cache_aware_transpose(void) {
    int i, j, ii, jj;
    
    for (i = 0; i < MATRIX_SIZE; i += BLOCK_SIZE) {
        for (j = 0; j < MATRIX_SIZE; j += BLOCK_SIZE) {
            /* Transpose block */
            for (ii = i; ii < i + BLOCK_SIZE && ii < MATRIX_SIZE; ii++) {
                for (jj = j; jj < j + BLOCK_SIZE && jj < MATRIX_SIZE; jj++) {
                    matrix_d[jj][ii] = matrix_c[ii][jj];
                }
            }
        }
    }
}

/**
 * Mixed access patterns to stress different cache considerations
 */
void mixed_access_pattern(void) {
    /* Strided access */
    for (int stride = 1; stride <= 16; stride *= 2) {
        for (int i = 0; i < MATRIX_SIZE; i += stride) {
            for (int j = 0; j < MATRIX_SIZE; j += stride) {
                matrix_d[i][j] = matrix_a[i][j] * 0.5 + matrix_b[j][i] * 0.5;
            }
        }
    }
    
    /* Random-like access (but deterministic) */
    for (int i = 0; i < MATRIX_SIZE * 10; i++) {
        int idx1 = (i * 13) % MATRIX_SIZE;
        int idx2 = (i * 17) % MATRIX_SIZE;
        matrix_c[idx1][idx2] += 0.01;
    }
}

/**
 * Compute a deterministic checksum to prevent optimization removal
 */
double compute_checksum(void) {
    double checksum = 0.0;
    
    for (int i = 0; i < MATRIX_SIZE; i += 32) {
        for (int j = 0; j < MATRIX_SIZE; j += 32) {
            checksum += matrix_a[i][j] + matrix_b[j][i] + 
                       matrix_c[i][j] + matrix_d[j][i];
        }
    }
    
    return checksum;
}

int main(void) {
    clock_t start, end;
    double cpu_time_used;
    
    printf("Starting cache-sensitive computation...\n");
    start = clock();
    
    /* Initialize matrices with different access patterns */
    row_major_traversal(matrix_a);
    column_major_traversal(matrix_b);
    
    /* Perform cache-sensitive operations multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        block_matrix_multiply();
        cache_aware_transpose();
        mixed_access_pattern();
        
        /* Rotate matrices to vary access patterns */
        double (*temp)[MATRIX_SIZE] = matrix_a;
        matrix_a = matrix_b;
        matrix_b = matrix_c;
        matrix_c = matrix_d;
        matrix_d = temp;
    }
    
    /* Compute final checksum */
    double final_checksum = compute_checksum();
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Computation completed.\n");
    printf("Final checksum: %.6f\n", final_checksum);
    printf("Time elapsed: %.3f seconds\n", cpu_time_used);
    printf("Matrix size: %dx%d (%.1f MB total)\n", 
           MATRIX_SIZE, MATRIX_SIZE,
           (4.0 * MATRIX_SIZE * MATRIX_SIZE * sizeof(double)) / (1024.0 * 1024.0));
    
    return 0;
}

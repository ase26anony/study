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
#define ITERATIONS 5

/* Large multi-dimensional arrays to stress cache */
static double matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_c[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_d[MATRIX_SIZE][MATRIX_SIZE];

/* Different access patterns to trigger various optimizations */
void row_major_access(double mat[MATRIX_SIZE][MATRIX_SIZE]) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            mat[i][j] = i * MATRIX_SIZE + j;
        }
    }
}

void column_major_access(double mat[MATRIX_SIZE][MATRIX_SIZE]) {
    for (int j = 0; j < MATRIX_SIZE; j++) {
        for (int i = 0; i < MATRIX_SIZE; i++) {
            mat[i][j] = i * MATRIX_SIZE + j;
        }
    }
}

/* Blocked matrix multiplication - cache-aware algorithm */
void blocked_matrix_multiply(int block_size) {
    for (int i = 0; i < MATRIX_SIZE; i += block_size) {
        for (int j = 0; j < MATRIX_SIZE; j += block_size) {
            for (int k = 0; k < MATRIX_SIZE; k += block_size) {
                /* Mini matrix multiplication on blocks */
                for (int ii = i; ii < i + block_size && ii < MATRIX_SIZE; ii++) {
                    for (int kk = k; kk < k + block_size && kk < MATRIX_SIZE; kk++) {
                        /* Prefetch hint - encourages GCC to consider cache */
                        __builtin_prefetch(&matrix_a[ii+1][kk], 0, 3);
                        __builtin_prefetch(&matrix_b[kk][j+1], 0, 3);
                        
                        double a = matrix_a[ii][kk];
                        for (int jj = j; jj < j + block_size && jj < MATRIX_SIZE; jj++) {
                            matrix_c[ii][jj] += a * matrix_b[kk][jj];
                        }
                    }
                }
            }
        }
    }
}

/* Transpose operation - poor cache locality */
void matrix_transpose(double src[MATRIX_SIZE][MATRIX_SIZE], 
                      double dst[MATRIX_SIZE][MATRIX_SIZE]) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            dst[j][i] = src[i][j];
        }
    }
}

/* Memory-intensive computation with different strides */
void stride_access_patterns() {
    const int strides[] = {1, 2, 4, 8, 16, 32, 64};
    const int num_strides = sizeof(strides) / sizeof(strides[0]);
    
    for (int s = 0; s < num_strides; s++) {
        int stride = strides[s];
        double sum = 0.0;
        
        /* Access with varying stride to test prefetch and cache behavior */
        for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i += stride) {
            int row = i / MATRIX_SIZE;
            int col = i % MATRIX_SIZE;
            if (row < MATRIX_SIZE && col < MATRIX_SIZE) {
                sum += matrix_a[row][col];
                /* Prefetch ahead based on stride */
                if (i + stride * 4 < MATRIX_SIZE * MATRIX_SIZE) {
                    int prefetch_idx = i + stride * 4;
                    int prefetch_row = prefetch_idx / MATRIX_SIZE;
                    int prefetch_col = prefetch_idx % MATRIX_SIZE;
                    __builtin_prefetch(&matrix_a[prefetch_row][prefetch_col], 0, 1);
                }
            }
        }
        
        /* Use sum to prevent dead code elimination */
        matrix_d[0][0] += sum;
    }
}

/* Compute checksum to validate results and prevent optimization */
double compute_checksum() {
    double checksum = 0.0;
    
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            checksum += matrix_a[i][j] * 0.001;
            checksum += matrix_b[i][j] * 0.001;
            checksum += matrix_c[i][j] * 0.0001;
        }
    }
    
    return checksum;
}

int main() {
    clock_t start, end;
    double cpu_time_used;
    
    printf("Starting cache-sensitive computation...\n");
    
    /* Initialize matrices with different patterns */
    start = clock();
    
    row_major_access(matrix_a);
    column_major_access(matrix_b);
    
    /* Perform blocked matrix multiplication */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        blocked_matrix_multiply(BLOCK_SIZE);
    }
    
    /* Perform transpose operation */
    matrix_transpose(matrix_a, matrix_d);
    
    /* Test different stride patterns */
    stride_access_patterns();
    
    /* Compute and print checksum */
    double checksum = compute_checksum();
    
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    printf("Computation completed in %.2f seconds\n", cpu_time_used);
    printf("Checksum: %.6f\n", checksum);
    printf("Matrix size: %dx%d, Block size: %d, Iterations: %d\n", 
           MATRIX_SIZE, MATRIX_SIZE, BLOCK_SIZE, ITERATIONS);
    
    return 0;
}

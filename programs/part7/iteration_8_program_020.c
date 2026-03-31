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

/* Large multi-dimensional arrays to stress cache */
static double matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_c[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_d[MATRIX_SIZE][MATRIX_SIZE];

/* Different access patterns to test various cache behaviors */
void row_major_access(double mat[MATRIX_SIZE][MATRIX_SIZE]) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            mat[i][j] = (double)(i * MATRIX_SIZE + j);
        }
    }
}

void column_major_access(double mat[MATRIX_SIZE][MATRIX_SIZE]) {
    for (int j = 0; j < MATRIX_SIZE; j++) {
        for (int i = 0; i < MATRIX_SIZE; i++) {
            mat[i][j] = (double)(i * MATRIX_SIZE + j);
        }
    }
}

/* Matrix multiplication with potential for cache blocking */
void matrix_multiply(double a[MATRIX_SIZE][MATRIX_SIZE],
                     double b[MATRIX_SIZE][MATRIX_SIZE],
                     double c[MATRIX_SIZE][MATRIX_SIZE]) {
    int block_size = 64; /* Common cache line size */
    
    for (int i = 0; i < MATRIX_SIZE; i += block_size) {
        for (int j = 0; j < MATRIX_SIZE; j += block_size) {
            for (int k = 0; k < MATRIX_SIZE; k += block_size) {
                /* Mini-block multiplication */
                for (int ii = i; ii < i + block_size && ii < MATRIX_SIZE; ii++) {
                    for (int jj = j; jj < j + block_size && jj < MATRIX_SIZE; jj++) {
                        double sum = 0.0;
                        for (int kk = k; kk < k + block_size && kk < MATRIX_SIZE; kk++) {
                            sum += a[ii][kk] * b[kk][jj];
                        }
                        c[ii][jj] += sum;
                    }
                }
            }
        }
    }
}

/* Function with prefetch hints to trigger cache-aware compilation */
void prefetch_heavy_computation(double mat[MATRIX_SIZE][MATRIX_SIZE]) {
    for (int i = 0; i < MATRIX_SIZE - 1; i++) {
        for (int j = 0; j < MATRIX_SIZE - 1; j++) {
            /* Prefetch next row elements */
            __builtin_prefetch(&mat[i+1][j], 0, 3);
            __builtin_prefetch(&mat[i][j+1], 0, 3);
            
            /* Computation that benefits from cache locality */
            mat[i][j] = mat[i][j] * 1.01 + mat[i+1][j] * 0.99;
        }
    }
}

/* Strided access pattern to test cache associativity */
void strided_access(double mat[MATRIX_SIZE][MATRIX_SIZE], int stride) {
    for (int i = 0; i < MATRIX_SIZE; i += stride) {
        for (int j = 0; j < MATRIX_SIZE; j += stride) {
            mat[i][j] = mat[i][j] * 2.0 - 1.0;
        }
    }
}

/* Compute checksum to prevent dead code elimination */
double compute_checksum(double mat[MATRIX_SIZE][MATRIX_SIZE]) {
    double sum = 0.0;
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            sum += mat[i][j];
        }
    }
    return sum;
}

int main() {
    clock_t start, end;
    double cpu_time_used;
    
    printf("Starting cache-sensitive computations...\n");
    
    /* Initialize matrices with different patterns */
    row_major_access(matrix_a);
    column_major_access(matrix_b);
    
    double total_checksum = 0.0;
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        start = clock();
        
        /* Perform various cache-sensitive operations */
        matrix_multiply(matrix_a, matrix_b, matrix_c);
        
        prefetch_heavy_computation(matrix_c);
        
        /* Test different stride patterns */
        strided_access(matrix_c, 2);
        strided_access(matrix_c, 4);
        strided_access(matrix_c, 8);
        strided_access(matrix_c, 16);
        
        /* More matrix operations */
        for (int i = 0; i < MATRIX_SIZE; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++) {
                matrix_d[i][j] = matrix_c[i][j] * 0.5 + matrix_a[i][j] * 0.3 + matrix_b[i][j] * 0.2;
            }
        }
        
        end = clock();
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        
        double checksum = compute_checksum(matrix_d);
        total_checksum += checksum;
        
        printf("Iteration %d: Time = %.2f seconds, Checksum = %.2f\n", 
               iter + 1, cpu_time_used, checksum);
        
        /* Rotate matrices for next iteration */
        memcpy(matrix_a, matrix_b, sizeof(matrix_a));
        memcpy(matrix_b, matrix_c, sizeof(matrix_b));
        memcpy(matrix_c, matrix_d, sizeof(matrix_c));
    }
    
    printf("\nTotal checksum across all iterations: %.2f\n", total_checksum);
    printf("Test completed successfully.\n");
    
    return 0;
}

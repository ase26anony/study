/*
 * Cache-sensitive test program to trigger GCC driver's cache detection logic
 * This program uses patterns that encourage the compiler to consider cache parameters
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MATRIX_SIZE 1024
#define ITERATIONS 5

/* Large multi-dimensional arrays to stress cache considerations */
static double matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_c[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_d[MATRIX_SIZE][MATRIX_SIZE];

/* Different access patterns to trigger various cache optimizations */
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

/* Blocked matrix multiplication - optimal block size depends on cache */
void blocked_matrix_multiply(int block_size) {
    for (int i0 = 0; i0 < MATRIX_SIZE; i0 += block_size) {
        for (int j0 = 0; j0 < MATRIX_SIZE; j0 += block_size) {
            for (int k0 = 0; k0 < MATRIX_SIZE; k0 += block_size) {
                for (int i = i0; i < i0 + block_size && i < MATRIX_SIZE; i++) {
                    for (int j = j0; j < j0 + block_size && j < MATRIX_SIZE; j++) {
                        for (int k = k0; k < k0 + block_size && k < MATRIX_SIZE; k++) {
                            matrix_c[i][j] += matrix_a[i][k] * matrix_b[k][j];
                        }
                    }
                }
            }
        }
    }
}

/* Function with prefetch hints - encourages compiler to consider cache lines */
void prefetch_heavy_computation(double *data, int size) {
    for (int i = 0; i < size - 8; i += 8) {
        /* Hint compiler about future accesses */
        __builtin_prefetch(&data[i + 16], 0, 3);
        __builtin_prefetch(&data[i + 32], 1, 2);
        
        /* Some computation to ensure code isn't optimized away */
        data[i] = data[i] * 1.01 + data[i + 1] * 0.99;
    }
}

/* Complex loop nest that benefits from cache-aware optimizations */
void complex_access_pattern() {
    double sum = 0.0;
    
    /* Tiled access with multiple arrays */
    for (int tile_i = 0; tile_i < MATRIX_SIZE; tile_i += 64) {
        for (int tile_j = 0; tile_j < MATRIX_SIZE; tile_j += 64) {
            for (int i = tile_i; i < tile_i + 64 && i < MATRIX_SIZE; i++) {
                for (int j = tile_j; j < tile_j + 64 && j < MATRIX_SIZE; j++) {
                    /* Mix of row and column access within tiles */
                    matrix_d[i][j] = matrix_a[i][j] + matrix_b[j][i];
                    
                    /* Occasionally use prefetch */
                    if ((i * j) % 128 == 0) {
                        __builtin_prefetch(&matrix_a[i][(j + 8) % MATRIX_SIZE], 0, 1);
                    }
                }
            }
        }
    }
}

/* Compute checksum to prevent dead code elimination */
double compute_checksum() {
    double checksum = 0.0;
    
    for (int i = 0; i < MATRIX_SIZE; i += 16) {
        for (int j = 0; j < MATRIX_SIZE; j += 16) {
            checksum += matrix_c[i][j] + matrix_d[i][j];
        }
    }
    
    return checksum;
}

int main() {
    clock_t start, end;
    double cpu_time_used;
    
    printf("Cache-sensitive test program for GCC driver coverage\n");
    printf("Matrix size: %d x %d\n", MATRIX_SIZE, MATRIX_SIZE);
    
    /* Initialize matrices with different patterns */
    start = clock();
    
    row_major_access(matrix_a);
    column_major_access(matrix_b);
    
    /* Try different block sizes - compiler may choose optimal based on cache */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        int block_size = 32 << (iter % 3); /* 32, 64, 128 */
        
        /* Reset result matrix */
        memset(matrix_c, 0, sizeof(matrix_c));
        
        blocked_matrix_multiply(block_size);
        
        /* Use prefetch in computation */
        prefetch_heavy_computation((double *)matrix_c, MATRIX_SIZE * MATRIX_SIZE);
    }
    
    complex_access_pattern();
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    double checksum = compute_checksum();
    
    printf("Computation completed in %.2f seconds\n", cpu_time_used);
    printf("Checksum: %e\n", checksum);
    printf("(Note: This value varies based on architecture and optimizations)\n");
    
    return 0;
}

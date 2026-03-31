/**
 * Cache detection test program
 * This program performs cache-sensitive operations to encourage the GCC driver
 * to query CPU cache information during compilation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MATRIX_SIZE 1024
#define ITERATIONS 5
#define BLOCK_SIZE 64

/* Large multi-dimensional arrays to stress cache */
static double matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_c[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_d[MATRIX_SIZE][MATRIX_SIZE];

/* Function prototypes for different access patterns */
void row_major_traversal(int size);
void column_major_traversal(int size);
void block_matrix_multiply(int size, int block);
void random_access_pattern(int size);
void prefetch_optimized_access(int size);

/**
 * Row-major traversal (cache-friendly for C arrays)
 */
void row_major_traversal(int size) {
    double sum = 0.0;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            matrix_a[i][j] = (double)(i + j) * 0.001;
            sum += matrix_a[i][j];
        }
    }
    /* Prevent dead code elimination */
    matrix_a[0][0] = sum / (size * size);
}

/**
 * Column-major traversal (cache-unfriendly for C arrays)
 */
void column_major_traversal(int size) {
    double sum = 0.0;
    for (int j = 0; j < size; j++) {
        for (int i = 0; i < size; i++) {
            matrix_b[i][j] = (double)(i * j) * 0.001;
            sum += matrix_b[i][j];
        }
    }
    /* Prevent dead code elimination */
    matrix_b[0][0] = sum / (size * size);
}

/**
 * Blocked matrix multiplication (cache-aware algorithm)
 */
void block_matrix_multiply(int size, int block) {
    for (int i = 0; i < size; i += block) {
        for (int j = 0; j < size; j += block) {
            for (int k = 0; k < size; k += block) {
                /* Mini matrix multiplication on blocks */
                for (int ii = i; ii < i + block && ii < size; ii++) {
                    for (int jj = j; jj < j + block && jj < size; jj++) {
                        double sum = 0.0;
                        for (int kk = k; kk < k + block && kk < size; kk++) {
                            sum += matrix_a[ii][kk] * matrix_b[kk][jj];
                        }
                        matrix_c[ii][jj] += sum;
                    }
                }
            }
        }
    }
}

/**
 * Random access pattern to stress cache associativity
 */
void random_access_pattern(int size) {
    /* Use a pseudo-random but deterministic pattern */
    unsigned int seed = 123456789;
    for (int i = 0; i < size * size; i++) {
        int row = (seed >> 16) % size;
        int col = seed % size;
        matrix_d[row][col] = (double)seed * 0.0000001;
        seed = seed * 1103515245 + 12345;
    }
}

/**
 * Prefetch-optimized access pattern
 */
void prefetch_optimized_access(int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j += 8) {
            /* Use __builtin_prefetch to hint at cache behavior */
            if (j + 16 < size) {
                __builtin_prefetch(&matrix_a[i][j + 16], 0, 3);
                __builtin_prefetch(&matrix_b[i][j + 16], 1, 3);
            }
            
            /* Perform some computation */
            double sum = 0.0;
            for (int k = 0; k < 8 && j + k < size; k++) {
                sum += matrix_a[i][j + k] * matrix_b[i][j + k];
            }
            matrix_c[i][j % 8] = sum;
        }
    }
}

/**
 * Compute checksum of matrices to ensure computations aren't optimized away
 */
double compute_checksum(void) {
    double checksum = 0.0;
    for (int i = 0; i < MATRIX_SIZE; i += 64) {
        for (int j = 0; j < MATRIX_SIZE; j += 64) {
            checksum += matrix_a[i][j] + matrix_b[i][j] + 
                       matrix_c[i][j] + matrix_d[i][j];
        }
    }
    return checksum;
}

int main(void) {
    clock_t start, end;
    double cpu_time_used;
    
    printf("Cache Detection Test Program\n");
    printf("Matrix size: %d x %d\n", MATRIX_SIZE, MATRIX_SIZE);
    printf("Block size: %d\n", BLOCK_SIZE);
    printf("Iterations: %d\n\n", ITERATIONS);
    
    /* Initialize matrices with deterministic values */
    printf("Initializing matrices...\n");
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = (double)(i + j) * 0.001;
            matrix_b[i][j] = (double)(i * j) * 0.001;
            matrix_c[i][j] = 0.0;
            matrix_d[i][j] = 0.0;
        }
    }
    
    /* Perform cache-sensitive operations */
    printf("Performing cache-sensitive operations...\n");
    
    start = clock();
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        row_major_traversal(MATRIX_SIZE / 2);
        column_major_traversal(MATRIX_SIZE / 2);
        block_matrix_multiply(MATRIX_SIZE / 2, BLOCK_SIZE);
        random_access_pattern(MATRIX_SIZE / 2);
        prefetch_optimized_access(MATRIX_SIZE / 2);
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    /* Compute and print checksum to prevent optimization */
    double checksum = compute_checksum();
    printf("\nComputation complete!\n");
    printf("Time elapsed: %.2f seconds\n", cpu_time_used);
    printf("Checksum: %.12f\n", checksum);
    printf("(Checksum verification: %s)\n", 
           checksum > 0.0 ? "PASS" : "FAIL");
    
    return checksum > 0.0 ? 0 : 1;
}

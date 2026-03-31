/*
 * Cache-sensitive test program to trigger GCC driver's cache detection logic.
 * This program performs operations that encourage the compiler to consider
 * cache parameters during optimization.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MATRIX_SIZE 1024
#define BLOCK_SIZE 32
#define ITERATIONS 5

/* Large multi-dimensional arrays to stress cache */
static double matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_c[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_temp[MATRIX_SIZE][MATRIX_SIZE];

/* Different access patterns to test cache behavior */
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
                    for (int jj = j; jj < j + block_size && jj < MATRIX_SIZE; jj++) {
                        double sum = matrix_c[ii][jj];
                        for (int kk = k; kk < k + block_size && kk < MATRIX_SIZE; kk++) {
                            sum += matrix_a[ii][kk] * matrix_b[kk][jj];
                        }
                        matrix_c[ii][jj] = sum;
                    }
                }
            }
        }
    }
}

/* Function with prefetch hints for cache-aware access */
void prefetch_aware_traversal(double mat[MATRIX_SIZE][MATRIX_SIZE]) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j += 8) {
            /* Prefetch next cache line */
            if (j + 16 < MATRIX_SIZE) {
                __builtin_prefetch(&mat[i][j + 16], 0, 3);
            }
            
            /* Process current elements */
            for (int k = 0; k < 8 && j + k < MATRIX_SIZE; k++) {
                mat[i][j + k] = mat[i][j + k] * 1.0001 + 0.5;
            }
        }
    }
}

/* Compute checksum to prevent dead code elimination */
double compute_checksum(double mat[MATRIX_SIZE][MATRIX_SIZE]) {
    double checksum = 0.0;
    for (int i = 0; i < MATRIX_SIZE; i += 64) {  /* Strided access */
        for (int j = 0; j < MATRIX_SIZE; j += 64) {
            checksum += mat[i][j];
        }
    }
    return checksum;
}

int main() {
    clock_t start, end;
    double cpu_time_used;
    
    printf("Cache detection test program\n");
    printf("Matrix size: %d x %d\n", MATRIX_SIZE, MATRIX_SIZE);
    printf("Block size: %d\n", BLOCK_SIZE);
    printf("Iterations: %d\n\n", ITERATIONS);
    
    /* Initialize matrices with deterministic values */
    srand(42);  /* Fixed seed for reproducibility */
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = (double)rand() / RAND_MAX;
            matrix_b[i][j] = (double)rand() / RAND_MAX;
            matrix_c[i][j] = 0.0;
        }
    }
    
    /* Test different access patterns */
    start = clock();
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        if (iter % 3 == 0) {
            row_major_access(matrix_temp);
        } else if (iter % 3 == 1) {
            column_major_access(matrix_temp);
        } else {
            prefetch_aware_traversal(matrix_temp);
        }
        
        /* Perform blocked matrix multiplication */
        blocked_matrix_multiply(BLOCK_SIZE);
        
        /* Mix access patterns */
        if (iter % 2 == 0) {
            /* Transpose-like operation */
            for (int i = 0; i < MATRIX_SIZE; i++) {
                for (int j = i + 1; j < MATRIX_SIZE; j++) {
                    double temp = matrix_a[i][j];
                    matrix_a[i][j] = matrix_a[j][i];
                    matrix_a[j][i] = temp;
                }
            }
        }
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    /* Compute and print checksum to ensure computations aren't optimized away */
    double checksum_a = compute_checksum(matrix_a);
    double checksum_b = compute_checksum(matrix_b);
    double checksum_c = compute_checksum(matrix_c);
    
    printf("Results:\n");
    printf("  Checksum A: %.6f\n", checksum_a);
    printf("  Checksum B: %.6f\n", checksum_b);
    printf("  Checksum C: %.6f\n", checksum_c);
    printf("  Total checksum: %.6f\n", checksum_a + checksum_b + checksum_c);
    printf("  Execution time: %.2f seconds\n", cpu_time_used);
    
    /* Validate result (simple sanity check) */
    if (checksum_a + checksum_b + checksum_c > 0.0) {
        printf("\nTest PASSED: Computations performed successfully.\n");
        return 0;
    } else {
        printf("\nTest FAILED: All checksums are zero.\n");
        return 1;
    }
}

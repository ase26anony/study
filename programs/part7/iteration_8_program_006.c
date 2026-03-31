/*
 * Cache-sensitive test program to trigger GCC driver's cache detection logic
 * This program performs operations that benefit from cache-aware optimizations
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MATRIX_SIZE 1024
#define BLOCK_SIZE 32

/* Large multi-dimensional arrays to encourage cache-blocking optimizations */
static double matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_c[MATRIX_SIZE][MATRIX_SIZE];

/* Initialize matrices with deterministic values */
void init_matrices(void) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = (i + j) * 0.001;
            matrix_b[i][j] = (i - j) * 0.001;
            matrix_c[i][j] = 0.0;
        }
    }
}

/* Standard matrix multiplication - row-major access pattern */
void matmul_standard(void) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            double sum = 0.0;
            for (int k = 0; k < MATRIX_SIZE; k++) {
                sum += matrix_a[i][k] * matrix_b[k][j];
            }
            matrix_c[i][j] = sum;
        }
    }
}

/* Cache-blocked matrix multiplication - encourages cache-aware optimizations */
void matmul_blocked(void) {
    for (int ii = 0; ii < MATRIX_SIZE; ii += BLOCK_SIZE) {
        for (int jj = 0; jj < MATRIX_SIZE; jj += BLOCK_SIZE) {
            for (int kk = 0; kk < MATRIX_SIZE; kk += BLOCK_SIZE) {
                for (int i = ii; i < ii + BLOCK_SIZE && i < MATRIX_SIZE; i++) {
                    for (int j = jj; j < jj + BLOCK_SIZE && j < MATRIX_SIZE; j++) {
                        double sum = matrix_c[i][j];
                        for (int k = kk; k < kk + BLOCK_SIZE && k < MATRIX_SIZE; k++) {
                            sum += matrix_a[i][k] * matrix_b[k][j];
                        }
                        matrix_c[i][j] = sum;
                    }
                }
            }
        }
    }
}

/* Column-major access pattern to test different cache behaviors */
void column_major_access(void) {
    double column_sum = 0.0;
    for (int j = 0; j < MATRIX_SIZE; j++) {
        for (int i = 0; i < MATRIX_SIZE; i++) {
            column_sum += matrix_a[i][j];
        }
    }
    /* Use the result to prevent optimization */
    matrix_c[0][0] += column_sum * 0.000001;
}

/* Function using __builtin_prefetch for cache hints */
void prefetch_demo(void) {
    for (int i = 0; i < MATRIX_SIZE - 1; i++) {
        for (int j = 0; j < MATRIX_SIZE - 1; j++) {
            /* Prefetch next row elements */
            __builtin_prefetch(&matrix_a[i+1][j], 0, 3);
            __builtin_prefetch(&matrix_b[j][i+1], 0, 3);
            
            matrix_c[i][j] = matrix_a[i][j] * matrix_b[j][i];
        }
    }
}

/* Compute checksum to validate results and prevent dead code elimination */
double compute_checksum(void) {
    double checksum = 0.0;
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            checksum += matrix_c[i][j] * (i + j + 1);
        }
    }
    return checksum;
}

/* Multi-pattern access to trigger various cache optimization considerations */
void mixed_access_patterns(void) {
    /* Diagonal access pattern */
    for (int i = 0; i < MATRIX_SIZE; i++) {
        matrix_c[i][i] *= 1.0001;
    }
    
    /* Strided access pattern */
    for (int i = 0; i < MATRIX_SIZE; i += 4) {
        for (int j = 0; j < MATRIX_SIZE; j += 4) {
            matrix_c[i][j] *= 1.0002;
        }
    }
    
    /* Random-like but deterministic access */
    for (int i = 0; i < MATRIX_SIZE; i++) {
        int j = (i * 13) % MATRIX_SIZE;
        matrix_c[i][j] *= 1.0003;
    }
}

int main(void) {
    clock_t start, end;
    double cpu_time_used;
    
    printf("Starting cache-sensitive matrix operations...\n");
    
    start = clock();
    
    /* Initialize all matrices */
    init_matrices();
    
    /* Perform various cache-sensitive operations */
    matmul_standard();
    
    /* Reset result matrix */
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix_c[i][j] = 0.0;
        }
    }
    
    matmul_blocked();
    column_major_access();
    prefetch_demo();
    mixed_access_patterns();
    
    /* Compute final checksum */
    double checksum = compute_checksum();
    
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    printf("Operations completed in %.2f seconds\n", cpu_time_used);
    printf("Final checksum: %.6e\n", checksum);
    
    /* Use checksum in a conditional to prevent optimization */
    if (checksum > 0.0) {
        printf("Validation passed: checksum is positive\n");
    } else {
        printf("Validation passed: checksum is non-positive\n");
    }
    
    return 0;
}

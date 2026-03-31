/*
 * Cache-sensitive test program to trigger GCC driver's cache detection logic.
 * This program performs operations that benefit from cache-aware optimizations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MATRIX_SIZE 1024
#define BLOCK_SIZE 32

/* Large multi-dimensional arrays to encourage cache-blocking optimizations */
static double matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_c[MATRIX_SIZE][MATRIX_SIZE];

/* Function with nested loops accessing arrays in different patterns */
void matrix_multiply_row_major() {
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

/* Column-major access pattern */
void matrix_multiply_column_major() {
    for (int j = 0; j < MATRIX_SIZE; j++) {
        for (int i = 0; i < MATRIX_SIZE; i++) {
            double sum = 0.0;
            for (int k = 0; k < MATRIX_SIZE; k++) {
                sum += matrix_a[i][k] * matrix_b[k][j];
            }
            matrix_c[i][j] = sum;
        }
    }
}

/* Cache-blocked matrix multiplication */
void matrix_multiply_blocked() {
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

/* Function using __builtin_prefetch for cache-aware access */
void prefetch_test(double* array, int size) {
    for (int i = 0; i < size - 8; i += 8) {
        /* Prefetch data for future iterations */
        __builtin_prefetch(&array[i + 8], 0, 3);  /* Read, high temporal locality */
        __builtin_prefetch(&array[i + 16], 0, 0); /* Read, low temporal locality */
        
        /* Perform some computation */
        array[i] = array[i] * 1.01 + array[i + 1];
    }
}

/* Initialize matrices with deterministic values */
void initialize_matrices() {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = (i + j) * 0.001;
            matrix_b[i][j] = (i - j) * 0.001;
            matrix_c[i][j] = 0.0;
        }
    }
}

/* Compute checksum to prevent optimization */
double compute_checksum() {
    double checksum = 0.0;
    for (int i = 0; i < MATRIX_SIZE; i += 64) {  /* Strided access */
        for (int j = 0; j < MATRIX_SIZE; j += 64) {
            checksum += matrix_c[i][j];
        }
    }
    return checksum;
}

/* Memory-intensive operations with different access patterns */
void memory_access_patterns() {
    const int N = 8192;
    double* linear = (double*)malloc(N * sizeof(double));
    double* random_access = (double*)malloc(N * sizeof(double));
    
    /* Linear access pattern */
    for (int i = 0; i < N; i++) {
        linear[i] = i * 0.5;
    }
    
    /* Strided access pattern */
    for (int i = 0; i < N; i += 16) {
        linear[i] *= 1.1;
    }
    
    /* Random-like access pattern */
    for (int i = 0; i < N; i++) {
        int idx = (i * 97) % N;  /* Pseudo-random index */
        random_access[idx] = linear[i];
    }
    
    prefetch_test(linear, N);
    
    free(linear);
    free(random_access);
}

int main() {
    clock_t start, end;
    double cpu_time_used;
    
    printf("Starting cache-sensitive computations...\n");
    
    initialize_matrices();
    
    /* Time different matrix multiplication patterns */
    start = clock();
    matrix_multiply_row_major();
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Row-major multiplication: %.2f seconds\n", cpu_time_used);
    
    double checksum1 = compute_checksum();
    
    /* Re-initialize for next test */
    initialize_matrices();
    
    start = clock();
    matrix_multiply_column_major();
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Column-major multiplication: %.2f seconds\n", cpu_time_used);
    
    double checksum2 = compute_checksum();
    
    /* Re-initialize for blocked multiplication */
    initialize_matrices();
    
    start = clock();
    matrix_multiply_blocked();
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Blocked multiplication: %.2f seconds\n", cpu_time_used);
    
    double checksum3 = compute_checksum();
    
    /* Perform memory access pattern tests */
    memory_access_patterns();
    
    /* Final validation checksum */
    double final_checksum = checksum1 + checksum2 + checksum3;
    printf("Validation checksum: %.6f\n", final_checksum);
    
    /* Additional loop with potential for vectorization */
    double sum = 0.0;
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            sum += matrix_c[i][j] * 0.0001;
        }
    }
    printf("Final sum: %.6f\n", sum);
    
    return 0;
}

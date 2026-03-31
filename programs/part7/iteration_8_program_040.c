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

/* Column-major access pattern to test different cache behavior */
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
void prefetch_test(double *arr, int size) {
    for (int i = 0; i < size - 8; i += 8) {
        /* Prefetch data for future iterations */
        __builtin_prefetch(&arr[i + 8], 0, 3);  /* Read, high temporal locality */
        __builtin_prefetch(&arr[i + 16], 0, 0); /* Read, low temporal locality */
        
        /* Perform some computation */
        arr[i] = arr[i] * 1.5 + arr[i + 1];
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

/* Compute checksum to prevent dead code elimination */
double compute_checksum() {
    double checksum = 0.0;
    for (int i = 0; i < MATRIX_SIZE; i += 64) {  /* Strided access */
        for (int j = 0; j < MATRIX_SIZE; j += 64) {
            checksum += matrix_c[i][j];
        }
    }
    return checksum;
}

/* Test different access patterns */
void test_access_patterns() {
    /* Row-major traversal */
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] += 1.0;
        }
    }
    
    /* Column-major traversal */
    for (int j = 0; j < MATRIX_SIZE; j++) {
        for (int i = 0; i < MATRIX_SIZE; i++) {
            matrix_b[i][j] += 2.0;
        }
    }
    
    /* Diagonal traversal */
    for (int i = 0; i < MATRIX_SIZE; i++) {
        matrix_c[i][i] += 3.0;
    }
}

int main() {
    clock_t start, end;
    double cpu_time_used;
    
    printf("Cache Detection Test Program\n");
    printf("Matrix size: %d x %d\n", MATRIX_SIZE, MATRIX_SIZE);
    
    /* Initialize with deterministic values */
    initialize_matrices();
    
    /* Test 1: Row-major multiplication */
    start = clock();
    matrix_multiply_row_major();
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Row-major multiplication: %.2f seconds\n", cpu_time_used);
    
    /* Test 2: Column-major multiplication */
    initialize_matrices();
    start = clock();
    matrix_multiply_column_major();
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Column-major multiplication: %.2f seconds\n", cpu_time_used);
    
    /* Test 3: Blocked multiplication */
    initialize_matrices();
    start = clock();
    matrix_multiply_blocked();
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Blocked multiplication: %.2f seconds\n", cpu_time_used);
    
    /* Test 4: Prefetch test */
    start = clock();
    prefetch_test((double*)matrix_a, MATRIX_SIZE * MATRIX_SIZE);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Prefetch test: %.2f seconds\n", cpu_time_used);
    
    /* Test 5: Different access patterns */
    start = clock();
    test_access_patterns();
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Access pattern test: %.2f seconds\n", cpu_time_used);
    
    /* Compute and print checksum to ensure computation isn't optimized away */
    double checksum = compute_checksum();
    printf("Final checksum: %.6f\n", checksum);
    
    /* Additional cache-sensitive operations */
    volatile double* volatile_ptr = (volatile double*)matrix_c;
    for (int i = 0; i < 1000; i++) {
        volatile_ptr[i % MATRIX_SIZE] *= 1.0001;
    }
    
    printf("Test completed successfully.\n");
    return 0;
}

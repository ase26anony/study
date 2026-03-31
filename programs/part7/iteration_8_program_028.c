/*
 * Cache-sensitive test program to trigger GCC driver's cache detection logic
 * This program performs operations that encourage the compiler to consider
 * cache parameters during optimization decisions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MATRIX_SIZE 1024
#define BLOCK_SIZE 32

/* Large multi-dimensional arrays to encourage cache-aware optimizations */
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

/* Cache-blocked matrix multiplication to encourage cache-aware optimizations */
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

/* Function using __builtin_prefetch to hint at cache-aware access patterns */
void prefetch_test(double* array, int size) {
    for (int i = 0; i < size - 8; i += 8) {
        /* Prefetch data that will be needed soon */
        __builtin_prefetch(&array[i + 8], 0, 3);  /* Read, high temporal locality */
        
        /* Simulate some computation */
        array[i] = array[i] * 1.01;
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

/* Compute checksum to ensure computations aren't optimized away */
double compute_checksum() {
    double checksum = 0.0;
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            checksum += matrix_c[i][j];
        }
    }
    return checksum;
}

/* Test different access patterns */
void test_access_patterns() {
    /* Test 1: Row-major access */
    clock_t start = clock();
    matrix_multiply_row_major();
    clock_t end = clock();
    double checksum1 = compute_checksum();
    printf("Row-major: checksum = %.6f, time = %.3f ms\n", 
           checksum1, (double)(end - start) * 1000.0 / CLOCKS_PER_SEC);
    
    /* Test 2: Column-major access */
    initialize_matrices();
    start = clock();
    matrix_multiply_column_major();
    end = clock();
    double checksum2 = compute_checksum();
    printf("Column-major: checksum = %.6f, time = %.3f ms\n", 
           checksum2, (double)(end - start) * 1000.0 / CLOCKS_PER_SEC);
    
    /* Test 3: Cache-blocked access */
    initialize_matrices();
    start = clock();
    matrix_multiply_blocked();
    end = clock();
    double checksum3 = compute_checksum();
    printf("Blocked: checksum = %.6f, time = %.3f ms\n", 
           checksum3, (double)(end - start) * 1000.0 / CLOCKS_PER_SEC);
    
    /* Verify all methods produce similar results */
    double diff1 = checksum1 - checksum2;
    double diff2 = checksum2 - checksum3;
    if (diff1 * diff1 + diff2 * diff2 > 1e-6) {
        printf("WARNING: Significant difference in checksums!\n");
    }
}

/* Additional cache-sensitive operations */
void cache_sensitive_operations() {
    /* Large 1D array for prefetch testing */
    double large_array[8192];
    for (int i = 0; i < 8192; i++) {
        large_array[i] = i * 0.01;
    }
    
    /* Test prefetching */
    prefetch_test(large_array, 8192);
    
    /* Compute sum to prevent dead code elimination */
    double sum = 0.0;
    for (int i = 0; i < 8192; i++) {
        sum += large_array[i];
    }
    printf("Array sum: %.6f\n", sum);
}

int main() {
    printf("Cache-sensitive test program\n");
    printf("Matrix size: %dx%d\n", MATRIX_SIZE, MATRIX_SIZE);
    printf("Block size: %d\n", BLOCK_SIZE);
    
    /* Initialize matrices */
    initialize_matrices();
    
    /* Run cache-sensitive tests */
    test_access_patterns();
    
    /* Additional cache operations */
    cache_sensitive_operations();
    
    printf("Test completed successfully.\n");
    return 0;
}

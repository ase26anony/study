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

/* Large matrices to stress cache behavior */
static double A[MATRIX_SIZE][MATRIX_SIZE];
static double B[MATRIX_SIZE][MATRIX_SIZE];
static double C[MATRIX_SIZE][MATRIX_SIZE];

/* Cache-sensitive matrix multiplication - row-major traversal */
void matrix_multiply_row_major(int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}

/* Cache-sensitive matrix multiplication - column-major traversal */
void matrix_multiply_col_major(int n) {
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < n; i++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}

/* Cache-blocked matrix multiplication (optimized for cache) */
void matrix_multiply_blocked(int n, int block_size) {
    for (int i0 = 0; i0 < n; i0 += block_size) {
        for (int j0 = 0; j0 < n; j0 += block_size) {
            for (int k0 = 0; k0 < n; k0 += block_size) {
                int i_end = (i0 + block_size < n) ? i0 + block_size : n;
                int j_end = (j0 + block_size < n) ? j0 + block_size : n;
                int k_end = (k0 + block_size < n) ? k0 + block_size : n;
                
                for (int i = i0; i < i_end; i++) {
                    for (int j = j0; j < j_end; j++) {
                        double sum = C[i][j];
                        for (int k = k0; k < k_end; k++) {
                            sum += A[i][k] * B[k][j];
                        }
                        C[i][j] = sum;
                    }
                }
            }
        }
    }
}

/* Function with prefetch hints for cache-aware access */
void prefetch_test(double* array, int size) {
    for (int i = 0; i < size - 8; i += 8) {
        /* Prefetch ahead to hide cache miss latency */
        __builtin_prefetch(&array[i + 8], 0, 3);  /* Read, high temporal locality */
        
        /* Perform some computation */
        array[i] = array[i] * 1.01;
    }
}

/* Initialize matrices with deterministic values */
void init_matrices(int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            A[i][j] = (double)((i + j) % 100) / 100.0;
            B[i][j] = (double)((i * j) % 100) / 100.0;
            C[i][j] = 0.0;
        }
    }
}

/* Compute checksum to prevent dead code elimination */
double compute_checksum(int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            sum += C[i][j];
        }
    }
    return sum;
}

/* Test different access patterns */
void test_access_patterns() {
    int small_size = 256;
    double small_array[small_size][small_size];
    
    /* Row-major access */
    for (int i = 0; i < small_size; i++) {
        for (int j = 0; j < small_size; j++) {
            small_array[i][j] = i + j;
        }
    }
    
    /* Column-major access */
    double col_sum = 0.0;
    for (int j = 0; j < small_size; j++) {
        for (int i = 0; i < small_size; i++) {
            col_sum += small_array[i][j];
        }
    }
    
    /* Diagonal access */
    double diag_sum = 0.0;
    for (int i = 0; i < small_size; i++) {
        diag_sum += small_array[i][i];
    }
    
    /* Strided access (every 4th element) */
    double stride_sum = 0.0;
    for (int i = 0; i < small_size; i += 4) {
        for (int j = 0; j < small_size; j += 4) {
            stride_sum += small_array[i][j];
        }
    }
}

int main() {
    clock_t start, end;
    double cpu_time_used;
    
    printf("Cache Detection Test Program\n");
    printf("============================\n\n");
    
    /* Initialize with deterministic values */
    init_matrices(MATRIX_SIZE);
    
    /* Test 1: Row-major multiplication */
    printf("Test 1: Row-major matrix multiplication...\n");
    start = clock();
    matrix_multiply_row_major(512);  /* Use smaller size for quick test */
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Time: %.3f seconds\n", cpu_time_used);
    printf("  Checksum: %.6f\n\n", compute_checksum(512));
    
    /* Re-initialize */
    init_matrices(MATRIX_SIZE);
    
    /* Test 2: Column-major multiplication */
    printf("Test 2: Column-major matrix multiplication...\n");
    start = clock();
    matrix_multiply_col_major(512);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Time: %.3f seconds\n", cpu_time_used);
    printf("  Checksum: %.6f\n\n", compute_checksum(512));
    
    /* Re-initialize */
    init_matrices(MATRIX_SIZE);
    
    /* Test 3: Cache-blocked multiplication */
    printf("Test 3: Cache-blocked matrix multiplication...\n");
    start = clock();
    matrix_multiply_blocked(512, BLOCK_SIZE);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Time: %.3f seconds\n", cpu_time_used);
    printf("  Checksum: %.6f\n\n", compute_checksum(512));
    
    /* Test 4: Prefetch test */
    printf("Test 4: Prefetch optimization test...\n");
    double test_array[10000];
    for (int i = 0; i < 10000; i++) {
        test_array[i] = i * 0.1;
    }
    start = clock();
    prefetch_test(test_array, 10000);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Time: %.6f seconds\n\n", cpu_time_used);
    
    /* Test 5: Different access patterns */
    printf("Test 5: Testing various memory access patterns...\n");
    start = clock();
    test_access_patterns();
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Time: %.6f seconds\n\n", cpu_time_used);
    
    printf("All tests completed successfully!\n");
    
    return 0;
}

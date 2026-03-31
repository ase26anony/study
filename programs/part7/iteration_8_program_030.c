/**
 * Cache detection test program
 * This program performs cache-sensitive operations that encourage
 * the GCC driver to query CPU cache information during optimization.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MATRIX_SIZE 1024
#define ITERATIONS 5
#define BLOCK_SIZE 64  // Typical cache line size

/* Large multi-dimensional arrays to stress cache */
static double matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_c[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_d[MATRIX_SIZE][MATRIX_SIZE];

/* Function prototypes */
void initialize_matrices(void);
double matrix_multiply_row_major(void);
double matrix_multiply_column_major(void);
double matrix_multiply_blocked(void);
void cache_sensitive_operations(void);
double compute_checksum(void);

/**
 * Initialize matrices with deterministic values
 */
void initialize_matrices(void) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = (i + j) * 0.001;
            matrix_b[i][j] = (i - j) * 0.001;
            matrix_c[i][j] = 0.0;
            matrix_d[i][j] = 0.0;
        }
    }
}

/**
 * Standard row-major matrix multiplication
 * Accesses memory in a cache-friendly pattern
 */
double matrix_multiply_row_major(void) {
    double sum = 0.0;
    
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int k = 0; k < MATRIX_SIZE; k++) {
            double aik = matrix_a[i][k];
            for (int j = 0; j < MATRIX_SIZE; j++) {
                matrix_c[i][j] += aik * matrix_b[k][j];
            }
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < MATRIX_SIZE; i += 128) {
        for (int j = 0; j < MATRIX_SIZE; j += 128) {
            sum += matrix_c[i][j];
        }
    }
    
    return sum;
}

/**
 * Column-major matrix multiplication
 * Accesses memory in a cache-unfriendly pattern
 */
double matrix_multiply_column_major(void) {
    double sum = 0.0;
    
    for (int j = 0; j < MATRIX_SIZE; j++) {
        for (int k = 0; k < MATRIX_SIZE; k++) {
            double bkj = matrix_b[k][j];
            for (int i = 0; i < MATRIX_SIZE; i++) {
                matrix_d[i][j] += matrix_a[i][k] * bkj;
            }
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < MATRIX_SIZE; i += 128) {
        for (int j = 0; j < MATRIX_SIZE; j += 128) {
            sum += matrix_d[i][j];
        }
    }
    
    return sum;
}

/**
 * Cache-blocked matrix multiplication
 * Uses blocking to improve cache utilization
 */
double matrix_multiply_blocked(void) {
    double sum = 0.0;
    const int block = BLOCK_SIZE;
    
    for (int ii = 0; ii < MATRIX_SIZE; ii += block) {
        for (int jj = 0; jj < MATRIX_SIZE; jj += block) {
            for (int kk = 0; kk < MATRIX_SIZE; kk += block) {
                for (int i = ii; i < ii + block && i < MATRIX_SIZE; i++) {
                    for (int k = kk; k < kk + block && k < MATRIX_SIZE; k++) {
                        /* Use __builtin_prefetch to hint at cache behavior */
                        if (k + 1 < MATRIX_SIZE) {
                            __builtin_prefetch(&matrix_a[i][k + 1], 0, 3);
                            __builtin_prefetch(&matrix_b[k + 1][jj], 0, 3);
                        }
                        
                        double aik = matrix_a[i][k];
                        for (int j = jj; j < jj + block && j < MATRIX_SIZE; j++) {
                            matrix_c[i][j] += aik * matrix_b[k][j];
                        }
                    }
                }
            }
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < MATRIX_SIZE; i += 64) {
        for (int j = 0; j < MATRIX_SIZE; j += 64) {
            sum += matrix_c[i][j];
        }
    }
    
    return sum;
}

/**
 * Additional cache-sensitive operations
 */
void cache_sensitive_operations(void) {
    /* Large 1D array with strided access patterns */
    double large_array[MATRIX_SIZE * MATRIX_SIZE];
    
    /* Initialize with pattern */
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        large_array[i] = i * 0.0001;
    }
    
    /* Various access patterns to trigger different cache behaviors */
    
    /* Sequential access (cache-friendly) */
    double sum_seq = 0.0;
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        sum_seq += large_array[i];
    }
    
    /* Strided access (may cause cache conflicts) */
    double sum_stride = 0.0;
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i += 17) {
        sum_stride += large_array[i];
    }
    
    /* Random access (cache-unfriendly) */
    double sum_random = 0.0;
    for (int iter = 0; iter < 10000; iter++) {
        int idx = (iter * 97) % (MATRIX_SIZE * MATRIX_SIZE);
        sum_random += large_array[idx];
    }
    
    /* Prevent dead code elimination */
    volatile double dummy = sum_seq + sum_stride + sum_random;
    (void)dummy;
}

/**
 * Compute final checksum to ensure computations aren't optimized away
 */
double compute_checksum(void) {
    double checksum = 0.0;
    
    for (int i = 0; i < MATRIX_SIZE; i += 256) {
        for (int j = 0; j < MATRIX_SIZE; j += 256) {
            checksum += matrix_c[i][j] + matrix_d[i][j];
        }
    }
    
    return checksum;
}

/**
 * Main function with timing and validation
 */
int main(void) {
    clock_t start, end;
    double cpu_time_used;
    double total_checksum = 0.0;
    
    printf("Cache Detection Test Program\n");
    printf("Matrix size: %d x %d\n", MATRIX_SIZE, MATRIX_SIZE);
    printf("Block size: %d\n", BLOCK_SIZE);
    
    /* Initialize matrices */
    start = clock();
    initialize_matrices();
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Initialization time: %.3f seconds\n", cpu_time_used);
    
    /* Perform cache-sensitive operations multiple times */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        printf("\nIteration %d:\n", iter + 1);
        
        /* Row-major multiplication */
        start = clock();
        double sum1 = matrix_multiply_row_major();
        end = clock();
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("  Row-major: %.3f seconds, checksum: %.6f\n", cpu_time_used, sum1);
        total_checksum += sum1;
        
        /* Column-major multiplication */
        start = clock();
        double sum2 = matrix_multiply_column_major();
        end = clock();
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("  Column-major: %.3f seconds, checksum: %.6f\n", cpu_time_used, sum2);
        total_checksum += sum2;
        
        /* Blocked multiplication */
        start = clock();
        double sum3 = matrix_multiply_blocked();
        end = clock();
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("  Blocked: %.3f seconds, checksum: %.6f\n", cpu_time_used, sum3);
        total_checksum += sum3;
        
        /* Additional cache operations */
        start = clock();
        cache_sensitive_operations();
        end = clock();
        cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("  Cache ops: %.3f seconds\n", cpu_time_used);
    }
    
    /* Final checksum */
    double final_checksum = compute_checksum();
    total_checksum += final_checksum;
    
    printf("\nFinal total checksum: %.12f\n", total_checksum);
    printf("Test completed successfully.\n");
    
    /* Return deterministic result for validation */
    if (total_checksum > 0.0) {
        return 0;  /* Success */
    } else {
        return 1;  /* Error - all computations optimized away */
    }
}

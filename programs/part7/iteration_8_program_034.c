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
#define BLOCK_SIZE 64

/* Large multi-dimensional arrays to stress cache */
static double matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_c[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_d[MATRIX_SIZE][MATRIX_SIZE];

/* Function prototypes */
void initialize_matrices(void);
double matrix_multiply_row_major(void);
double matrix_multiply_column_major(void);
void cache_blocked_multiply(void);
void mixed_access_pattern(void);
double compute_checksum(void);

/* Initialize matrices with deterministic values */
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

/* Row-major matrix multiplication - cache-friendly */
double matrix_multiply_row_major(void) {
    double sum = 0.0;
    
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int k = 0; k < MATRIX_SIZE; k++) {
            double aik = matrix_a[i][k];
            /* Use __builtin_prefetch to hint at cache behavior */
            if (k + 4 < MATRIX_SIZE) {
                __builtin_prefetch(&matrix_b[k+4][0], 0, 3);
            }
            for (int j = 0; j < MATRIX_SIZE; j++) {
                matrix_c[i][j] += aik * matrix_b[k][j];
            }
        }
    }
    
    /* Compute checksum */
    for (int i = 0; i < MATRIX_SIZE; i += 64) {
        sum += matrix_c[i][i];
    }
    
    return sum;
}

/* Column-major matrix multiplication - cache-unfriendly */
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
    for (int i = 0; i < MATRIX_SIZE; i += 64) {
        sum += matrix_d[i][i];
    }
    
    return sum;
}

/* Cache-blocked matrix multiplication */
void cache_blocked_multiply(void) {
    const int block = BLOCK_SIZE;
    
    for (int ii = 0; ii < MATRIX_SIZE; ii += block) {
        for (int jj = 0; jj < MATRIX_SIZE; jj += block) {
            for (int kk = 0; kk < MATRIX_SIZE; kk += block) {
                for (int i = ii; i < ii + block && i < MATRIX_SIZE; i++) {
                    for (int k = kk; k < kk + block && k < MATRIX_SIZE; k++) {
                        double aik = matrix_a[i][k];
                        /* Prefetching for next blocks */
                        if (k == kk && i == ii) {
                            __builtin_prefetch(&matrix_a[ii][kk+block], 0, 1);
                            __builtin_prefetch(&matrix_b[kk+block][jj], 0, 1);
                        }
                        for (int j = jj; j < jj + block && j < MATRIX_SIZE; j++) {
                            matrix_c[i][j] += aik * matrix_b[k][j];
                        }
                    }
                }
            }
        }
    }
}

/* Mixed access patterns to trigger various optimizations */
void mixed_access_pattern(void) {
    /* Strided access */
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j += 8) {
            matrix_d[i][j] = matrix_a[i][j] * 2.0;
        }
    }
    
    /* Reverse access */
    for (int i = MATRIX_SIZE - 1; i >= 0; i--) {
        for (int j = MATRIX_SIZE - 1; j >= 0; j--) {
            matrix_c[i][j] += matrix_d[j][i];
        }
    }
    
    /* Diagonal access */
    for (int d = 0; d < MATRIX_SIZE * 2; d++) {
        for (int i = 0; i <= d; i++) {
            int j = d - i;
            if (i < MATRIX_SIZE && j < MATRIX_SIZE) {
                matrix_a[i][j] = matrix_b[j][i];
            }
        }
    }
}

/* Compute final checksum to prevent dead code elimination */
double compute_checksum(void) {
    double checksum = 0.0;
    
    for (int i = 0; i < MATRIX_SIZE; i += 16) {
        for (int j = 0; j < MATRIX_SIZE; j += 16) {
            checksum += matrix_a[i][j] + matrix_b[i][j] + 
                       matrix_c[i][j] + matrix_d[i][j];
        }
    }
    
    return checksum;
}

int main(void) {
    double total_checksum = 0.0;
    clock_t start, end;
    
    printf("Starting cache-sensitive computation...\n");
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        printf("Iteration %d/%d\n", iter + 1, ITERATIONS);
        
        /* Reinitialize matrices each iteration */
        initialize_matrices();
        
        /* Time different access patterns */
        start = clock();
        total_checksum += matrix_multiply_row_major();
        end = clock();
        printf("  Row-major: %.2f seconds\n", 
               (double)(end - start) / CLOCKS_PER_SEC);
        
        start = clock();
        total_checksum += matrix_multiply_column_major();
        end = clock();
        printf("  Column-major: %.2f seconds\n", 
               (double)(end - start) / CLOCKS_PER_SEC);
        
        start = clock();
        cache_blocked_multiply();
        end = clock();
        printf("  Cache-blocked: %.2f seconds\n", 
               (double)(end - start) / CLOCKS_PER_SEC);
        
        start = clock();
        mixed_access_pattern();
        end = clock();
        printf("  Mixed pattern: %.2f seconds\n", 
               (double)(end - start) / CLOCKS_PER_SEC);
    }
    
    /* Final checksum computation */
    double final_checksum = compute_checksum();
    total_checksum += final_checksum;
    
    printf("\nTotal checksum: %e\n", total_checksum);
    printf("Final checksum: %e\n", final_checksum);
    
    /* Print a deterministic result */
    if (total_checksum > 0) {
        printf("SUCCESS: Computation completed with non-zero result\n");
        return 0;
    } else {
        printf("ERROR: Zero checksum detected\n");
        return 1;
    }
}

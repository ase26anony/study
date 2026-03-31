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

/* Large multi-dimensional arrays to stress cache */
static double matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_c[MATRIX_SIZE][MATRIX_SIZE];

/* Different access pattern functions to encourage cache-aware optimizations */

/* Row-major access pattern */
void row_major_traversal(int n) {
    volatile double sum = 0.0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            sum += matrix_a[i][j];
        }
    }
    (void)sum; /* Prevent dead code elimination */
}

/* Column-major access pattern (cache-unfriendly) */
void column_major_traversal(int n) {
    volatile double sum = 0.0;
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < n; i++) {
            sum += matrix_b[i][j];
        }
    }
    (void)sum;
}

/* Cache-blocked matrix multiplication */
void blocked_matrix_multiply(int n, int block_size) {
    for (int i0 = 0; i0 < n; i0 += block_size) {
        for (int j0 = 0; j0 < n; j0 += block_size) {
            for (int k0 = 0; k0 < n; k0 += block_size) {
                /* Process block */
                int i_end = (i0 + block_size < n) ? i0 + block_size : n;
                int j_end = (j0 + block_size < n) ? j0 + block_size : n;
                int k_end = (k0 + block_size < n) ? k0 + block_size : n;
                
                for (int i = i0; i < i_end; i++) {
                    for (int k = k0; k < k_end; k++) {
                        /* Use __builtin_prefetch to hint at cache behavior */
                        if (k + 1 < k_end) {
                            __builtin_prefetch(&matrix_a[i][k + 1], 0, 3);
                            __builtin_prefetch(&matrix_b[k + 1][j0], 0, 3);
                        }
                        
                        double a = matrix_a[i][k];
                        for (int j = j0; j < j_end; j++) {
                            matrix_c[i][j] += a * matrix_b[k][j];
                        }
                    }
                }
            }
        }
    }
}

/* Initialize matrices with deterministic values */
void init_matrices(int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matrix_a[i][j] = (i + j) % 7;
            matrix_b[i][j] = (i * j) % 11;
            matrix_c[i][j] = 0.0;
        }
    }
}

/* Compute checksum to ensure computations aren't optimized away */
double compute_checksum(int n) {
    double checksum = 0.0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            checksum += matrix_c[i][j];
        }
    }
    return checksum;
}

/* Mixed access patterns to stress different cache behaviors */
void mixed_access_patterns(int n) {
    volatile double temp = 0.0;
    
    /* Diagonal access */
    for (int i = 0; i < n; i++) {
        temp += matrix_a[i][i];
    }
    
    /* Strided access */
    for (int i = 0; i < n; i += 4) {
        for (int j = 0; j < n; j += 4) {
            temp += matrix_b[i][j];
        }
    }
    
    /* Random-like but deterministic access */
    for (int i = 0; i < n; i++) {
        int j = (i * 17) % n;
        temp += matrix_c[i][j];
    }
    
    (void)temp;
}

int main(int argc, char *argv[]) {
    int test_size = MATRIX_SIZE;
    
    if (argc > 1) {
        test_size = atoi(argv[1]);
        if (test_size <= 0 || test_size > MATRIX_SIZE) {
            test_size = MATRIX_SIZE;
        }
    }
    
    printf("Initializing matrices (%dx%d)...\n", test_size, test_size);
    init_matrices(test_size);
    
    printf("Performing row-major traversal...\n");
    row_major_traversal(test_size);
    
    printf("Performing column-major traversal...\n");
    column_major_traversal(test_size);
    
    printf("Performing blocked matrix multiplication...\n");
    blocked_matrix_multiply(test_size, BLOCK_SIZE);
    
    printf("Performing mixed access patterns...\n");
    mixed_access_patterns(test_size);
    
    double checksum = compute_checksum(test_size);
    printf("Computation complete. Checksum: %f\n", checksum);
    
    /* Print a deterministic result for validation */
    printf("Result validation: %s\n", 
           (checksum > 0.0) ? "PASS" : "FAIL");
    
    return 0;
}

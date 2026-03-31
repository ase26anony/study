/* test_cache_detect.c - Cache-sensitive program to trigger GCC driver cache detection */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MATRIX_SIZE 1024
#define ITERATIONS 5
#define BLOCK_SIZE 32

/* Large multi-dimensional arrays to encourage cache-aware optimizations */
static double matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_c[MATRIX_SIZE][MATRIX_SIZE];

/* Initialize matrices with deterministic values */
void init_matrices() {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = (i + j) * 0.001;
            matrix_b[i][j] = (i - j) * 0.001;
            matrix_c[i][j] = 0.0;
        }
    }
}

/* Row-major traversal - cache-friendly */
void matrix_multiply_row_major() {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int k = 0; k < MATRIX_SIZE; k++) {
            double aik = matrix_a[i][k];
            /* Prefetch hint for cache-aware access */
            __builtin_prefetch(&matrix_b[k+1][0], 0, 3);
            for (int j = 0; j < MATRIX_SIZE; j++) {
                matrix_c[i][j] += aik * matrix_b[k][j];
            }
        }
    }
}

/* Column-major traversal - cache-unfriendly */
void matrix_multiply_col_major() {
    for (int j = 0; j < MATRIX_SIZE; j++) {
        for (int k = 0; k < MATRIX_SIZE; k++) {
            double bkj = matrix_b[k][j];
            /* Prefetch hint with varying strides */
            __builtin_prefetch(&matrix_a[k+1][j], 0, 2);
            for (int i = 0; i < MATRIX_SIZE; i++) {
                matrix_c[i][j] += matrix_a[i][k] * bkj;
            }
        }
    }
}

/* Blocked matrix multiplication - explicitly cache-aware */
void matrix_multiply_blocked() {
    for (int ii = 0; ii < MATRIX_SIZE; ii += BLOCK_SIZE) {
        for (int jj = 0; jj < MATRIX_SIZE; jj += BLOCK_SIZE) {
            for (int kk = 0; kk < MATRIX_SIZE; kk += BLOCK_SIZE) {
                /* Process block */
                for (int i = ii; i < ii + BLOCK_SIZE && i < MATRIX_SIZE; i++) {
                    for (int k = kk; k < kk + BLOCK_SIZE && k < MATRIX_SIZE; k++) {
                        double aik = matrix_a[i][k];
                        __builtin_prefetch(&matrix_b[k][jj], 0, 3);
                        for (int j = jj; j < jj + BLOCK_SIZE && j < MATRIX_SIZE; j++) {
                            matrix_c[i][j] += aik * matrix_b[k][j];
                        }
                    }
                }
            }
        }
    }
}

/* Compute checksum to prevent dead code elimination */
double compute_checksum() {
    double checksum = 0.0;
    for (int i = 0; i < MATRIX_SIZE; i += 64) {  // Strided access
        for (int j = 0; j < MATRIX_SIZE; j += 64) {
            checksum += matrix_c[i][j];
        }
    }
    return checksum;
}

/* Transpose operation - cache pattern changes */
void transpose_matrix() {
    double temp[MATRIX_SIZE][MATRIX_SIZE];
    for (int i = 0; i < MATRIX_SIZE; i++) {
        __builtin_prefetch(&matrix_a[i+4][0], 0, 1);
        for (int j = 0; j < MATRIX_SIZE; j++) {
            temp[j][i] = matrix_a[i][j];
        }
    }
    /* Copy back */
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = temp[i][j];
        }
    }
}

int main() {
    clock_t start, end;
    double total_time = 0.0;
    double final_checksum = 0.0;
    
    printf("Starting cache-sensitive matrix operations...\n");
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        init_matrices();
        
        start = clock();
        
        /* Mix different access patterns to encourage various optimizations */
        if (iter % 3 == 0) {
            matrix_multiply_row_major();
        } else if (iter % 3 == 1) {
            matrix_multiply_col_major();
        } else {
            matrix_multiply_blocked();
        }
        
        /* Occasionally transpose to change access patterns */
        if (iter % 2 == 0) {
            transpose_matrix();
        }
        
        end = clock();
        double iter_time = ((double)(end - start)) / CLOCKS_PER_SEC;
        total_time += iter_time;
        
        double iter_checksum = compute_checksum();
        final_checksum += iter_checksum;
        
        printf("Iteration %d: time = %.3f sec, checksum = %.6f\n", 
               iter, iter_time, iter_checksum);
    }
    
    printf("\nTotal time: %.3f seconds\n", total_time);
    printf("Final accumulated checksum: %.6f\n", final_checksum);
    
    /* Use checksum in conditional to prevent optimization */
    if (final_checksum > 0.0) {
        printf("Computation completed successfully.\n");
    } else {
        printf("Warning: Zero checksum detected.\n");
    }
    
    return 0;
}

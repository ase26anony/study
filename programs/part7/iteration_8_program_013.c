/*
 * Cache-sensitive test program to trigger GCC driver's cache detection logic
 * This program uses patterns that encourage the compiler to consider cache parameters
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MATRIX_SIZE 1024
#define ITERATIONS 5

/* Large multi-dimensional arrays to stress cache considerations */
static double matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_c[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_d[MATRIX_SIZE][MATRIX_SIZE];

/* Function with row-major access pattern */
void row_major_traversal(int size) {
    volatile double sum = 0.0;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            sum += matrix_a[i][j];  /* Row-major access */
            /* Use __builtin_prefetch to hint at cache-aware patterns */
            if (j + 4 < size) {
                __builtin_prefetch(&matrix_a[i][j + 4], 0, 3);
            }
        }
    }
    /* Prevent dead code elimination */
    *(volatile double*)&matrix_a[0][0] = sum;
}

/* Function with column-major access pattern */
void column_major_traversal(int size) {
    volatile double sum = 0.0;
    for (int j = 0; j < size; j++) {
        for (int i = 0; i < size; i++) {
            sum += matrix_b[i][j];  /* Column-major access */
            if (i + 4 < size) {
                __builtin_prefetch(&matrix_b[i + 4][j], 0, 3);
            }
        }
    }
    *(volatile double*)&matrix_b[0][0] = sum;
}

/* Matrix multiplication - cache blocking sensitive */
void matrix_multiply(int size) {
    const int block_size = 32;  /* Typical cache block size */
    
    for (int i0 = 0; i0 < size; i0 += block_size) {
        for (int j0 = 0; j0 < size; j0 += block_size) {
            for (int k0 = 0; k0 < size; k0 += block_size) {
                /* Process block */
                int i_end = (i0 + block_size) < size ? (i0 + block_size) : size;
                int j_end = (j0 + block_size) < size ? (j0 + block_size) : size;
                int k_end = (k0 + block_size) < size ? (k0 + block_size) : size;
                
                for (int i = i0; i < i_end; i++) {
                    for (int k = k0; k < k_end; k++) {
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

/* Transposition - cache sensitive operation */
void matrix_transpose(int size) {
    for (int i = 0; i < size; i++) {
        for (int j = i + 1; j < size; j++) {
            double temp = matrix_d[i][j];
            matrix_d[i][j] = matrix_d[j][i];
            matrix_d[j][i] = temp;
        }
    }
}

/* Initialize matrices with deterministic values */
void init_matrices(int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            matrix_a[i][j] = (i + j) * 0.001;
            matrix_b[i][j] = (i - j) * 0.001;
            matrix_c[i][j] = 0.0;
            matrix_d[i][j] = (i * j) * 0.001;
        }
    }
}

/* Compute checksum to prevent optimization */
double compute_checksum(int size) {
    double checksum = 0.0;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            checksum += matrix_a[i][j] + matrix_b[i][j] + 
                       matrix_c[i][j] + matrix_d[i][j];
        }
    }
    return checksum;
}

int main() {
    int size = MATRIX_SIZE;
    
    printf("Initializing matrices...\n");
    init_matrices(size);
    
    printf("Performing cache-sensitive operations...\n");
    
    clock_t start = clock();
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        row_major_traversal(size);
        column_major_traversal(size);
        matrix_multiply(size);
        matrix_transpose(size);
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    double checksum = compute_checksum(size);
    
    printf("Operations completed in %.2f seconds\n", elapsed);
    printf("Checksum: %.6f\n", checksum);
    printf("Matrix size: %dx%d, Iterations: %d\n", 
           size, size, ITERATIONS);
    
    /* Use checksum to affect program output */
    if (checksum > 0.0) {
        printf("Result validation: PASS\n");
        return 0;
    } else {
        printf("Result validation: FAIL\n");
        return 1;
    }
}

/*
 * Cache-sensitive test program to trigger GCC driver's cache detection logic
 * This program performs operations that benefit from cache-aware optimizations
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MATRIX_SIZE 1024
#define ITERATIONS 5

/* Large multi-dimensional arrays to stress cache */
static double matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_c[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_temp[MATRIX_SIZE][MATRIX_SIZE];

/* Different access patterns to trigger various optimizations */
void row_major_access(double mat[MATRIX_SIZE][MATRIX_SIZE]) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            mat[i][j] = i * MATRIX_SIZE + j;
        }
    }
}

void column_major_access(double mat[MATRIX_SIZE][MATRIX_SIZE]) {
    for (int j = 0; j < MATRIX_SIZE; j++) {
        for (int i = 0; i < MATRIX_SIZE; i++) {
            mat[i][j] = i * MATRIX_SIZE + j;
        }
    }
}

/* Matrix multiplication with potential for cache blocking */
void matrix_multiply(double a[MATRIX_SIZE][MATRIX_SIZE],
                     double b[MATRIX_SIZE][MATRIX_SIZE],
                     double c[MATRIX_SIZE][MATRIX_SIZE]) {
    int i, j, k;
    
    /* Clear result matrix */
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            c[i][j] = 0.0;
        }
    }
    
    /* Perform multiplication with cache-aware blocking hint */
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (k = 0; k < MATRIX_SIZE; k++) {
            double aik = a[i][k];
            /* Use __builtin_prefetch to hint at cache behavior */
            if (k + 4 < MATRIX_SIZE) {
                __builtin_prefetch(&a[i][k+4], 0, 3);
                __builtin_prefetch(&b[k+4][0], 0, 3);
            }
            for (j = 0; j < MATRIX_SIZE; j++) {
                c[i][j] += aik * b[k][j];
            }
        }
    }
}

/* Transpose operation - poor cache behavior without optimization */
void matrix_transpose(double src[MATRIX_SIZE][MATRIX_SIZE],
                      double dst[MATRIX_SIZE][MATRIX_SIZE]) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            dst[j][i] = src[i][j];
        }
    }
}

/* Compute checksum to prevent dead code elimination */
double compute_checksum(double mat[MATRIX_SIZE][MATRIX_SIZE]) {
    double sum = 0.0;
    for (int i = 0; i < MATRIX_SIZE; i += 16) {  /* Strided access */
        for (int j = 0; j < MATRIX_SIZE; j += 16) {
            sum += mat[i][j];
            /* Prefetch ahead */
            if (j + 32 < MATRIX_SIZE) {
                __builtin_prefetch(&mat[i][j+32], 0, 1);
            }
        }
    }
    return sum;
}

int main() {
    clock_t start, end;
    double cpu_time_used;
    double total_checksum = 0.0;
    
    printf("Cache detection test program\n");
    printf("Matrix size: %d x %d\n", MATRIX_SIZE, MATRIX_SIZE);
    printf("Element size: %lu bytes\n", sizeof(double));
    printf("Total data per matrix: %.2f MB\n", 
           (double)(MATRIX_SIZE * MATRIX_SIZE * sizeof(double)) / (1024 * 1024));
    
    /* Initialize matrices with different access patterns */
    start = clock();
    
    row_major_access(matrix_a);
    column_major_access(matrix_b);
    
    /* Perform multiple operations to give compiler optimization opportunities */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        if (iter % 2 == 0) {
            matrix_multiply(matrix_a, matrix_b, matrix_c);
        } else {
            matrix_transpose(matrix_c, matrix_temp);
            /* Swap matrices */
            double (*temp_ptr)[MATRIX_SIZE] = matrix_c;
            matrix_c = matrix_temp;
            matrix_temp = temp_ptr;
        }
        
        /* Compute and accumulate checksum */
        total_checksum += compute_checksum(matrix_c);
        
        /* Modify matrices slightly for next iteration */
        for (int i = 0; i < MATRIX_SIZE; i += 64) {
            matrix_a[i][i] += 0.001 * iter;
            matrix_b[i][i] += 0.001 * iter;
        }
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Total computation time: %.2f seconds\n", cpu_time_used);
    printf("Final checksum: %e\n", total_checksum);
    printf("Checksum validation: %s\n", 
           (total_checksum > 0 && total_checksum < 1e12) ? "PASS" : "FAIL");
    
    return 0;
}

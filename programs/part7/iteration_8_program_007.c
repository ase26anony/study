/* Cache-sensitive test program to trigger GCC driver cache detection */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MATRIX_SIZE 1024
#define ITERATIONS 5

/* Large multi-dimensional arrays to encourage cache-aware optimizations */
static double matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_c[MATRIX_SIZE][MATRIX_SIZE];

/* Function with row-major access pattern */
void row_major_multiply() {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int k = 0; k < MATRIX_SIZE; k++) {
            /* Use __builtin_prefetch to hint at cache-aware access */
            if (k + 4 < MATRIX_SIZE) {
                __builtin_prefetch(&matrix_a[i][k + 4], 0, 3);
                __builtin_prefetch(&matrix_b[k + 4][0], 0, 3);
            }
            double a_ik = matrix_a[i][k];
            for (int j = 0; j < MATRIX_SIZE; j++) {
                matrix_c[i][j] += a_ik * matrix_b[k][j];
            }
        }
    }
}

/* Function with column-major access pattern */
void column_major_transpose() {
    for (int j = 0; j < MATRIX_SIZE; j++) {
        for (int i = 0; i < MATRIX_SIZE; i++) {
            /* Prefetch hints for column-major access */
            if (i + 8 < MATRIX_SIZE) {
                __builtin_prefetch(&matrix_c[i + 8][j], 1, 2);
            }
            matrix_c[i][j] = matrix_b[j][i];
        }
    }
}

/* Cache-blocked matrix multiplication */
void blocked_matrix_multiply(int block_size) {
    for (int ii = 0; ii < MATRIX_SIZE; ii += block_size) {
        for (int jj = 0; jj < MATRIX_SIZE; jj += block_size) {
            for (int kk = 0; kk < MATRIX_SIZE; kk += block_size) {
                /* Process block */
                for (int i = ii; i < ii + block_size && i < MATRIX_SIZE; i++) {
                    for (int k = kk; k < kk + block_size && k < MATRIX_SIZE; k++) {
                        double a_ik = matrix_a[i][k];
                        for (int j = jj; j < jj + block_size && j < MATRIX_SIZE; j++) {
                            matrix_c[i][j] += a_ik * matrix_b[k][j];
                        }
                    }
                }
            }
        }
    }
}

/* Initialize matrices with deterministic values */
void initialize_matrices() {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = (i + j) % 256;
            matrix_b[i][j] = (i * j) % 256;
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

int main() {
    clock_t start, end;
    double cpu_time_used;
    
    printf("Starting cache-sensitive computation...\n");
    
    initialize_matrices();
    
    /* Time different access patterns */
    start = clock();
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Mix different access patterns */
        row_major_multiply();
        column_major_transpose();
        
        /* Try different block sizes to trigger different cache optimizations */
        blocked_matrix_multiply(32);   /* Small block - L1 cache */
        blocked_matrix_multiply(64);   /* Medium block - L1/L2 cache */
        blocked_matrix_multiply(128);  /* Larger block - L2/L3 cache */
        
        /* Re-initialize to prevent overflow */
        if (iter < ITERATIONS - 1) {
            for (int i = 0; i < MATRIX_SIZE; i++) {
                for (int j = 0; j < MATRIX_SIZE; j++) {
                    matrix_c[i][j] = 0.0;
                }
            }
        }
    }
    
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    double checksum = compute_checksum();
    
    printf("Computation completed in %.2f seconds\n", cpu_time_used);
    printf("Checksum: %.6f\n", checksum);
    printf("Matrix size: %dx%d, Iterations: %d\n", 
           MATRIX_SIZE, MATRIX_SIZE, ITERATIONS);
    
    /* Use checksum in conditional to prevent optimization */
    if (checksum > 0.0) {
        printf("Result validated - checksum is positive\n");
    } else {
        printf("Result validated - checksum is non-positive\n");
    }
    
    return 0;
}

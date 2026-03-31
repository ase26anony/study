/* test_cache_detect.c - Cache-sensitive program to trigger GCC driver cache detection */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MATRIX_SIZE 1024
#define ITERATIONS 5
#define BLOCK_SIZE 64

/* Cache-sensitive matrix multiplication */
void matrix_multiply(double A[MATRIX_SIZE][MATRIX_SIZE],
                     double B[MATRIX_SIZE][MATRIX_SIZE],
                     double C[MATRIX_SIZE][MATRIX_SIZE]) {
    int i, j, k;
    
    /* Blocked matrix multiplication for better cache utilization */
    for (i = 0; i < MATRIX_SIZE; i += BLOCK_SIZE) {
        for (j = 0; j < MATRIX_SIZE; j += BLOCK_SIZE) {
            for (k = 0; k < MATRIX_SIZE; k += BLOCK_SIZE) {
                /* Process block */
                for (int ii = i; ii < i + BLOCK_SIZE && ii < MATRIX_SIZE; ii++) {
                    for (int jj = j; jj < j + BLOCK_SIZE && jj < MATRIX_SIZE; jj++) {
                        double sum = C[ii][jj];
                        for (int kk = k; kk < k + BLOCK_SIZE && kk < MATRIX_SIZE; kk++) {
                            sum += A[ii][kk] * B[kk][jj];
                        }
                        C[ii][jj] = sum;
                    }
                }
            }
        }
    }
}

/* Row-major vs column-major access patterns */
double row_major_sum(double matrix[MATRIX_SIZE][MATRIX_SIZE]) {
    double sum = 0.0;
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            sum += matrix[i][j];
            /* Use prefetch hint for next cache line */
            if (j % 16 == 0 && j + 32 < MATRIX_SIZE) {
                __builtin_prefetch(&matrix[i][j + 32], 0, 3);
            }
        }
    }
    return sum;
}

double column_major_sum(double matrix[MATRIX_SIZE][MATRIX_SIZE]) {
    double sum = 0.0;
    for (int j = 0; j < MATRIX_SIZE; j++) {
        for (int i = 0; i < MATRIX_SIZE; i++) {
            sum += matrix[i][j];
            /* Prefetch hint with different locality */
            if (i % 16 == 0 && i + 32 < MATRIX_SIZE) {
                __builtin_prefetch(&matrix[i + 32][j], 0, 1);
            }
        }
    }
    return sum;
}

/* Cache line size sensitive operation */
void cache_line_test(double *array, int size) {
    volatile double sum = 0.0;
    int stride = 8; /* Likely cache line size / sizeof(double) */
    
    for (int iter = 0; iter < 100; iter++) {
        for (int i = 0; i < size; i += stride) {
            sum += array[i];
        }
    }
    
    /* Prevent dead code elimination */
    if (sum == 0.0) {
        printf("Impossible\n");
    }
}

/* Initialize matrices with deterministic values */
void init_matrices(double A[MATRIX_SIZE][MATRIX_SIZE],
                   double B[MATRIX_SIZE][MATRIX_SIZE],
                   double C[MATRIX_SIZE][MATRIX_SIZE]) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            A[i][j] = (i + j) * 0.001;
            B[i][j] = (i - j) * 0.001;
            C[i][j] = 0.0;
        }
    }
}

int main() {
    /* Large stack arrays to encourage cache-sensitive optimizations */
    double A[MATRIX_SIZE][MATRIX_SIZE];
    double B[MATRIX_SIZE][MATRIX_SIZE];
    double C[MATRIX_SIZE][MATRIX_SIZE];
    
    /* Heap allocation for cache line testing */
    double *heap_array = (double*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(double));
    if (!heap_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with deterministic pattern */
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        heap_array[i] = i * 0.0001;
    }
    
    printf("Starting cache-sensitive computations...\n");
    
    /* Initialize matrices */
    init_matrices(A, B, C);
    
    double total_sum = 0.0;
    clock_t start = clock();
    
    /* Perform multiple iterations with different access patterns */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Matrix multiplication (cache-blocked) */
        matrix_multiply(A, B, C);
        
        /* Different access patterns */
        total_sum += row_major_sum(C);
        total_sum += column_major_sum(C);
        
        /* Cache line sensitive operation */
        cache_line_test(heap_array, MATRIX_SIZE * MATRIX_SIZE);
        
        /* Modify data slightly for next iteration */
        for (int i = 0; i < MATRIX_SIZE; i++) {
            A[i][i] += 0.001;
        }
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    /* Compute checksum to prevent optimization */
    double checksum = total_sum + elapsed * 1000.0;
    
    printf("Computation completed in %.2f seconds\n", elapsed);
    printf("Checksum: %.6f\n", checksum);
    printf("Matrix size: %dx%d, Block size: %d\n", 
           MATRIX_SIZE, MATRIX_SIZE, BLOCK_SIZE);
    
    free(heap_array);
    
    /* Return deterministic result for validation */
    return (checksum > 0.0) ? 0 : 1;
}

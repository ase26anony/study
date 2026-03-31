/* test_cache_detect.c - Program designed to trigger GCC driver cache detection */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MATRIX_SIZE 1024
#define ITERATIONS 5

/* Cache-sensitive matrix operations */
void matrix_multiply(double A[MATRIX_SIZE][MATRIX_SIZE],
                     double B[MATRIX_SIZE][MATRIX_SIZE],
                     double C[MATRIX_SIZE][MATRIX_SIZE]) {
    int i, j, k;
    
    /* Row-major traversal - cache-friendly */
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (k = 0; k < MATRIX_SIZE; k++) {
            double a = A[i][k];
            /* Prefetch hint for next cache line */
            if (k % 16 == 0) {
                __builtin_prefetch(&B[0][k + 8], 0, 3);
                __builtin_prefetch(&C[i][0], 1, 3);
            }
            for (j = 0; j < MATRIX_SIZE; j++) {
                C[i][j] += a * B[k][j];
            }
        }
    }
}

/* Column-major traversal - cache-unfriendly */
double column_sum(double M[MATRIX_SIZE][MATRIX_SIZE]) {
    double sum = 0.0;
    int i, j;
    
    for (j = 0; j < MATRIX_SIZE; j++) {
        for (i = 0; i < MATRIX_SIZE; i++) {
            sum += M[i][j];
        }
    }
    return sum;
}

/* Blocked matrix multiplication - explicitly cache-aware */
void blocked_matrix_multiply(double A[MATRIX_SIZE][MATRIX_SIZE],
                             double B[MATRIX_SIZE][MATRIX_SIZE],
                             double C[MATRIX_SIZE][MATRIX_SIZE],
                             int block_size) {
    int i, j, k, ii, jj, kk;
    
    for (i = 0; i < MATRIX_SIZE; i += block_size) {
        for (j = 0; j < MATRIX_SIZE; j += block_size) {
            for (k = 0; k < MATRIX_SIZE; k += block_size) {
                /* Process block */
                for (ii = i; ii < i + block_size && ii < MATRIX_SIZE; ii++) {
                    for (kk = k; kk < k + block_size && kk < MATRIX_SIZE; kk++) {
                        double a = A[ii][kk];
                        /* Prefetch for next iteration */
                        if (kk % 8 == 0) {
                            __builtin_prefetch(&B[kk][j + 8], 0, 2);
                        }
                        for (jj = j; jj < j + block_size && jj < MATRIX_SIZE; jj++) {
                            C[ii][jj] += a * B[kk][jj];
                        }
                    }
                }
            }
        }
    }
}

/* Initialize matrices with deterministic values */
void init_matrices(double A[MATRIX_SIZE][MATRIX_SIZE],
                   double B[MATRIX_SIZE][MATRIX_SIZE]) {
    int i, j;
    
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            A[i][j] = (i + j) * 0.001;
            B[i][j] = (i - j) * 0.001;
        }
    }
}

int main() {
    static double A[MATRIX_SIZE][MATRIX_SIZE];
    static double B[MATRIX_SIZE][MATRIX_SIZE];
    static double C[MATRIX_SIZE][MATRIX_SIZE] = {{0.0}};
    static double D[MATRIX_SIZE][MATRIX_SIZE] = {{0.0}};
    
    clock_t start, end;
    double cpu_time_used;
    double checksum = 0.0;
    
    /* Initialize matrices */
    init_matrices(A, B);
    
    printf("Testing cache-sensitive operations...\n");
    
    /* Test 1: Standard matrix multiplication */
    start = clock();
    matrix_multiply(A, B, C);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Standard multiplication: %.2f seconds\n", cpu_time_used);
    
    /* Test 2: Blocked matrix multiplication with different block sizes */
    start = clock();
    blocked_matrix_multiply(A, B, D, 64);  /* Try block size 64 */
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Blocked multiplication (64): %.2f seconds\n", cpu_time_used);
    
    /* Test 3: Column-major access pattern */
    start = clock();
    checksum = column_sum(C);
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Column sum: %.2f seconds, checksum: %e\n", cpu_time_used, checksum);
    
    /* Additional cache-stressing operations */
    {
        /* Large 3D array access */
        int size3d = 128;
        double (*array3d)[size3d][size3d] = malloc(size3d * size3d * size3d * sizeof(double));
        
        if (array3d) {
            int x, y, z;
            double sum3d = 0.0;
            
            /* Access in different patterns */
            for (z = 0; z < size3d; z++) {
                for (y = 0; y < size3d; y++) {
                    for (x = 0; x < size3d; x++) {
                        array3d[z][y][x] = (x + y + z) * 0.001;
                        sum3d += array3d[z][y][x];
                    }
                }
            }
            
            printf("3D array sum: %e\n", sum3d);
            free(array3d);
        }
    }
    
    /* Compute final validation checksum */
    {
        double final_checksum = 0.0;
        int i, j;
        
        for (i = 0; i < MATRIX_SIZE; i += 32) {  /* Strided access */
            for (j = 0; j < MATRIX_SIZE; j += 32) {
                final_checksum += C[i][j] + D[i][j];
            }
        }
        
        printf("Final validation checksum: %e\n", final_checksum);
        printf("Test completed successfully.\n");
    }
    
    return 0;
}

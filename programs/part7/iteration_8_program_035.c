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
#define BLOCK_SIZE 64

/* Large multi-dimensional arrays to stress cache */
static double matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_c[MATRIX_SIZE][MATRIX_SIZE];
static double matrix_d[MATRIX_SIZE][MATRIX_SIZE];

/* Function with nested loops accessing arrays in different patterns */
void cache_sensitive_operations() {
    int i, j, k, ii, jj, kk;
    double sum;
    
    /* Pattern 1: Row-major access (cache-friendly) */
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            matrix_c[i][j] = 0.0;
            for (k = 0; k < MATRIX_SIZE; k++) {
                matrix_c[i][j] += matrix_a[i][k] * matrix_b[k][j];
            }
        }
    }
    
    /* Pattern 2: Column-major access (cache-unfriendly) */
    for (j = 0; j < MATRIX_SIZE; j++) {
        for (i = 0; i < MATRIX_SIZE; i++) {
            matrix_d[i][j] = 0.0;
            for (k = 0; k < MATRIX_SIZE; k++) {
                matrix_d[i][j] += matrix_a[k][i] * matrix_b[j][k];
            }
        }
    }
    
    /* Pattern 3: Cache-blocked matrix multiplication */
    for (ii = 0; ii < MATRIX_SIZE; ii += BLOCK_SIZE) {
        for (jj = 0; jj < MATRIX_SIZE; jj += BLOCK_SIZE) {
            for (kk = 0; kk < MATRIX_SIZE; kk += BLOCK_SIZE) {
                for (i = ii; i < ii + BLOCK_SIZE && i < MATRIX_SIZE; i++) {
                    for (j = jj; j < jj + BLOCK_SIZE && j < MATRIX_SIZE; j++) {
                        sum = matrix_c[i][j];
                        for (k = kk; k < kk + BLOCK_SIZE && k < MATRIX_SIZE; k++) {
                            sum += matrix_a[i][k] * matrix_b[k][j];
                        }
                        matrix_c[i][j] = sum;
                    }
                }
            }
        }
    }
    
    /* Use __builtin_prefetch for cache-aware access */
    for (i = 0; i < MATRIX_SIZE - 1; i++) {
        for (j = 0; j < MATRIX_SIZE - 1; j++) {
            /* Prefetch next row elements */
            __builtin_prefetch(&matrix_a[i+1][j], 0, 3);
            __builtin_prefetch(&matrix_b[j][i+1], 0, 3);
            
            /* Prefetch next column elements */
            __builtin_prefetch(&matrix_a[i][j+1], 0, 3);
            __builtin_prefetch(&matrix_b[j+1][i], 0, 3);
        }
    }
}

/* Initialize matrices with deterministic values */
void initialize_matrices() {
    int i, j;
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            matrix_a[i][j] = (i + j) * 0.001;
            matrix_b[i][j] = (i - j) * 0.001;
            matrix_c[i][j] = 0.0;
            matrix_d[i][j] = 0.0;
        }
    }
}

/* Compute checksum to prevent dead code elimination */
double compute_checksum() {
    double checksum = 0.0;
    int i, j;
    
    for (i = 0; i < MATRIX_SIZE; i += 16) {
        for (j = 0; j < MATRIX_SIZE; j += 16) {
            checksum += matrix_c[i][j] + matrix_d[i][j];
        }
    }
    
    return checksum;
}

/* Memory access patterns that stress different cache levels */
void memory_access_patterns() {
    int *buffer1 = (int*)malloc(1024 * 1024 * sizeof(int));  // 4MB - L3 cache
    int *buffer2 = (int*)malloc(256 * 1024 * sizeof(int));   // 1MB - L2 cache
    int *buffer3 = (int*)malloc(32 * 1024 * sizeof(int));    // 128KB - L1/L2
    
    if (!buffer1 || !buffer2 || !buffer3) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    /* Sequential access pattern */
    for (int i = 0; i < 1024 * 1024; i++) {
        buffer1[i] = i;
    }
    
    /* Strided access pattern */
    for (int i = 0; i < 256 * 1024; i += 8) {
        buffer2[i] = i * 2;
    }
    
    /* Random access pattern */
    for (int i = 0; i < 10000; i++) {
        int idx = (i * 97) % (32 * 1024);
        buffer3[idx] = i;
    }
    
    free(buffer1);
    free(buffer2);
    free(buffer3);
}

int main() {
    clock_t start, end;
    double cpu_time_used;
    double total_checksum = 0.0;
    int iter;
    
    printf("Starting cache-sensitive operations...\n");
    
    start = clock();
    
    for (iter = 0; iter < ITERATIONS; iter++) {
        initialize_matrices();
        cache_sensitive_operations();
        memory_access_patterns();
        
        double iter_checksum = compute_checksum();
        total_checksum += iter_checksum;
        
        printf("Iteration %d checksum: %.6f\n", iter + 1, iter_checksum);
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("\nTotal checksum: %.6f\n", total_checksum);
    printf("Total time: %.2f seconds\n", cpu_time_used);
    printf("Average time per iteration: %.2f seconds\n", cpu_time_used / ITERATIONS);
    
    /* Print deterministic result for validation */
    printf("Final validation value: %.12f\n", total_checksum / ITERATIONS);
    
    return 0;
}

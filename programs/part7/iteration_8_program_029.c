/* test_cache_detect.c
 * A cache-sensitive program that encourages the GCC driver to detect CPU cache parameters
 * during compilation with various -march flags.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MATRIX_SIZE 1024
#define BLOCK_SIZE 32

/* Cache-sensitive matrix multiplication */
void matrix_multiply(double A[MATRIX_SIZE][MATRIX_SIZE],
                     double B[MATRIX_SIZE][MATRIX_SIZE],
                     double C[MATRIX_SIZE][MATRIX_SIZE]) {
    int i, j, k, ii, jj, kk;
    
    /* Blocked matrix multiplication for better cache utilization */
    for (i = 0; i < MATRIX_SIZE; i += BLOCK_SIZE) {
        for (j = 0; j < MATRIX_SIZE; j += BLOCK_SIZE) {
            for (k = 0; k < MATRIX_SIZE; k += BLOCK_SIZE) {
                /* Process block */
                for (ii = i; ii < i + BLOCK_SIZE && ii < MATRIX_SIZE; ii++) {
                    for (jj = j; jj < j + BLOCK_SIZE && jj < MATRIX_SIZE; jj++) {
                        double sum = C[ii][jj];
                        for (kk = k; kk < k + BLOCK_SIZE && kk < MATRIX_SIZE; kk++) {
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
            if (j % 8 == 0) {
                __builtin_prefetch(&matrix[i][j + 8], 0, 3);
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
            /* Prefetch for column-major access */
            if (i % 8 == 0) {
                __builtin_prefetch(&matrix[i + 8][j], 0, 3);
            }
        }
    }
    return sum;
}

/* Cache line size sensitive operation */
void cache_line_test(char *buffer, size_t size) {
    volatile char *p = buffer;
    for (size_t i = 0; i < size; i += 64) { /* Assume 64-byte cache lines */
        p[i] = (char)(i & 0xFF);
        __builtin_prefetch(&p[i + 64], 1, 3); /* Prefetch for write */
    }
}

/* Main computation that uses all cache-sensitive patterns */
double compute_cache_sensitive_checksum() {
    static double A[MATRIX_SIZE][MATRIX_SIZE];
    static double B[MATRIX_SIZE][MATRIX_SIZE];
    static double C[MATRIX_SIZE][MATRIX_SIZE] = {{0}};
    
    /* Initialize matrices with deterministic values */
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            A[i][j] = (i + j) * 0.001;
            B[i][j] = (i - j) * 0.001;
        }
    }
    
    /* Perform cache-sensitive operations */
    matrix_multiply(A, B, C);
    
    double row_sum = row_major_sum(C);
    double col_sum = column_major_sum(C);
    
    /* Cache line test */
    char buffer[8192];
    cache_line_test(buffer, sizeof(buffer));
    
    /* Combine results for final checksum */
    double checksum = row_sum + col_sum;
    for (int i = 0; i < sizeof(buffer); i += 64) {
        checksum += buffer[i];
    }
    
    return checksum;
}

int main() {
    printf("Cache Detection Test Program\n");
    printf("============================\n");
    
    /* Perform computation that benefits from cache-aware optimizations */
    double checksum = compute_cache_sensitive_checksum();
    
    /* Print deterministic result to prevent dead code elimination */
    printf("Computed checksum: %.6f\n", checksum);
    printf("(This value should be consistent across runs)\n");
    
    /* Additional cache hinting */
    volatile int hint = 0;
    for (int i = 0; i < 1000; i++) {
        hint = i * 2;
        __builtin_prefetch(&hint, 0, 1); /* Low temporal locality hint */
    }
    
    return 0;
}

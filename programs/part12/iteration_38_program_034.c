/* test_vector_comparisons.c
 * Designed to trigger vectorization of comparison operations
 * to cover lines 12216-12233 in tree-vect-stmts.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test functions for each comparison operator */

void test_gt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

void test_ge(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] * 2;
        }
    }
}

void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            c[i] = a[i] << 1;
        } else {
            c[i] = b[i] << 1;
        }
    }
}

void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            c[i] = a[i] | b[i];
        } else {
            c[i] = a[i] & b[i];
        }
    }
}

/* Additional tests with floating point to ensure different data types */
void test_gt_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] + b[i];
        }
    }
}

void test_le_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            c[i] = a[i] - b[i];
        } else {
            c[i] = a[i] / (b[i] + 1.0f);
        }
    }
}

/* Helper function to initialize arrays with pattern */
void init_arrays(int *a, int *b) {
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i - 1;  // Creates varied comparison results
    }
}

void init_float_arrays(float *a, float *b) {
    for (int i = 0; i < N; i++) {
        a[i] = (float)i * 1.5f;
        b[i] = (float)(N - i) * 0.75f;
    }
}

/* Checksum function to ensure computations are not optimized away */
int compute_checksum(int *arr) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr[i];
    }
    return sum;
}

float compute_checksum_float(float *arr) {
    float sum = 0.0f;
    for (int i = 0; i < N; i++) {
        sum += arr[i];
    }
    return sum;
}

int main() {
    /* Aligned allocations for better vectorization */
    int *a_int = (int*)aligned_alloc(32, N * sizeof(int));
    int *b_int = (int*)aligned_alloc(32, N * sizeof(int));
    int *c_int = (int*)aligned_alloc(32, N * sizeof(int));
    int *d_int = (int*)aligned_alloc(32, N * sizeof(int));
    int *e_int = (int*)aligned_alloc(32, N * sizeof(int));
    int *f_int = (int*)aligned_alloc(32, N * sizeof(int));
    
    float *a_float = (float*)aligned_alloc(32, N * sizeof(float));
    float *b_float = (float*)aligned_alloc(32, N * sizeof(float));
    float *c_float = (float*)aligned_alloc(32, N * sizeof(float));
    float *d_float = (float*)aligned_alloc(32, N * sizeof(float));
    
    if (!a_int || !b_int || !c_int || !d_int || !e_int || !f_int ||
        !a_float || !b_float || !c_float || !d_float) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    init_arrays(a_int, b_int);
    init_float_arrays(a_float, b_float);
    
    /* Execute all test functions */
    test_gt(a_int, b_int, c_int);
    test_ge(a_int, b_int, d_int);
    test_lt(a_int, b_int, e_int);
    test_le(a_int, b_int, f_int);
    
    test_gt_float(a_float, b_float, c_float);
    test_le_float(a_float, b_float, d_float);
    
    /* Compute checksums to prevent dead code elimination */
    int checksum1 = compute_checksum(c_int);
    int checksum2 = compute_checksum(d_int);
    int checksum3 = compute_checksum(e_int);
    int checksum4 = compute_checksum(f_int);
    
    float checksum5 = compute_checksum_float(c_float);
    float checksum6 = compute_checksum_float(d_float);
    
    /* Print results to ensure execution */
    printf("Integer checksums: %d %d %d %d\n", checksum1, checksum2, checksum3, checksum4);
    printf("Float checksums: %f %f\n", checksum5, checksum6);
    
    /* Cleanup */
    free(a_int); free(b_int); free(c_int); free(d_int); free(e_int); free(f_int);
    free(a_float); free(b_float); free(c_float); free(d_float);
    
    return 0;
}

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
        if (a[i] > b[i]) {  /* GT_EXPR */
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

void test_ge(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {  /* GE_EXPR */
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] * 2;
        }
    }
}

void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {  /* LT_EXPR */
            c[i] = a[i] << 1;
        } else {
            c[i] = b[i] << 1;
        }
    }
}

void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {  /* LE_EXPR */
            c[i] = a[i] | b[i];
        } else {
            c[i] = a[i] & b[i];
        }
    }
}

/* Additional tests with floating point to ensure different data types */
void test_gt_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  /* GT_EXPR with floats */
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

void test_ge_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {  /* GE_EXPR with floats */
            c[i] = a[i] * 2.0f;
        } else {
            c[i] = b[i] * 2.0f;
        }
    }
}

/* Helper function to initialize arrays with pattern */
void init_arrays(int *a, int *b) {
    for (int i = 0; i < N; i++) {
        a[i] = i;           /* 0, 1, 2, 3, ... */
        b[i] = N - i - 1;   /* 1023, 1022, 1021, ... */
    }
}

void init_float_arrays(float *a, float *b) {
    for (int i = 0; i < N; i++) {
        a[i] = (float)i * 1.5f;
        b[i] = (float)(N - i) * 0.5f;
    }
}

/* Checksum to ensure computations are not optimized away */
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
    /* Aligned arrays to help vectorization */
    int a_int[N] ALIGNED;
    int b_int[N] ALIGNED;
    int c_int[N] ALIGNED;
    int d_int[N] ALIGNED;
    int e_int[N] ALIGNED;
    int f_int[N] ALIGNED;
    
    float a_float[N] ALIGNED;
    float b_float[N] ALIGNED;
    float c_float[N] ALIGNED;
    float d_float[N] ALIGNED;
    
    /* Initialize arrays */
    init_arrays(a_int, b_int);
    init_float_arrays(a_float, b_float);
    
    /* Clear output arrays */
    memset(c_int, 0, sizeof(c_int));
    memset(d_int, 0, sizeof(d_int));
    memset(e_int, 0, sizeof(e_int));
    memset(f_int, 0, sizeof(f_int));
    memset(c_float, 0, sizeof(c_float));
    memset(d_float, 0, sizeof(d_float));
    
    /* Execute all test functions to trigger vectorization */
    test_gt(a_int, b_int, c_int);
    test_ge(a_int, b_int, d_int);
    test_lt(a_int, b_int, e_int);
    test_le(a_int, b_int, f_int);
    
    test_gt_float(a_float, b_float, c_float);
    test_ge_float(a_float, b_float, d_float);
    
    /* Compute checksums to ensure computations happen */
    int checksum1 = compute_checksum(c_int);
    int checksum2 = compute_checksum(d_int);
    int checksum3 = compute_checksum(e_int);
    int checksum4 = compute_checksum(f_int);
    
    float checksum5 = compute_checksum_float(c_float);
    float checksum6 = compute_checksum_float(d_float);
    
    /* Print results to prevent dead code elimination */
    printf("Checksums (to ensure execution):\n");
    printf("GT int: %d\n", checksum1);
    printf("GE int: %d\n", checksum2);
    printf("LT int: %d\n", checksum3);
    printf("LE int: %d\n", checksum4);
    printf("GT float: %.2f\n", checksum5);
    printf("GE float: %.2f\n", checksum6);
    
    /* Additional test with mixed comparisons in same loop */
    int g_int[N] ALIGNED;
    for (int i = 0; i < N; i++) {
        /* Mix of comparisons in one loop */
        if (a_int[i] > b_int[i]) {
            g_int[i] = 1;
        } else if (a_int[i] >= b_int[i]) {
            g_int[i] = 2;
        } else if (a_int[i] < b_int[i]) {
            g_int[i] = 3;
        } else if (a_int[i] <= b_int[i]) {
            g_int[i] = 4;
        } else {
            g_int[i] = 0;
        }
    }
    
    printf("Mixed comparisons checksum: %d\n", compute_checksum(g_int));
    
    return 0;
}

/* test_vector_comparisons.c
 * Designed to trigger vectorizer transformation of comparison operations
 * to bitwise mask operations in GCC's tree-vect-stmts.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test functions for each comparison operator */

void test_gt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  /* GT_EXPR - should trigger BIT_NOT_EXPR, BIT_AND_EXPR */
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

void test_ge(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {  /* GE_EXPR - should trigger BIT_NOT_EXPR, BIT_IOR_EXPR */
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] * 3;
        }
    }
}

void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {  /* LT_EXPR - should trigger BIT_NOT_EXPR, BIT_AND_EXPR with swap */
            c[i] = a[i] << 1;
        } else {
            c[i] = b[i] >> 1;
        }
    }
}

void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {  /* LE_EXPR - should trigger BIT_NOT_EXPR, BIT_IOR_EXPR with swap */
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
            c[i] = b[i] * 3.0f;
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

/* Compute checksum to ensure loops execute */
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
    ALIGNED int a_int[N], b_int[N], c_int[N];
    ALIGNED float a_float[N], b_float[N], c_float[N];
    
    /* Initialize data */
    init_arrays(a_int, b_int);
    init_float_arrays(a_float, b_float);
    
    /* Test each comparison operator */
    test_gt(a_int, b_int, c_int);
    int sum_gt = compute_checksum(c_int);
    printf("GT test checksum: %d\n", sum_gt);
    
    test_ge(a_int, b_int, c_int);
    int sum_ge = compute_checksum(c_int);
    printf("GE test checksum: %d\n", sum_ge);
    
    test_lt(a_int, b_int, c_int);
    int sum_lt = compute_checksum(c_int);
    printf("LT test checksum: %d\n", sum_lt);
    
    test_le(a_int, b_int, c_int);
    int sum_le = compute_checksum(c_int);
    printf("LE test checksum: %d\n", sum_le);
    
    /* Test floating point comparisons */
    test_gt_float(a_float, b_float, c_float);
    float sum_gt_float = compute_checksum_float(c_float);
    printf("GT float test checksum: %f\n", sum_gt_float);
    
    test_ge_float(a_float, b_float, c_float);
    float sum_ge_float = compute_checksum_float(c_float);
    printf("GE float test checksum: %f\n", sum_ge_float);
    
    /* Verify results are non-zero to ensure execution */
    if (sum_gt != 0 && sum_ge != 0 && sum_lt != 0 && sum_le != 0) {
        printf("All tests executed successfully.\n");
        return 0;
    } else {
        printf("Error: Some tests may have been optimized away.\n");
        return 1;
    }
}

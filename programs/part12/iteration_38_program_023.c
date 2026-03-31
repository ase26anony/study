/* test_vector_comparisons.c
 * Designed to trigger vectorization of comparison operations
 * to cover lines 12216-12233 in tree-vect-stmts.cc
 */

#include <stdio.h>
#include <stdlib.h>

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
            c[i] = a[i] | b[i];
        } else {
            c[i] = a[i] & b[i];
        }
    }
}

void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {  /* LE_EXPR */
            c[i] = a[i] ^ b[i];
        } else {
            c[i] = a[i] + b[i] * 2;
        }
    }
}

/* Floating point versions to ensure different type handling */

void test_gt_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  /* GT_EXPR with floats */
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] / (b[i] + 1.0f);
        }
    }
}

void test_ge_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {  /* GE_EXPR with floats */
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

/* Additional tests with different patterns */

void test_lt_mixed(int *restrict a, int *restrict b, int *restrict c, int *restrict d) {
    for (int i = 0; i < N; i++) {
        /* Complex condition using LT_EXPR */
        if ((a[i] < b[i]) && (d[i] > 0)) {
            c[i] = a[i] * 3;
        } else {
            c[i] = b[i];
        }
    }
}

void test_le_ternary(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        /* Using ternary operator with LE_EXPR */
        c[i] = (a[i] <= b[i]) ? a[i] + b[i] : a[i] - b[i];
    }
}

/* Main test driver */

int main() {
    /* Aligned arrays to help vectorization */
    ALIGNED int a_int[N], b_int[N], c_int[N], d_int[N];
    ALIGNED float a_float[N], b_float[N], c_float[N];
    
    /* Initialize with patterned data to create varied comparison results */
    for (int i = 0; i < N; i++) {
        a_int[i] = i;
        b_int[i] = N - i;
        d_int[i] = (i % 2 == 0) ? 1 : -1;
        
        a_float[i] = (float)i * 1.5f;
        b_float[i] = (float)(N - i) * 0.8f;
    }
    
    /* Test all comparison operators */
    test_gt(a_int, b_int, c_int);
    test_ge(a_int, b_int, c_int);
    test_lt(a_int, b_int, c_int);
    test_le(a_int, b_int, c_int);
    
    test_gt_float(a_float, b_float, c_float);
    test_ge_float(a_float, b_float, c_float);
    
    test_lt_mixed(a_int, b_int, c_int, d_int);
    test_le_ternary(a_int, b_int, c_int);
    
    /* Compute checksums to ensure code executes */
    int int_sum = 0;
    float float_sum = 0.0f;
    
    for (int i = 0; i < N; i++) {
        int_sum += c_int[i];
        float_sum += c_float[i];
    }
    
    printf("Integer checksum: %d\n", int_sum);
    printf("Float checksum: %f\n", float_sum);
    
    /* Validate with simple test */
    int validation = 1;
    for (int i = 0; i < 10; i++) {
        if (c_int[i] == 0 && i > 0) {
            validation = 0;
        }
    }
    
    if (validation) {
        printf("All tests completed successfully.\n");
        return 0;
    } else {
        printf("Test validation failed.\n");
        return 1;
    }
}

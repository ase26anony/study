/* test_vectorize_comparisons.c
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
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i];
        }
    }
}

void test_ge(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {  /* GE_EXPR */
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {  /* LT_EXPR */
            c[i] = a[i] * 3;
        } else {
            c[i] = b[i] * 2;
        }
    }
}

void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {  /* LE_EXPR */
            c[i] = a[i] + 5;
        } else {
            c[i] = b[i] + 10;
        }
    }
}

/* Additional tests with floating point to ensure different data types */
void test_gt_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  /* GT_EXPR with floats */
            c[i] = a[i] * 2.0f;
        } else {
            c[i] = b[i];
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

/* Test with ternary operator (alternative pattern) */
void test_lt_ternary(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? a[i] * 4 : b[i] * 3;  /* LT_EXPR in ternary */
    }
}

void test_le_ternary(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? a[i] + b[i] : a[i] - b[i];  /* LE_EXPR in ternary */
    }
}

/* Initialize arrays with pattern to create varied comparison results */
void init_arrays(int *a, int *b) {
    for (int i = 0; i < N; i++) {
        a[i] = i;              /* 0, 1, 2, 3, ... */
        b[i] = N - i - 1;      /* 1023, 1022, 1021, ... */
    }
}

void init_float_arrays(float *a, float *b) {
    for (int i = 0; i < N; i++) {
        a[i] = (float)i * 1.5f;
        b[i] = (float)(N - i) * 0.75f;
    }
}

/* Compute checksum to verify correctness and prevent dead code elimination */
int checksum(int *arr) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr[i];
    }
    return sum;
}

float checksum_float(float *arr) {
    float sum = 0.0f;
    for (int i = 0; i < N; i++) {
        sum += arr[i];
    }
    return sum;
}

int main() {
    /* Aligned arrays for better vectorization */
    ALIGNED int a_int[N], b_int[N], c_int[N], d_int[N], e_int[N], f_int[N];
    ALIGNED float a_float[N], b_float[N], c_float[N], d_float[N];
    
    /* Initialize data */
    init_arrays(a_int, b_int);
    init_float_arrays(a_float, b_float);
    
    /* Execute all test functions */
    test_gt(a_int, b_int, c_int);
    test_ge(a_int, b_int, d_int);
    test_lt(a_int, b_int, e_int);
    test_le(a_int, b_int, f_int);
    
    test_gt_float(a_float, b_float, c_float);
    test_ge_float(a_float, b_float, d_float);
    
    test_lt_ternary(a_int, b_int, e_int);
    test_le_ternary(a_int, b_int, f_int);
    
    /* Compute and print checksums to ensure code executes */
    printf("Checksum GT: %d\n", checksum(c_int));
    printf("Checksum GE: %d\n", checksum(d_int));
    printf("Checksum LT: %d\n", checksum(e_int));
    printf("Checksum LE: %d\n", checksum(f_int));
    printf("Checksum GT float: %.2f\n", checksum_float(c_float));
    printf("Checksum GE float: %.2f\n", checksum_float(d_float));
    
    return 0;
}

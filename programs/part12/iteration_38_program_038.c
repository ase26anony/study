/* test_vectorize_comparisons.c
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
            c[i] = a[i] + b[i] * 3;
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

/* Helper function to initialize arrays with patterned data */
void init_arrays(int *a, int *b) {
    for (int i = 0; i < N; i++) {
        a[i] = i;           /* 0, 1, 2, 3, ... */
        b[i] = N - i - 1;   /* N-1, N-2, ... 0 */
    }
}

void init_float_arrays(float *a, float *b) {
    for (int i = 0; i < N; i++) {
        a[i] = (float)i * 1.5f;
        b[i] = (float)(N - i) * 0.5f;
    }
}

/* Compute checksum to verify correctness and prevent dead code elimination */
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
    /* Aligned arrays for better vectorization */
    ALIGNED int a_int[N], b_int[N], c_gt[N], c_ge[N], c_lt[N], c_le[N];
    ALIGNED float a_float[N], b_float[N], c_gt_float[N], c_ge_float[N];
    
    /* Initialize arrays */
    init_arrays(a_int, b_int);
    init_float_arrays(a_float, b_float);
    
    /* Test each comparison operator with integer types */
    test_gt(a_int, b_int, c_gt);
    test_ge(a_int, b_int, c_ge);
    test_lt(a_int, b_int, c_lt);
    test_le(a_int, b_int, c_le);
    
    /* Test with floating point types */
    test_gt_float(a_float, b_float, c_gt_float);
    test_ge_float(a_float, b_float, c_ge_float);
    
    /* Compute and print checksums to ensure code executes */
    printf("Integer checksums:\n");
    printf("  GT: %d\n", compute_checksum(c_gt));
    printf("  GE: %d\n", compute_checksum(c_ge));
    printf("  LT: %d\n", compute_checksum(c_lt));
    printf("  LE: %d\n", compute_checksum(c_le));
    
    printf("\nFloat checksums:\n");
    printf("  GT: %f\n", compute_checksum_float(c_gt_float));
    printf("  GE: %f\n", compute_checksum_float(c_ge_float));
    
    /* Additional test with mixed patterns to ensure all branches are taken */
    printf("\nTesting with alternating patterns...\n");
    
    /* Create alternating pattern to ensure both true and false branches execute */
    for (int i = 0; i < N; i++) {
        a_int[i] = (i % 2 == 0) ? i * 2 : i;
        b_int[i] = (i % 3 == 0) ? i + 10 : i * 3;
    }
    
    /* Run tests again with different data */
    test_gt(a_int, b_int, c_gt);
    test_ge(a_int, b_int, c_ge);
    test_lt(a_int, b_int, c_lt);
    test_le(a_int, b_int, c_le);
    
    printf("Alternating pattern checksums:\n");
    printf("  GT: %d\n", compute_checksum(c_gt));
    printf("  GE: %d\n", compute_checksum(c_ge));
    printf("  LT: %d\n", compute_checksum(c_lt));
    printf("  LE: %d\n", compute_checksum(c_le));
    
    return 0;
}

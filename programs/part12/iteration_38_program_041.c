/* test_vector_comparisons.c
 * Designed to trigger vectorization of comparison operations
 * and cover the switch cases for GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR
 * in tree-vect-stmts.cc lines 12216-12233
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test functions for each comparison operator */

void test_gt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  /* GT_EXPR - should trigger case GT_EXPR */
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

void test_ge(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {  /* GE_EXPR - should trigger case GE_EXPR */
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] * 2;
        }
    }
}

void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {  /* LT_EXPR - should trigger case LT_EXPR with swap */
            c[i] = a[i] << 1;
        } else {
            c[i] = b[i] << 1;
        }
    }
}

void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {  /* LE_EXPR - should trigger case LE_EXPR with swap */
            c[i] = a[i] | b[i];
        } else {
            c[i] = a[i] & b[i];
        }
    }
}

/* Additional test with floating point to ensure different data types */
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

/* Test with more complex conditional expressions */
void test_complex_cond(int *restrict a, int *restrict b, int *restrict c, int *restrict d) {
    for (int i = 0; i < N; i++) {
        /* Mixed comparisons to potentially trigger multiple cases */
        if ((a[i] > b[i]) && (c[i] < d[i])) {  /* GT_EXPR and LT_EXPR */
            a[i] = b[i] + c[i];
        }
        if ((a[i] >= b[i]) || (c[i] <= d[i])) {  /* GE_EXPR and LE_EXPR */
            b[i] = a[i] - d[i];
        }
    }
}

/* Initialize arrays with patterned data to create varied comparison results */
void init_arrays(int *a, int *b, float *fa, float *fb) {
    for (int i = 0; i < N; i++) {
        a[i] = i;               /* 0, 1, 2, 3, ... */
        b[i] = N - i - 1;       /* 1023, 1022, 1021, ... */
        fa[i] = (float)i * 1.5f;
        fb[i] = (float)(N - i) * 1.5f;
    }
}

/* Compute checksum to ensure loops execute and produce results */
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
    /* Use aligned arrays to help vectorization */
    ALIGNED int a[N], b[N], c1[N], c2[N], c3[N], c4[N];
    ALIGNED int d[N], e[N];
    ALIGNED float fa[N], fb[N], fc1[N], fc2[N];
    
    /* Initialize data */
    init_arrays(a, b, fa, fb);
    for (int i = 0; i < N; i++) {
        d[i] = i % 100;
        e[i] = (i + 50) % 100;
    }
    
    /* Execute all test functions */
    test_gt(a, b, c1);
    test_ge(a, b, c2);
    test_lt(a, b, c3);
    test_le(a, b, c4);
    test_gt_float(fa, fb, fc1);
    test_ge_float(fa, fb, fc2);
    test_complex_cond(a, b, d, e);
    
    /* Compute and print checksums to ensure code runs */
    printf("Checksums (to verify execution):\n");
    printf("test_gt: %d\n", compute_checksum(c1));
    printf("test_ge: %d\n", compute_checksum(c2));
    printf("test_lt: %d\n", compute_checksum(c3));
    printf("test_le: %d\n", compute_checksum(c4));
    printf("test_gt_float: %f\n", compute_checksum_float(fc1));
    printf("test_ge_float: %f\n", compute_checksum_float(fc2));
    
    return 0;
}

/* test_vector_comparisons.c
 * Designed to trigger vectorization of comparison operations
 * and cover the switch cases in tree-vect-stmts.cc lines 12216-12233
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test functions for each comparison operator */

void test_gt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  // GT_EXPR
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

void test_ge(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {  // GE_EXPR
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] * 2;
        }
    }
}

void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {  // LT_EXPR
            c[i] = a[i] << 1;
        } else {
            c[i] = b[i] << 1;
        }
    }
}

void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {  // LE_EXPR
            c[i] = a[i] | b[i];
        } else {
            c[i] = a[i] & b[i];
        }
    }
}

/* Additional tests with floating point to ensure different data types */
void test_gt_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  // GT_EXPR with floats
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

void test_ge_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {  // GE_EXPR with floats
            c[i] = a[i] * 2.0f;
        } else {
            c[i] = b[i] * 2.0f;
        }
    }
}

/* Test with mixed comparisons in same loop */
void test_mixed_comparisons(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        // This should trigger multiple comparison types
        if (a[i] > b[i]) {      // GT_EXPR
            c[i] = 1;
        } else if (a[i] < b[i]) { // LT_EXPR
            c[i] = -1;
        } else if (a[i] >= b[i]) { // GE_EXPR (when equal)
            c[i] = 0;
        } else if (a[i] <= b[i]) { // LE_EXPR (always true when equal)
            c[i] = 100;
        }
    }
}

/* Test with ternary operator (conditional expression) */
void test_ternary_gt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] > b[i]) ? a[i] : b[i];  // GT_EXPR in ternary
    }
}

void test_ternary_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? a[i] + b[i] : a[i] - b[i];  // LE_EXPR in ternary
    }
}

/* Initialize arrays with patterns that create varied comparison results */
void init_arrays(int *a, int *b, float *fa, float *fb) {
    for (int i = 0; i < N; i++) {
        a[i] = i;                    // 0, 1, 2, 3, ...
        b[i] = N - i - 1;            // 1023, 1022, 1021, ...
        fa[i] = (float)i * 0.5f;
        fb[i] = (float)(N - i) * 0.5f;
    }
}

/* Compute checksum to ensure loops execute and prevent dead code elimination */
int compute_checksum(int *c) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += c[i];
    }
    return sum;
}

float compute_checksum_float(float *c) {
    float sum = 0.0f;
    for (int i = 0; i < N; i++) {
        sum += c[i];
    }
    return sum;
}

int main() {
    /* Aligned arrays to help vectorization */
    ALIGNED int a[N], b[N], c1[N], c2[N], c3[N], c4[N], c5[N], c6[N], c7[N];
    ALIGNED float fa[N], fb[N], fc1[N], fc2[N];
    
    /* Initialize data */
    init_arrays(a, b, fa, fb);
    
    /* Execute all test functions */
    test_gt(a, b, c1);
    test_ge(a, b, c2);
    test_lt(a, b, c3);
    test_le(a, b, c4);
    test_mixed_comparisons(a, b, c5);
    test_ternary_gt(a, b, c6);
    test_ternary_le(a, b, c7);
    test_gt_float(fa, fb, fc1);
    test_ge_float(fa, fb, fc2);
    
    /* Compute checksums to ensure code executes */
    int checksums[7];
    checksums[0] = compute_checksum(c1);
    checksums[1] = compute_checksum(c2);
    checksums[2] = compute_checksum(c3);
    checksums[3] = compute_checksum(c4);
    checksums[4] = compute_checksum(c5);
    checksums[5] = compute_checksum(c6);
    checksums[6] = compute_checksum(c7);
    
    float fchecksums[2];
    fchecksums[0] = compute_checksum_float(fc1);
    fchecksums[1] = compute_checksum_float(fc2);
    
    /* Print results to prevent optimization */
    printf("Integer checksums: ");
    for (int i = 0; i < 7; i++) {
        printf("%d ", checksums[i]);
    }
    printf("\nFloat checksums: %.2f %.2f\n", fchecksums[0], fchecksums[1]);
    
    return 0;
}

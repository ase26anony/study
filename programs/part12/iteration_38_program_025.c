/* test_vector_comparisons.c
 * Designed to trigger vectorization of comparison operations
 * and cover the specific switch cases in tree-vect-stmts.cc
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test functions for each comparison operator */

void test_gt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            c[i] = a[i] - b[i];
        } else {
            c[i] = 0;
        }
    }
}

void test_ge(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            c[i] = a[i] + b[i];
        } else {
            c[i] = -1;
        }
    }
}

void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            c[i] = b[i] - a[i];
        } else {
            c[i] = 0;
        }
    }
}

void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i];
        }
    }
}

/* Additional tests with floating point to ensure different data types */
void test_gt_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            c[i] = a[i] - b[i];
        } else {
            c[i] = 0.0f;
        }
    }
}

void test_ge_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            c[i] = a[i] + b[i];
        } else {
            c[i] = -1.0f;
        }
    }
}

/* Test with ternary operator (alternative pattern) */
void test_lt_ternary(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? (b[i] - a[i]) : 0;
    }
}

void test_le_ternary(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? (a[i] * 2) : b[i];
    }
}

/* Complex condition combining comparisons */
void test_complex_condition(int *restrict a, int *restrict b, int *restrict c, int *restrict d) {
    for (int i = 0; i < N; i++) {
        if ((a[i] > b[i]) && (c[i] < d[i])) {
            a[i] = b[i] + c[i];
        }
    }
}

/* Initialize arrays with pattern to ensure varied comparison results */
void init_arrays(int *a, int *b) {
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i - 1;  // Creates mix of true/false comparisons
    }
}

void init_float_arrays(float *a, float *b) {
    for (int i = 0; i < N; i++) {
        a[i] = (float)i * 1.5f;
        b[i] = (float)(N - i) * 0.8f;
    }
}

/* Compute checksum to ensure loops execute and prevent dead code elimination */
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
    int *a = (int*)aligned_alloc(32, N * sizeof(int));
    int *b = (int*)aligned_alloc(32, N * sizeof(int));
    int *c1 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c2 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c3 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c4 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c5 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c6 = (int*)aligned_alloc(32, N * sizeof(int));
    int *d = (int*)aligned_alloc(32, N * sizeof(int));
    
    float *fa = (float*)aligned_alloc(32, N * sizeof(float));
    float *fb = (float*)aligned_alloc(32, N * sizeof(float));
    float *fc1 = (float*)aligned_alloc(32, N * sizeof(float));
    float *fc2 = (float*)aligned_alloc(32, N * sizeof(float));
    
    if (!a || !b || !c1 || !c2 || !c3 || !c4 || !c5 || !c6 || !d ||
        !fa || !fb || !fc1 || !fc2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(a, b);
    init_arrays(c5, d);  // For complex condition test
    init_float_arrays(fa, fb);
    
    /* Execute all test functions */
    test_gt(a, b, c1);
    test_ge(a, b, c2);
    test_lt(a, b, c3);
    test_le(a, b, c4);
    test_lt_ternary(a, b, c5);
    test_le_ternary(a, b, c6);
    test_complex_condition(a, b, c5, d);
    test_gt_float(fa, fb, fc1);
    test_ge_float(fa, fb, fc2);
    
    /* Compute checksums to ensure execution */
    int checksum = 0;
    checksum += compute_checksum(c1);
    checksum += compute_checksum(c2);
    checksum += compute_checksum(c3);
    checksum += compute_checksum(c4);
    checksum += compute_checksum(c5);
    checksum += compute_checksum(c6);
    checksum += compute_checksum(a);
    checksum += compute_checksum(d);
    
    float fchecksum = 0.0f;
    fchecksum += compute_checksum_float(fc1);
    fchecksum += compute_checksum_float(fc2);
    
    printf("Integer checksum: %d\n", checksum);
    printf("Float checksum: %f\n", fchecksum);
    
    /* Cleanup */
    free(a); free(b); free(c1); free(c2); free(c3); free(c4);
    free(c5); free(c6); free(d);
    free(fa); free(fb); free(fc1); free(fc2);
    
    return 0;
}

/* test_vectorized_comparisons.c
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
            c[i] = a[i] + b[i] * 2;
        }
    }
}

/* Additional tests with floating point for completeness */
void test_gt_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  /* GT_EXPR with floats */
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] + b[i];
        }
    }
}

void test_ge_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {  /* GE_EXPR with floats */
            c[i] = a[i] - b[i];
        } else {
            c[i] = b[i] - a[i];
        }
    }
}

/* Using ternary operator to create mask directly */
void test_lt_ternary(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? a[i] : b[i];  /* LT_EXPR in ternary */
    }
}

void test_le_ternary(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? a[i] + b[i] : a[i] - b[i];  /* LE_EXPR in ternary */
    }
}

/* Complex boolean expression to force mask combination */
void test_complex_condition(int *restrict a, int *restrict b, int *restrict c, 
                           int *restrict d, int *restrict e) {
    for (int i = 0; i < N; i++) {
        /* Combined conditions that may need multiple masks */
        if ((a[i] > b[i]) && (c[i] < d[i])) {  /* GT_EXPR and LT_EXPR */
            e[i] = a[i] + c[i];
        } else if (a[i] >= d[i]) {  /* GE_EXPR */
            e[i] = b[i] + d[i];
        } else {
            e[i] = 0;
        }
    }
}

/* Initialize arrays with patterned data to ensure varied comparison results */
void init_arrays(int *a, int *b, int *c, int *d, int *e) {
    for (int i = 0; i < N; i++) {
        a[i] = i;               /* 0, 1, 2, 3, ... */
        b[i] = N - i;           /* 1024, 1023, 1022, ... */
        c[i] = i % 64;          /* 0-63 repeating */
        d[i] = (i * 3) % 128;   /* 0-127 with stride 3 */
        e[i] = 0;
    }
}

void init_float_arrays(float *a, float *b, float *c) {
    for (int i = 0; i < N; i++) {
        a[i] = (float)i * 1.5f;
        b[i] = (float)(N - i) * 0.7f;
        c[i] = 0.0f;
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
    /* Use aligned allocations for better vectorization */
    int *a = (int*)aligned_alloc(32, N * sizeof(int));
    int *b = (int*)aligned_alloc(32, N * sizeof(int));
    int *c = (int*)aligned_alloc(32, N * sizeof(int));
    int *d = (int*)aligned_alloc(32, N * sizeof(int));
    int *e = (int*)aligned_alloc(32, N * sizeof(int));
    float *fa = (float*)aligned_alloc(32, N * sizeof(float));
    float *fb = (float*)aligned_alloc(32, N * sizeof(float));
    float *fc = (float*)aligned_alloc(32, N * sizeof(float));
    
    if (!a || !b || !c || !d || !e || !fa || !fb || !fc) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(a, b, c, d, e);
    init_float_arrays(fa, fb, fc);
    
    /* Execute all test functions to trigger different comparison cases */
    test_gt(a, b, c);
    int sum1 = compute_checksum(c);
    printf("GT test checksum: %d\n", sum1);
    
    test_ge(a, b, c);
    int sum2 = compute_checksum(c);
    printf("GE test checksum: %d\n", sum2);
    
    test_lt(a, b, c);
    int sum3 = compute_checksum(c);
    printf("LT test checksum: %d\n", sum3);
    
    test_le(a, b, c);
    int sum4 = compute_checksum(c);
    printf("LE test checksum: %d\n", sum4);
    
    test_gt_float(fa, fb, fc);
    float sum5 = compute_checksum_float(fc);
    printf("GT float test checksum: %f\n", sum5);
    
    test_ge_float(fa, fb, fc);
    float sum6 = compute_checksum_float(fc);
    printf("GE float test checksum: %f\n", sum6);
    
    test_lt_ternary(a, b, c);
    int sum7 = compute_checksum(c);
    printf("LT ternary test checksum: %d\n", sum7);
    
    test_le_ternary(a, b, c);
    int sum8 = compute_checksum(c);
    printf("LE ternary test checksum: %d\n", sum8);
    
    test_complex_condition(a, b, c, d, e);
    int sum9 = compute_checksum(e);
    printf("Complex condition test checksum: %d\n", sum9);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d); free(e);
    free(fa); free(fb); free(fc);
    
    return 0;
}

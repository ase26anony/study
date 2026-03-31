/* test_vectorize_comparisons.c
 * Designed to trigger vectorization of comparison operations
 * that map to specific bitwise transformations in GCC's tree-vect-stmts.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test functions for each comparison operator */

void test_gt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  /* GT_EXPR - should map to BIT_NOT_EXPR, BIT_AND_EXPR */
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

void test_ge(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {  /* GE_EXPR - should map to BIT_NOT_EXPR, BIT_IOR_EXPR */
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] * 2;
        }
    }
}

void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {  /* LT_EXPR - should map to BIT_NOT_EXPR, BIT_AND_EXPR with swap */
            c[i] = a[i] << 1;
        } else {
            c[i] = b[i] << 1;
        }
    }
}

void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {  /* LE_EXPR - should map to BIT_NOT_EXPR, BIT_IOR_EXPR with swap */
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
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] + b[i];
        }
    }
}

/* Test with more complex conditional expressions */
void test_mixed_comparisons(int *restrict a, int *restrict b, int *restrict c, int *restrict d) {
    for (int i = 0; i < N; i++) {
        /* Complex expression that should still vectorize */
        if ((a[i] > b[i]) && (c[i] <= d[i])) {  /* Mix of GT_EXPR and LE_EXPR */
            a[i] = b[i] + c[i];
        } else if (a[i] < b[i]) {  /* LT_EXPR */
            a[i] = b[i] - c[i];
        }
    }
}

/* Initialize arrays with pattern data to create varied comparison results */
void init_arrays(int *a, int *b, int *c, int *d) {
    for (int i = 0; i < N; i++) {
        a[i] = i;              /* 0, 1, 2, 3, ... */
        b[i] = N - i - 1;      /* 1023, 1022, 1021, ... */
        c[i] = i % 64;         /* 0-63 repeating */
        d[i] = (i + 32) % 128; /* 32-159 repeating */
    }
}

void init_float_arrays(float *a, float *b) {
    for (int i = 0; i < N; i++) {
        a[i] = (float)i * 0.5f;
        b[i] = (float)(N - i) * 0.3f;
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
    /* Use aligned allocation for better vectorization */
    int *a = (int*)aligned_alloc(32, N * sizeof(int));
    int *b = (int*)aligned_alloc(32, N * sizeof(int));
    int *c = (int*)aligned_alloc(32, N * sizeof(int));
    int *d = (int*)aligned_alloc(32, N * sizeof(int));
    int *result1 = (int*)aligned_alloc(32, N * sizeof(int));
    int *result2 = (int*)aligned_alloc(32, N * sizeof(int));
    int *result3 = (int*)aligned_alloc(32, N * sizeof(int));
    int *result4 = (int*)aligned_alloc(32, N * sizeof(int));
    
    float *fa = (float*)aligned_alloc(32, N * sizeof(float));
    float *fb = (float*)aligned_alloc(32, N * sizeof(float));
    float *fc = (float*)aligned_alloc(32, N * sizeof(float));
    
    if (!a || !b || !c || !d || !result1 || !result2 || !result3 || !result4 || !fa || !fb || !fc) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    init_arrays(a, b, c, d);
    init_float_arrays(fa, fb);
    
    /* Execute all test functions */
    test_gt(a, b, result1);
    test_ge(a, b, result2);
    test_lt(a, b, result3);
    test_le(a, b, result4);
    test_gt_float(fa, fb, fc);
    test_mixed_comparisons(a, b, c, d);
    
    /* Compute checksums to ensure code isn't optimized away */
    int checksum1 = compute_checksum(result1);
    int checksum2 = compute_checksum(result2);
    int checksum3 = compute_checksum(result3);
    int checksum4 = compute_checksum(result4);
    float checksum5 = compute_checksum_float(fc);
    int checksum6 = compute_checksum(a);
    
    printf("Checksums (for verification):\n");
    printf("GT test: %d\n", checksum1);
    printf("GE test: %d\n", checksum2);
    printf("LT test: %d\n", checksum3);
    printf("LE test: %d\n", checksum4);
    printf("Float GT test: %f\n", checksum5);
    printf("Mixed test: %d\n", checksum6);
    
    /* Clean up */
    free(a); free(b); free(c); free(d);
    free(result1); free(result2); free(result3); free(result4);
    free(fa); free(fb); free(fc);
    
    return 0;
}

/* test_vectorize_comparisons.c
 * Designed to trigger vectorization of comparison operations
 * that map to the uncovered switch cases in tree-vect-stmts.cc
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

/* Additional tests with floating point to ensure different data types */
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

/* Test with mixed comparisons in same loop */
void test_mixed_comparisons(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        /* This should trigger multiple comparison transformations */
        if (a[i] > b[i]) {    /* GT_EXPR */
            c[i] = 1;
        } else if (a[i] < b[i]) {  /* LT_EXPR */
            c[i] = -1;
        } else if (a[i] >= b[i]) { /* GE_EXPR */
            c[i] = 0;
        } else if (a[i] <= b[i]) { /* LE_EXPR */
            c[i] = 2;
        }
    }
}

/* Test with ternary operator (conditional expression) */
void test_ternary_gt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        /* Ternary with > comparison */
        c[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
}

void test_ternary_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        /* Ternary with <= comparison */
        c[i] = (a[i] <= b[i]) ? a[i] + b[i] : a[i] - b[i];
    }
}

/* Initialize arrays with patterned data to ensure varied comparisons */
void init_arrays(int *a, int *b, float *fa, float *fb) {
    for (int i = 0; i < N; i++) {
        a[i] = i;                    /* 0, 1, 2, 3, ... */
        b[i] = N - i;                /* 1024, 1023, 1022, ... */
        fa[i] = (float)i * 0.5f;
        fb[i] = (float)(N - i) * 0.3f;
    }
}

/* Compute checksum to verify correctness and prevent dead code elimination */
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
    /* Use aligned allocation for better vectorization */
    int *a = (int*)aligned_alloc(32, N * sizeof(int));
    int *b = (int*)aligned_alloc(32, N * sizeof(int));
    int *c1 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c2 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c3 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c4 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c5 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c6 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c7 = (int*)aligned_alloc(32, N * sizeof(int));
    
    float *fa = (float*)aligned_alloc(32, N * sizeof(float));
    float *fb = (float*)aligned_alloc(32, N * sizeof(float));
    float *fc1 = (float*)aligned_alloc(32, N * sizeof(float));
    float *fc2 = (float*)aligned_alloc(32, N * sizeof(float));
    
    if (!a || !b || !c1 || !c2 || !c3 || !c4 || !c5 || !c6 || !c7 || 
        !fa || !fb || !fc1 || !fc2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(a, b, fa, fb);
    
    /* Execute all test functions to trigger each comparison case */
    test_gt(a, b, c1);
    test_ge(a, b, c2);
    test_lt(a, b, c3);
    test_le(a, b, c4);
    test_mixed_comparisons(a, b, c5);
    test_ternary_gt(a, b, c6);
    test_ternary_le(a, b, c7);
    
    test_gt_float(fa, fb, fc1);
    test_ge_float(fa, fb, fc2);
    
    /* Compute and print checksums to ensure code executes */
    printf("Checksums (to verify execution):\n");
    printf("test_gt: %d\n", compute_checksum(c1));
    printf("test_ge: %d\n", compute_checksum(c2));
    printf("test_lt: %d\n", compute_checksum(c3));
    printf("test_le: %d\n", compute_checksum(c4));
    printf("test_mixed: %d\n", compute_checksum(c5));
    printf("test_ternary_gt: %d\n", compute_checksum(c6));
    printf("test_ternary_le: %d\n", compute_checksum(c7));
    printf("test_gt_float: %f\n", compute_checksum_float(fc1));
    printf("test_ge_float: %f\n", compute_checksum_float(fc2));
    
    /* Cleanup */
    free(a); free(b); free(c1); free(c2); free(c3); free(c4);
    free(c5); free(c6); free(c7); free(fa); free(fb); free(fc1); free(fc2);
    
    return 0;
}

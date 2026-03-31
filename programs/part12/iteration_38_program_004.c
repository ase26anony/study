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

/* Additional tests with floating point to ensure coverage */
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

/* Test with ternary operator (another pattern that creates masks) */
void test_lt_ternary(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? a[i] * 4 : b[i] * 3;  /* LT_EXPR in ternary */
    }
}

void test_le_ternary(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? a[i] + 100 : b[i] + 200;  /* LE_EXPR in ternary */
    }
}

/* Complex condition to force mask combination */
void test_complex_condition(int *restrict a, int *restrict b, int *restrict c, 
                           int *restrict d, int *restrict e) {
    for (int i = 0; i < N; i++) {
        /* Combined condition that may use multiple comparisons */
        if ((a[i] > b[i]) && (c[i] < d[i])) {  /* GT_EXPR and LT_EXPR */
            e[i] = a[i] + c[i];
        } else {
            e[i] = b[i] + d[i];
        }
    }
}

/* Initialize arrays with patterned data to create varied comparison results */
void init_arrays(int *a, int *b, float *fa, float *fb) {
    for (int i = 0; i < N; i++) {
        a[i] = i;                    /* 0, 1, 2, 3, ... */
        b[i] = N - i - 1;            /* 1023, 1022, 1021, ... */
        fa[i] = (float)i * 0.5f;
        fb[i] = (float)(N - i) * 0.3f;
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
    int *c1 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c2 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c3 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c4 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c5 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c6 = (int*)aligned_alloc(32, N * sizeof(int));
    int *d = (int*)aligned_alloc(32, N * sizeof(int));
    int *e = (int*)aligned_alloc(32, N * sizeof(int));
    
    float *fa = (float*)aligned_alloc(32, N * sizeof(float));
    float *fb = (float*)aligned_alloc(32, N * sizeof(float));
    float *fc1 = (float*)aligned_alloc(32, N * sizeof(float));
    float *fc2 = (float*)aligned_alloc(32, N * sizeof(float));
    
    if (!a || !b || !c1 || !c2 || !c3 || !c4 || !c5 || !c6 || !d || !e ||
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
    test_gt_float(fa, fb, fc1);
    test_ge_float(fa, fb, fc2);
    test_lt_ternary(a, b, c5);
    test_le_ternary(a, b, c6);
    
    /* Initialize d with different pattern for complex test */
    for (int i = 0; i < N; i++) {
        d[i] = i * 2;
    }
    test_complex_condition(a, b, c1, d, e);
    
    /* Compute checksums to ensure code executes and produces results */
    int checksum1 = compute_checksum(c1);
    int checksum2 = compute_checksum(c2);
    int checksum3 = compute_checksum(c3);
    int checksum4 = compute_checksum(c4);
    int checksum5 = compute_checksum(c5);
    int checksum6 = compute_checksum(c6);
    int checksum_e = compute_checksum(e);
    
    float fchecksum1 = compute_checksum_float(fc1);
    float fchecksum2 = compute_checksum_float(fc2);
    
    printf("Checksums (for verification):\n");
    printf("  test_gt: %d\n", checksum1);
    printf("  test_ge: %d\n", checksum2);
    printf("  test_lt: %d\n", checksum3);
    printf("  test_le: %d\n", checksum4);
    printf("  test_lt_ternary: %d\n", checksum5);
    printf("  test_le_ternary: %d\n", checksum6);
    printf("  test_complex_condition: %d\n", checksum_e);
    printf("  test_gt_float: %.2f\n", fchecksum1);
    printf("  test_ge_float: %.2f\n", fchecksum2);
    
    /* Free allocated memory */
    free(a); free(b); free(c1); free(c2); free(c3); free(c4);
    free(c5); free(c6); free(d); free(e);
    free(fa); free(fb); free(fc1); free(fc2);
    
    return 0;
}

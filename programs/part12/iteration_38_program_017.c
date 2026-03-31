/* test_vectorized_comparisons.c
 * Designed to trigger vectorization of comparison operations
 * and cover the switch cases for GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR
 * in tree-vect-stmts.cc lines 12216-12233
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test function for > operator (GT_EXPR) */
void test_gt(int *restrict a, int *restrict b, int *restrict c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {  /* GT_EXPR - should map to BIT_NOT_EXPR, BIT_AND_EXPR */
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i];
        }
    }
}

/* Test function for >= operator (GE_EXPR) */
void test_ge(int *restrict a, int *restrict b, int *restrict c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {  /* GE_EXPR - should map to BIT_NOT_EXPR, BIT_IOR_EXPR */
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

/* Test function for < operator (LT_EXPR) */
void test_lt(int *restrict a, int *restrict b, int *restrict c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {  /* LT_EXPR - should map to BIT_NOT_EXPR, BIT_AND_EXPR with swap */
            c[i] = a[i] * 3;
        } else {
            c[i] = b[i] * 2;
        }
    }
}

/* Test function for <= operator (LE_EXPR) */
void test_le(int *restrict a, int *restrict b, int *restrict c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {  /* LE_EXPR - should map to BIT_NOT_EXPR, BIT_IOR_EXPR with swap */
            c[i] = a[i] + 5;
        } else {
            c[i] = b[i] + 10;
        }
    }
}

/* Additional test with floating point to ensure different type handling */
void test_gt_float(float *restrict a, float *restrict b, float *restrict c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {  /* GT_EXPR with floats */
            c[i] = a[i] * 2.0f;
        } else {
            c[i] = b[i];
        }
    }
}

/* Test with mixed operations to ensure all paths are taken */
void test_mixed_comparisons(int *restrict a, int *restrict b, int *restrict c, int n) {
    for (int i = 0; i < n; i++) {
        /* Use ternary operator which also generates comparison */
        c[i] = (a[i] > b[i]) ? a[i] : b[i];      /* GT_EXPR */
        c[i] += (a[i] >= b[i]) ? i : -i;         /* GE_EXPR */
        c[i] *= (a[i] < b[i]) ? 2 : 1;           /* LT_EXPR */
        c[i] -= (a[i] <= b[i]) ? i/2 : i;        /* LE_EXPR */
    }
}

/* Initialize arrays with pattern that creates varied comparison results */
void init_arrays(int *a, int *b, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = i;              /* 0, 1, 2, 3, ... */
        b[i] = n - i - 1;      /* n-1, n-2, ... 0 */
    }
}

void init_float_arrays(float *a, float *b, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (float)i * 1.5f;
        b[i] = (float)(n - i) * 0.5f;
    }
}

/* Compute checksum to ensure loops execute and produce results */
int compute_checksum(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

float compute_float_checksum(float *arr, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int main() {
    /* Use aligned allocations for better vectorization */
    int *a = aligned_alloc(32, N * sizeof(int));
    int *b = aligned_alloc(32, N * sizeof(int));
    int *c1 = aligned_alloc(32, N * sizeof(int));
    int *c2 = aligned_alloc(32, N * sizeof(int));
    int *c3 = aligned_alloc(32, N * sizeof(int));
    int *c4 = aligned_alloc(32, N * sizeof(int));
    int *c5 = aligned_alloc(32, N * sizeof(int));
    
    float *fa = aligned_alloc(32, N * sizeof(float));
    float *fb = aligned_alloc(32, N * sizeof(float));
    float *fc = aligned_alloc(32, N * sizeof(float));
    
    if (!a || !b || !c1 || !c2 || !c3 || !c4 || !c5 || !fa || !fb || !fc) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    init_arrays(a, b, N);
    init_float_arrays(fa, fb, N);
    
    /* Execute all test functions */
    test_gt(a, b, c1, N);
    test_ge(a, b, c2, N);
    test_lt(a, b, c3, N);
    test_le(a, b, c4, N);
    test_mixed_comparisons(a, b, c5, N);
    test_gt_float(fa, fb, fc, N);
    
    /* Compute and print checksums to ensure code runs */
    printf("Checksum GT: %d\n", compute_checksum(c1, N));
    printf("Checksum GE: %d\n", compute_checksum(c2, N));
    printf("Checksum LT: %d\n", compute_checksum(c3, N));
    printf("Checksum LE: %d\n", compute_checksum(c4, N));
    printf("Checksum Mixed: %d\n", compute_checksum(c5, N));
    printf("Checksum Float GT: %f\n", compute_float_checksum(fc, N));
    
    /* Clean up */
    free(a); free(b); free(c1); free(c2); free(c3); free(c4); free(c5);
    free(fa); free(fb); free(fc);
    
    return 0;
}

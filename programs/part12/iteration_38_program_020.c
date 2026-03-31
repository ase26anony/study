/* test_vectorize_comparisons.c
 * Designed to trigger vectorization of comparison operations
 * to cover lines 12216-12233 in tree-vect-stmts.cc
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define SEED 42

/* Initialize arrays with patterned data to ensure varied comparison results */
void init_arrays(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        a[i] = i;               /* 0, 1, 2, 3, ... */
        b[i] = N - i - 1;       /* 1023, 1022, 1021, ... */
        c[i] = 0;               /* Output array */
    }
}

/* Test greater-than comparisons - triggers GT_EXPR case */
void test_gt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {      /* GT_EXPR */
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] + 1;
        }
    }
}

/* Test greater-or-equal comparisons - triggers GE_EXPR case */
void test_ge(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {     /* GE_EXPR */
            c[i] = a[i] * 3;
        } else {
            c[i] = b[i] + 2;
        }
    }
}

/* Test less-than comparisons - triggers LT_EXPR case (with swap) */
void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {      /* LT_EXPR */
            c[i] = a[i] * 4;
        } else {
            c[i] = b[i] + 3;
        }
    }
}

/* Test less-or-equal comparisons - triggers LE_EXPR case (with swap) */
void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {     /* LE_EXPR */
            c[i] = a[i] * 5;
        } else {
            c[i] = b[i] + 4;
        }
    }
}

/* Additional test with floating point to ensure different type handling */
void test_float_gt(float *restrict fa, float *restrict fb, float *restrict fc) {
    for (int i = 0; i < N; i++) {
        if (fa[i] > fb[i]) {    /* GT_EXPR with floats */
            fc[i] = fa[i] * 1.5f;
        } else {
            fc[i] = fb[i] * 0.5f;
        }
    }
}

/* Test with ternary operator (alternative mask generation pattern) */
void test_ternary_ge(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? a[i] * 6 : b[i] * 2;  /* GE_EXPR in ternary */
    }
}

/* Compute checksum to verify execution and prevent dead code elimination */
int compute_checksum(int *arr) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr[i];
    }
    return sum;
}

float compute_checksum_f(float *arr) {
    float sum = 0.0f;
    for (int i = 0; i < N; i++) {
        sum += arr[i];
    }
    return sum;
}

int main() {
    /* Allocate aligned memory for better vectorization */
    int *a = (int*)aligned_alloc(64, N * sizeof(int));
    int *b = (int*)aligned_alloc(64, N * sizeof(int));
    int *c = (int*)aligned_alloc(64, N * sizeof(int));
    float *fa = (float*)aligned_alloc(64, N * sizeof(float));
    float *fb = (float*)aligned_alloc(64, N * sizeof(float));
    float *fc = (float*)aligned_alloc(64, N * sizeof(float));
    
    if (!a || !b || !c || !fa || !fb || !fc) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize integer arrays */
    init_arrays(a, b, c);
    
    /* Test all four comparison operators with integer types */
    printf("Testing GT_EXPR (integer)...\n");
    test_gt(a, b, c);
    int sum_gt = compute_checksum(c);
    printf("GT checksum: %d\n", sum_gt);
    
    printf("Testing GE_EXPR (integer)...\n");
    test_ge(a, b, c);
    int sum_ge = compute_checksum(c);
    printf("GE checksum: %d\n", sum_ge);
    
    printf("Testing LT_EXPR (integer)...\n");
    test_lt(a, b, c);
    int sum_lt = compute_checksum(c);
    printf("LT checksum: %d\n", sum_lt);
    
    printf("Testing LE_EXPR (integer)...\n");
    test_le(a, b, c);
    int sum_le = compute_checksum(c);
    printf("LE checksum: %d\n", sum_le);
    
    /* Test ternary operator version */
    printf("Testing GE_EXPR (ternary)...\n");
    test_ternary_ge(a, b, c);
    int sum_ternary = compute_checksum(c);
    printf("Ternary GE checksum: %d\n", sum_ternary);
    
    /* Initialize and test floating point */
    for (int i = 0; i < N; i++) {
        fa[i] = (float)i * 1.5f;
        fb[i] = (float)(N - i) * 0.7f;
    }
    
    printf("Testing GT_EXPR (float)...\n");
    test_float_gt(fa, fb, fc);
    float sum_float = compute_checksum_f(fc);
    printf("Float GT checksum: %.2f\n", sum_float);
    
    /* Final validation */
    int total_int = sum_gt + sum_ge + sum_lt + sum_le + sum_ternary;
    printf("Total integer checksum: %d\n", total_int);
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(fa); free(fb); free(fc);
    
    return 0;
}

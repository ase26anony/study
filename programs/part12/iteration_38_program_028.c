/* test_vector_comparisons.c
 * Designed to trigger vectorization of comparison operations
 * and cover the switch cases for GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR
 * in tree-vect-stmts.cc lines 12216-12233
 */

#include <stdio.h>
#include <stdlib.h>

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
void test_lt(float *restrict a, float *restrict b, float *restrict c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {  /* LT_EXPR - should map to BIT_NOT_EXPR, BIT_AND_EXPR with swap */
            c[i] = a[i] * 3.0f;
        } else {
            c[i] = b[i];
        }
    }
}

/* Test function for <= operator (LE_EXPR) */
void test_le(float *restrict a, float *restrict b, float *restrict c, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {  /* LE_EXPR - should map to BIT_NOT_EXPR, BIT_IOR_EXPR with swap */
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

/* Additional test with mixed operators in same loop */
void test_mixed_comparisons(int *restrict a, int *restrict b, int *restrict c, int n) {
    for (int i = 0; i < n; i++) {
        /* This should trigger multiple comparison transformations */
        if (a[i] > b[i]) {   /* GT_EXPR */
            c[i] = 1;
        } else if (a[i] < b[i]) {  /* LT_EXPR */
            c[i] = -1;
        } else if (a[i] >= b[i]) {  /* GE_EXPR */
            c[i] = 0;
        } else if (a[i] <= b[i]) {  /* LE_EXPR */
            c[i] = 2;
        }
    }
}

/* Test with ternary operator (?:) which also uses comparisons */
void test_ternary(int *restrict a, int *restrict b, int *restrict c, int n) {
    for (int i = 0; i < n; i++) {
        /* Ternary with > comparison */
        c[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
}

/* Test with boolean combination of comparisons */
void test_combined(int *restrict a, int *restrict b, int *restrict c, int n) {
    for (int i = 0; i < n; i++) {
        /* Combined comparison using && - both comparisons should be vectorized */
        if (a[i] > 0 && a[i] < b[i]) {  /* GT_EXPR and LT_EXPR */
            c[i] = a[i] * b[i];
        } else {
            c[i] = 0;
        }
    }
}

int main(void) {
    /* Aligned arrays to help vectorization */
    ALIGNED int a_int[N], b_int[N], c_int[N];
    ALIGNED float a_float[N], b_float[N], c_float[N];
    
    /* Initialize with patterned data to create varied comparison results */
    for (int i = 0; i < N; i++) {
        a_int[i] = i;
        b_int[i] = N - i - 1;  /* Creates mix of true/false comparisons */
        a_float[i] = (float)i * 1.5f;
        b_float[i] = (float)(N - i) * 0.5f;
    }
    
    /* Execute all test functions to trigger different comparison operators */
    test_gt(a_int, b_int, c_int, N);
    test_ge(a_int, b_int, c_int, N);
    test_lt(a_float, b_float, c_float, N);
    test_le(a_float, b_float, c_float, N);
    test_mixed_comparisons(a_int, b_int, c_int, N);
    test_ternary(a_int, b_int, c_int, N);
    test_combined(a_int, b_int, c_int, N);
    
    /* Compute checksum to ensure code isn't optimized away */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += c_int[i];
        checksum += (int)c_float[i];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("All comparison tests completed.\n");
    
    return 0;
}

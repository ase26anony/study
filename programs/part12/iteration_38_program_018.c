/* test_vectorized_comparisons.c
 * Designed to trigger vectorization of comparison operations
 * to cover lines 12216-12233 in tree-vect-stmts.cc
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define SEED 42

/* Initialize arrays with pattern that creates varied comparison results */
void init_arrays(int *restrict a, int *restrict b) {
    for (int i = 0; i < N; i++) {
        a[i] = i;              /* 0, 1, 2, 3, ... */
        b[i] = N - i - 1;      /* 1023, 1022, 1021, ... */
    }
}

/* Initialize arrays for floating point comparisons */
void init_arrays_float(float *restrict a, float *restrict b) {
    for (int i = 0; i < N; i++) {
        a[i] = (float)i * 1.5f;
        b[i] = (float)(N - i) * 0.5f;
    }
}

/* Test GT_EXPR (>) - should trigger case GT_EXPR in tree-vect-stmts.cc */
int test_gt(int *restrict a, int *restrict b, int *restrict c) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  /* GT_EXPR */
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i];
        }
        sum += c[i];
    }
    return sum;
}

/* Test GE_EXPR (>=) - should trigger case GE_EXPR in tree-vect-stmts.cc */
int test_ge(int *restrict a, int *restrict b, int *restrict c) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {  /* GE_EXPR */
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
        sum += c[i];
    }
    return sum;
}

/* Test LT_EXPR (<) - should trigger case LT_EXPR in tree-vect-stmts.cc */
int test_lt(int *restrict a, int *restrict b, int *restrict c) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {  /* LT_EXPR */
            c[i] = a[i] * 3;
        } else {
            c[i] = b[i] * 2;
        }
        sum += c[i];
    }
    return sum;
}

/* Test LE_EXPR (<=) - should trigger case LE_EXPR in tree-vect-stmts.cc */
int test_le(int *restrict a, int *restrict b, int *restrict c) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {  /* LE_EXPR */
            c[i] = a[i] + 1;
        } else {
            c[i] = b[i] - 1;
        }
        sum += c[i];
    }
    return sum;
}

/* Test with floating point to ensure both integer and FP comparisons are covered */
float test_float_gt(float *restrict a, float *restrict b, float *restrict c) {
    float sum = 0.0f;
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  /* GT_EXPR with floats */
            c[i] = a[i] * 2.0f;
        } else {
            c[i] = b[i];
        }
        sum += c[i];
    }
    return sum;
}

/* Test with mixed comparisons in same loop */
int test_mixed_comparisons(int *restrict a, int *restrict b, int *restrict c) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        /* Use different comparisons based on position to potentially trigger multiple cases */
        if (i % 4 == 0) {
            if (a[i] > b[i]) c[i] = 1;   /* GT_EXPR */
        } else if (i % 4 == 1) {
            if (a[i] >= b[i]) c[i] = 2;  /* GE_EXPR */
        } else if (i % 4 == 2) {
            if (a[i] < b[i]) c[i] = 3;   /* LT_EXPR */
        } else {
            if (a[i] <= b[i]) c[i] = 4;  /* LE_EXPR */
        }
        sum += c[i];
    }
    return sum;
}

/* Test with ternary operator (conditional expression) */
int test_ternary_gt(int *restrict a, int *restrict b, int *restrict c) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        /* Ternary operator with > comparison */
        c[i] = (a[i] > b[i]) ? (a[i] * 2) : b[i];  /* GT_EXPR in ternary */
        sum += c[i];
    }
    return sum;
}

/* Test with compound conditions */
int test_compound_condition(int *restrict a, int *restrict b, int *restrict c) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        /* Compound condition using > and < */
        if (a[i] > b[i] && a[i] < N/2) {  /* GT_EXPR and LT_EXPR */
            c[i] = a[i] + b[i];
        } else {
            c[i] = 0;
        }
        sum += c[i];
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
    
    /* Initialize arrays */
    init_arrays(a, b);
    init_arrays_float(fa, fb);
    
    int total_checksum = 0;
    
    /* Test each comparison operator separately */
    printf("Testing GT_EXPR (>):\n");
    total_checksum += test_gt(a, b, c);
    printf("  Result checksum: %d\n", test_gt(a, b, c));
    
    printf("Testing GE_EXPR (>=):\n");
    total_checksum += test_ge(a, b, c);
    printf("  Result checksum: %d\n", test_ge(a, b, c));
    
    printf("Testing LT_EXPR (<):\n");
    total_checksum += test_lt(a, b, c);
    printf("  Result checksum: %d\n", test_lt(a, b, c));
    
    printf("Testing LE_EXPR (<=):\n");
    total_checksum += test_le(a, b, c);
    printf("  Result checksum: %d\n", test_le(a, b, c));
    
    printf("Testing float GT_EXPR (>):\n");
    float float_sum = test_float_gt(fa, fb, fc);
    printf("  Result sum: %f\n", float_sum);
    
    printf("Testing mixed comparisons:\n");
    total_checksum += test_mixed_comparisons(a, b, c);
    printf("  Result checksum: %d\n", test_mixed_comparisons(a, b, c));
    
    printf("Testing ternary operator with GT_EXPR:\n");
    total_checksum += test_ternary_gt(a, b, c);
    printf("  Result checksum: %d\n", test_ternary_gt(a, b, c));
    
    printf("Testing compound conditions:\n");
    total_checksum += test_compound_condition(a, b, c);
    printf("  Result checksum: %d\n", test_compound_condition(a, b, c));
    
    printf("\nTotal checksum: %d\n", total_checksum);
    
    /* Free allocated memory */
    free(a); free(b); free(c);
    free(fa); free(fb); free(fc);
    
    return 0;
}

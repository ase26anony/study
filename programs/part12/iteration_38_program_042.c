/* test_vector_comparisons.c
 * Designed to trigger vectorization of comparison operations
 * to cover lines 12216-12233 in tree-vect-stmts.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test functions for each comparison operator */

/* GT_EXPR case */
void test_gt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  /* GT_EXPR - should trigger case GT_EXPR */
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i];
        }
    }
}

/* GE_EXPR case */
void test_ge(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {  /* GE_EXPR - should trigger case GE_EXPR */
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

/* LT_EXPR case */
void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {  /* LT_EXPR - should trigger case LT_EXPR */
            c[i] = a[i] * 3;
        } else {
            c[i] = b[i] * 2;
        }
    }
}

/* LE_EXPR case */
void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {  /* LE_EXPR - should trigger case LE_EXPR */
            c[i] = a[i] + 1;
        } else {
            c[i] = b[i] - 1;
        }
    }
}

/* Additional tests with floating point to ensure different data types */
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

/* Complex pattern with multiple comparisons in one loop */
void test_mixed_comparisons(int *restrict a, int *restrict b, 
                           int *restrict c, int *restrict d) {
    for (int i = 0; i < N; i++) {
        /* Multiple comparisons to potentially trigger different cases */
        if (a[i] > b[i]) {   /* GT_EXPR */
            c[i] = 1;
        } else if (a[i] < b[i]) {  /* LT_EXPR */
            c[i] = -1;
        } else {
            c[i] = 0;
        }
        
        if (a[i] >= b[i]) {  /* GE_EXPR */
            d[i] = a[i];
        } else {
            d[i] = b[i];
        }
    }
}

/* Ternary operator form - another pattern that creates masks */
void test_ternary_gt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] > b[i]) ? a[i] : b[i];  /* GT_EXPR in ternary */
    }
}

void test_ternary_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? a[i] * 2 : b[i] * 3;  /* LE_EXPR in ternary */
    }
}

/* Initialize arrays with patterned data to create varied comparison results */
void init_arrays(int *a, int *b, float *fa, float *fb) {
    for (int i = 0; i < N; i++) {
        a[i] = i;               /* 0, 1, 2, 3, ... */
        b[i] = N - i - 1;       /* 1023, 1022, 1021, ... */
        fa[i] = (float)i * 1.5f;
        fb[i] = (float)(N - i) * 0.5f;
    }
}

/* Verify results to ensure code isn't optimized away */
int verify_results(int *c1, int *c2, int *c3, int *c4, 
                   int *c5, int *c6, float *fc1, float *fc2) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += c1[i] + c2[i] + c3[i] + c4[i] + c5[i] + c6[i];
        sum += (int)fc1[i] + (int)fc2[i];
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
    
    float *fa = (float*)aligned_alloc(32, N * sizeof(float));
    float *fb = (float*)aligned_alloc(32, N * sizeof(float));
    float *fc1 = (float*)aligned_alloc(32, N * sizeof(float));
    float *fc2 = (float*)aligned_alloc(32, N * sizeof(float));
    
    if (!a || !b || !c1 || !c2 || !c3 || !c4 || !c5 || !c6 || !d ||
        !fa || !fb || !fc1 || !fc2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(a, b, fa, fb);
    
    /* Execute all test functions to trigger each comparison case */
    test_gt(a, b, c1);           /* GT_EXPR */
    test_ge(a, b, c2);           /* GE_EXPR */
    test_lt(a, b, c3);           /* LT_EXPR */
    test_le(a, b, c4);           /* LE_EXPR */
    
    test_gt_float(fa, fb, fc1);  /* GT_EXPR with float */
    test_ge_float(fa, fb, fc2);  /* GE_EXPR with float */
    
    test_mixed_comparisons(a, b, c5, d);  /* Multiple comparisons */
    test_ternary_gt(a, b, c6);            /* GT_EXPR in ternary */
    
    /* Verify results to prevent dead code elimination */
    int checksum = verify_results(c1, c2, c3, c4, c5, c6, fc1, fc2);
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c1); free(c2); free(c3); free(c4);
    free(c5); free(c6); free(d); free(fa); free(fb); free(fc1); free(fc2);
    
    return 0;
}

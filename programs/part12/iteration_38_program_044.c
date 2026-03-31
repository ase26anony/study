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
        if (a[i] > b[i]) {
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

/* GE_EXPR case */
void test_ge(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] * 2;
        }
    }
}

/* LT_EXPR case */
void test_lt(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] / (b[i] + 1.0f);
        }
    }
}

/* LE_EXPR case */
void test_le(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

/* Mixed comparisons in same loop to test multiple cases */
void test_mixed_comparisons(int *restrict a, int *restrict b, 
                           int *restrict c, int *restrict d) {
    for (int i = 0; i < N; i++) {
        /* This should trigger GT_EXPR transformation */
        if (a[i] > b[i]) {
            c[i] = 1;
        } else {
            c[i] = 0;
        }
        
        /* This should trigger LT_EXPR transformation */
        if (a[i] < b[i]) {
            d[i] = 1;
        } else {
            d[i] = 0;
        }
    }
}

/* Another variant with GE and LE */
void test_mixed_comparisons2(float *restrict a, float *restrict b,
                            float *restrict c, float *restrict d) {
    for (int i = 0; i < N; i++) {
        /* GE_EXPR case */
        c[i] = (a[i] >= b[i]) ? a[i] : b[i];
        
        /* LE_EXPR case */
        d[i] = (a[i] <= b[i]) ? a[i] + b[i] : a[i] - b[i];
    }
}

/* Initialize arrays with patterned data to ensure varied comparisons */
void init_arrays(int *a, int *b, float *fa, float *fb) {
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i - 1;  /* Creates mix of true/false comparisons */
        fa[i] = (float)i * 1.5f;
        fb[i] = (float)(N - i) * 1.5f;
    }
}

/* Compute checksum to ensure loops execute and produce results */
int compute_checksum(int *c, float *fc) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += c[i];
        sum += (int)fc[i];
    }
    return sum;
}

int main() {
    /* Use aligned allocations for better vectorization */
    int *a = (int*)aligned_alloc(32, N * sizeof(int));
    int *b = (int*)aligned_alloc(32, N * sizeof(int));
    int *c_int = (int*)aligned_alloc(32, N * sizeof(int));
    int *d_int = (int*)aligned_alloc(32, N * sizeof(int));
    
    float *fa = (float*)aligned_alloc(32, N * sizeof(float));
    float *fb = (float*)aligned_alloc(32, N * sizeof(float));
    float *c_float = (float*)aligned_alloc(32, N * sizeof(float));
    float *d_float = (float*)aligned_alloc(32, N * sizeof(float));
    
    if (!a || !b || !c_int || !d_int || !fa || !fb || !c_float || !d_float) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with patterned data */
    init_arrays(a, b, fa, fb);
    
    /* Test each comparison operator separately */
    test_gt(a, b, c_int);
    test_ge(a, b, d_int);
    test_lt(fa, fb, c_float);
    test_le(fa, fb, d_float);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(a, b, c_int, d_int);
    test_mixed_comparisons2(fa, fb, c_float, d_float);
    
    /* Compute and print checksum to ensure execution */
    int checksum = compute_checksum(c_int, c_float);
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c_int); free(d_int);
    free(fa); free(fb); free(c_float); free(d_float);
    
    return 0;
}

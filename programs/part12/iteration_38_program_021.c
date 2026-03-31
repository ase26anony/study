/* test_vectorize_comparisons.c
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
        if (a[i] > b[i]) {  /* GT_EXPR */
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

/* GE_EXPR case */
void test_ge(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {  /* GE_EXPR */
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] / (b[i] + 1.0f);
        }
    }
}

/* LT_EXPR case */
void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {  /* LT_EXPR */
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] * 3;
        }
    }
}

/* LE_EXPR case */
void test_le(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {  /* LE_EXPR */
            c[i] = a[i] - b[i];
        } else {
            c[i] = b[i] - a[i];
        }
    }
}

/* Additional test with mixed comparisons in same loop */
void test_mixed_comparisons(int *restrict a, int *restrict b, int *restrict c, 
                           int *restrict d) {
    for (int i = 0; i < N; i++) {
        /* Multiple comparisons that should all be vectorized */
        if (a[i] > b[i]) {   /* GT_EXPR */
            c[i] = 1;
        } else if (a[i] >= b[i]) {  /* GE_EXPR */
            c[i] = 2;
        } else if (a[i] < b[i]) {   /* LT_EXPR */
            c[i] = 3;
        } else if (a[i] <= b[i]) {  /* LE_EXPR */
            c[i] = 4;
        } else {
            c[i] = 0;
        }
        
        /* Another comparison for d array */
        d[i] = (a[i] <= b[i]) ? a[i] : b[i];  /* LE_EXPR in ternary */
    }
}

/* Test with nested comparisons */
void test_nested_comparisons(int *restrict a, int *restrict b, 
                            int *restrict c, int *restrict d) {
    for (int i = 0; i < N; i++) {
        /* Complex condition with multiple comparisons */
        if ((a[i] > b[i]) && (a[i] < c[i])) {  /* GT_EXPR and LT_EXPR */
            d[i] = a[i];
        } else if (a[i] >= c[i] || b[i] <= c[i]) {  /* GE_EXPR and LE_EXPR */
            d[i] = b[i];
        } else {
            d[i] = c[i];
        }
    }
}

/* Initialize arrays with patterned data */
void init_arrays(int *a, int *b, float *fa, float *fb) {
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i - 1;  /* Creates varying comparison results */
        fa[i] = (float)i * 1.5f;
        fb[i] = (float)(N - i) * 0.7f;
    }
}

/* Compute checksum to prevent dead code elimination */
int compute_checksum(int *arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

float compute_fchecksum(float *arr, int size) {
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

int main() {
    /* Use aligned allocation for better vectorization */
    int *a = (int*)aligned_alloc(32, N * sizeof(int));
    int *b = (int*)aligned_alloc(32, N * sizeof(int));
    int *c_int = (int*)aligned_alloc(32, N * sizeof(int));
    int *d_int = (int*)aligned_alloc(32, N * sizeof(int));
    int *e_int = (int*)aligned_alloc(32, N * sizeof(int));
    
    float *fa = (float*)aligned_alloc(32, N * sizeof(float));
    float *fb = (float*)aligned_alloc(32, N * sizeof(float));
    float *fc_float = (float*)aligned_alloc(32, N * sizeof(float));
    float *fd_float = (float*)aligned_alloc(32, N * sizeof(float));
    
    if (!a || !b || !c_int || !d_int || !e_int || 
        !fa || !fb || !fc_float || !fd_float) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with patterned data */
    init_arrays(a, b, fa, fb);
    
    /* Test each comparison operator separately */
    test_gt(a, b, c_int);
    test_ge(fa, fb, fc_float);
    test_lt(a, b, d_int);
    test_le(fa, fb, fd_float);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(a, b, c_int, d_int);
    
    /* Test nested comparisons */
    test_nested_comparisons(a, b, c_int, e_int);
    
    /* Compute checksums to ensure code isn't eliminated */
    int checksum1 = compute_checksum(c_int, N);
    int checksum2 = compute_checksum(d_int, N);
    int checksum3 = compute_checksum(e_int, N);
    float fchecksum1 = compute_fchecksum(fc_float, N);
    float fchecksum2 = compute_fchecksum(fd_float, N);
    
    printf("Checksums: %d %d %d %.2f %.2f\n", 
           checksum1, checksum2, checksum3, fchecksum1, fchecksum2);
    
    /* Free allocated memory */
    free(a); free(b); free(c_int); free(d_int); free(e_int);
    free(fa); free(fb); free(fc_float); free(fd_float);
    
    return 0;
}

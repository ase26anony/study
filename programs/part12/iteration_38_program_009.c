/* test_vectorized_comparisons.c
 * Designed to trigger vectorization of GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR
 * and reach the uncovered switch cases in tree-vect-stmts.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Separate test functions for each comparison operator */

void test_gt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  /* GT_EXPR - should trigger case GT_EXPR */
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] / 2;
        }
    }
}

void test_ge(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {  /* GE_EXPR - should trigger case GE_EXPR */
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {  /* LT_EXPR - should trigger case LT_EXPR */
            c[i] = a[i] | b[i];
        } else {
            c[i] = a[i] & b[i];
        }
    }
}

void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {  /* LE_EXPR - should trigger case LE_EXPR */
            c[i] = a[i] ^ b[i];
        } else {
            c[i] = a[i] << 1;
        }
    }
}

/* Additional tests with floating point to ensure different data types */
void test_gt_float(float *restrict fa, float *restrict fb, float *restrict fc) {
    for (int i = 0; i < N; i++) {
        if (fa[i] > fb[i]) {  /* GT_EXPR with floats */
            fc[i] = fa[i] * 2.0f;
        } else {
            fc[i] = fb[i] / 2.0f;
        }
    }
}

void test_ge_float(float *restrict fa, float *restrict fb, float *restrict fc) {
    for (int i = 0; i < N; i++) {
        if (fa[i] >= fb[i]) {  /* GE_EXPR with floats */
            fc[i] = fa[i] + fb[i];
        } else {
            fc[i] = fa[i] - fb[i];
        }
    }
}

/* Test with more complex conditional expressions */
void test_complex_cond(int *restrict a, int *restrict b, int *restrict c, 
                       int *restrict d, int *restrict out) {
    for (int i = 0; i < N; i++) {
        /* Complex condition that should still vectorize */
        if ((a[i] > b[i]) && (c[i] <= d[i])) {  /* Mix of GT_EXPR and LE_EXPR */
            out[i] = a[i] + c[i];
        } else if (a[i] < b[i]) {  /* LT_EXPR */
            out[i] = b[i] - d[i];
        } else {
            out[i] = 0;
        }
    }
}

/* Initialize arrays with patterned data to create varied comparison results */
void init_arrays(int *a, int *b, int *c, float *fa, float *fb, float *fc,
                 int *d, int *e, int *f) {
    for (int i = 0; i < N; i++) {
        /* Create data patterns that ensure all comparison outcomes occur */
        a[i] = i;                    /* 0, 1, 2, 3, ... */
        b[i] = N - i - 1;            /* 1023, 1022, 1021, ... */
        c[i] = (i % 2 == 0) ? i : -i; /* Alternating positive/negative */
        
        fa[i] = (float)i * 1.5f;
        fb[i] = (float)(N - i) * 0.5f;
        fc[i] = 0.0f;
        
        d[i] = i * 2;
        e[i] = i * 3;
        f[i] = 0;
    }
}

/* Compute checksum to verify correctness and prevent dead code elimination */
int compute_checksum(int *arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

float compute_checksum_float(float *arr, int size) {
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

int main() {
    /* Use aligned allocations to help vectorization */
    int *a = aligned_alloc(32, N * sizeof(int));
    int *b = aligned_alloc(32, N * sizeof(int));
    int *c = aligned_alloc(32, N * sizeof(int));
    int *d = aligned_alloc(32, N * sizeof(int));
    int *e = aligned_alloc(32, N * sizeof(int));
    int *f = aligned_alloc(32, N * sizeof(int));
    int *out = aligned_alloc(32, N * sizeof(int));
    
    float *fa = aligned_alloc(32, N * sizeof(float));
    float *fb = aligned_alloc(32, N * sizeof(float));
    float *fc = aligned_alloc(32, N * sizeof(float));
    
    if (!a || !b || !c || !d || !e || !f || !out || !fa || !fb || !fc) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(a, b, c, fa, fb, fc, d, e, f);
    
    /* Execute all test functions to trigger each comparison case */
    test_gt(a, b, c);
    int sum1 = compute_checksum(c, N);
    
    test_ge(a, b, c);
    int sum2 = compute_checksum(c, N);
    
    test_lt(a, b, c);
    int sum3 = compute_checksum(c, N);
    
    test_le(a, b, c);
    int sum4 = compute_checksum(c, N);
    
    test_gt_float(fa, fb, fc);
    float sum5 = compute_checksum_float(fc, N);
    
    test_ge_float(fa, fb, fc);
    float sum6 = compute_checksum_float(fc, N);
    
    test_complex_cond(a, b, c, d, out);
    int sum7 = compute_checksum(out, N);
    
    /* Print results to prevent optimization */
    printf("Checksums: %d %d %d %d %.2f %.2f %d\n", 
           sum1, sum2, sum3, sum4, sum5, sum6, sum7);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d); free(e); free(f); free(out);
    free(fa); free(fb); free(fc);
    
    return 0;
}

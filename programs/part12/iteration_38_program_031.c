/* test_vector_comparisons.c
 * Designed to trigger vectorization of comparison operations
 * to cover lines 12216-12233 in tree-vect-stmts.cc
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test functions for each comparison operator */

void test_gt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

void test_ge(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] * 2;
        }
    }
}

void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            c[i] = a[i] | b[i];
        } else {
            c[i] = a[i] & b[i];
        }
    }
}

void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            c[i] = a[i] ^ b[i];
        } else {
            c[i] = a[i] << 2;
        }
    }
}

/* Additional tests with floating point to ensure different data types */
void test_gt_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

void test_ge_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            c[i] = a[i] * 2.0f;
        } else {
            c[i] = b[i] * 2.0f;
        }
    }
}

void test_lt_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] / (b[i] + 1.0f);
        }
    }
}

void test_le_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            c[i] = a[i] - b[i];
        } else {
            c[i] = b[i] - a[i];
        }
    }
}

/* Test with more complex conditional expressions */
void test_complex_gt(int *restrict a, int *restrict b, int *restrict c, int *restrict d) {
    for (int i = 0; i < N; i++) {
        /* Complex condition that should still vectorize */
        if ((a[i] > b[i]) && (c[i] > 0)) {
            d[i] = a[i] + b[i] + c[i];
        } else {
            d[i] = a[i] - b[i];
        }
    }
}

void test_complex_lt(int *restrict a, int *restrict b, int *restrict c, int *restrict d) {
    for (int i = 0; i < N; i++) {
        /* Another complex condition */
        if ((a[i] < b[i]) || (c[i] < 0)) {
            d[i] = a[i] * b[i];
        } else {
            d[i] = a[i] / (b[i] + 1);
        }
    }
}

/* Initialize arrays with patterned data to ensure varied comparison results */
void init_arrays(int *a, int *b, int *c, float *fa, float *fb, float *fc) {
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i - 1;  /* Creates mixed comparison results */
        c[i] = 0;
        
        fa[i] = (float)i * 1.5f;
        fb[i] = (float)(N - i) * 1.5f;
        fc[i] = 0.0f;
    }
}

/* Compute checksum to ensure loops execute and produce results */
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
    int *c = (int*)aligned_alloc(32, N * sizeof(int));
    int *d = (int*)aligned_alloc(32, N * sizeof(int));
    int *e = (int*)aligned_alloc(32, N * sizeof(int));
    
    float *fa = (float*)aligned_alloc(32, N * sizeof(float));
    float *fb = (float*)aligned_alloc(32, N * sizeof(float));
    float *fc = (float*)aligned_alloc(32, N * sizeof(float));
    
    if (!a || !b || !c || !d || !e || !fa || !fb || !fc) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(a, b, c, fa, fb, fc);
    
    /* Execute all test functions to trigger each comparison case */
    test_gt(a, b, c);
    int sum1 = compute_checksum(c);
    
    test_ge(a, b, d);
    int sum2 = compute_checksum(d);
    
    test_lt(a, b, e);
    int sum3 = compute_checksum(e);
    
    test_le(a, b, c);
    int sum4 = compute_checksum(c);
    
    test_gt_float(fa, fb, fc);
    float sum5 = compute_checksum_float(fc);
    
    test_ge_float(fa, fb, fc);
    float sum6 = compute_checksum_float(fc);
    
    test_lt_float(fa, fb, fc);
    float sum7 = compute_checksum_float(fc);
    
    test_le_float(fa, fb, fc);
    float sum8 = compute_checksum_float(fc);
    
    test_complex_gt(a, b, c, d);
    int sum9 = compute_checksum(d);
    
    test_complex_lt(a, b, c, e);
    int sum10 = compute_checksum(e);
    
    /* Print results to prevent dead code elimination */
    printf("Checksums: %d %d %d %d %f %f %f %f %d %d\n", 
           sum1, sum2, sum3, sum4, sum5, sum6, sum7, sum8, sum9, sum10);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d); free(e);
    free(fa); free(fb); free(fc);
    
    return 0;
}

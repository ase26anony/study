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

void test_gt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  /* GT_EXPR */
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

void test_ge(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {  /* GE_EXPR */
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] * 3;
        }
    }
}

void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {  /* LT_EXPR */
            c[i] = a[i] << 1;
        } else {
            c[i] = b[i] >> 1;
        }
    }
}

void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {  /* LE_EXPR */
            c[i] = a[i] | b[i];
        } else {
            c[i] = a[i] & b[i];
        }
    }
}

/* Additional tests with floating point to ensure different data types */
void test_gt_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  /* GT_EXPR with floats */
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

void test_ge_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {  /* GE_EXPR with floats */
            c[i] = a[i] * 2.0f;
        } else {
            c[i] = b[i] * 3.0f;
        }
    }
}

/* Test with ternary operator (another pattern that creates masks) */
void test_lt_ternary(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? a[i] + 100 : b[i] - 100;  /* LT_EXPR in ternary */
    }
}

void test_le_ternary(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? a[i] * b[i] : a[i] / (b[i] + 1);  /* LE_EXPR in ternary */
    }
}

/* Complex condition with multiple comparisons */
void test_complex_condition(int *restrict a, int *restrict b, int *restrict c, int *restrict d) {
    for (int i = 0; i < N; i++) {
        if ((a[i] > b[i]) && (c[i] < d[i])) {  /* GT_EXPR and LT_EXPR combined */
            a[i] = b[i] + c[i];
        } else {
            a[i] = b[i] - d[i];
        }
    }
}

/* Initialize arrays with pattern to ensure varied comparison results */
void init_arrays(int *a, int *b, float *fa, float *fb) {
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i - 1;  /* Creates mixed true/false conditions */
        fa[i] = (float)i * 1.5f;
        fb[i] = (float)(N - i) * 0.75f;
    }
}

/* Verify results by computing checksum */
int verify_results(int *c) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += c[i];
    }
    return sum;
}

float verify_results_float(float *c) {
    float sum = 0.0f;
    for (int i = 0; i < N; i++) {
        sum += c[i];
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
    
    if (!a || !b || !c1 || !c2 || !c3 || !c4 || !c5 || !c6 || !d || !fa || !fb || !fc1 || !fc2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(a, b, fa, fb);
    
    /* Test all comparison operators with different patterns */
    test_gt(a, b, c1);
    test_ge(a, b, c2);
    test_lt(a, b, c3);
    test_le(a, b, c4);
    test_lt_ternary(a, b, c5);
    test_le_ternary(a, b, c6);
    test_complex_condition(a, b, c1, d);
    
    /* Test floating point comparisons */
    test_gt_float(fa, fb, fc1);
    test_ge_float(fa, fb, fc2);
    
    /* Compute checksums to ensure code executes */
    int checksum1 = verify_results(c1);
    int checksum2 = verify_results(c2);
    int checksum3 = verify_results(c3);
    int checksum4 = verify_results(c4);
    float checksum5 = verify_results_float(fc1);
    float checksum6 = verify_results_float(fc2);
    
    printf("Checksums: %d %d %d %d %.2f %.2f\n", 
           checksum1, checksum2, checksum3, checksum4, checksum5, checksum6);
    
    /* Cleanup */
    free(a); free(b); free(c1); free(c2); free(c3); free(c4); free(c5); free(c6); free(d);
    free(fa); free(fb); free(fc1); free(fc2);
    
    return 0;
}

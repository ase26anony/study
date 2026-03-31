/* test_vectorize_comparisons.c
 * Designed to trigger vectorization of comparison operations
 * to cover lines 12216-12233 in tree-vect-stmts.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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
            c[i] = b[i] * 2;
        }
    }
}

void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {  /* LT_EXPR */
            c[i] = a[i] | b[i];
        } else {
            c[i] = a[i] & b[i];
        }
    }
}

void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {  /* LE_EXPR */
            c[i] = a[i] ^ b[i];
        } else {
            c[i] = a[i] + b[i] * 3;
        }
    }
}

/* Additional tests with floating point to ensure different data types */
void test_gt_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  /* GT_EXPR with float */
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] + b[i];
        }
    }
}

void test_ge_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {  /* GE_EXPR with float */
            c[i] = a[i] - b[i];
        } else {
            c[i] = b[i] - a[i];
        }
    }
}

/* Test with mixed operations to create more complex mask usage */
void test_mixed_comparisons(int *restrict a, int *restrict b, 
                           int *restrict c, int *restrict d) {
    for (int i = 0; i < N; i++) {
        /* Multiple comparisons that need to be combined */
        int cond1 = a[i] > b[i];   /* GT_EXPR */
        int cond2 = a[i] <= c[i];  /* LE_EXPR */
        int cond3 = b[i] < d[i];   /* LT_EXPR */
        
        if (cond1 && (cond2 || cond3)) {
            d[i] = a[i] + b[i] + c[i];
        } else {
            d[i] = a[i] - b[i] - c[i];
        }
    }
}

/* Initialize arrays with patterned data to ensure varied comparison results */
void init_arrays(int *a, int *b, float *fa, float *fb) {
    for (int i = 0; i < N; i++) {
        a[i] = i;                /* 0, 1, 2, 3, ... */
        b[i] = N - i - 1;        /* 1023, 1022, 1021, ... */
        fa[i] = (float)i * 1.5f;
        fb[i] = (float)(N - i) * 0.5f;
    }
}

/* Compute checksum to verify correctness and prevent dead code elimination */
int compute_checksum(int *arr) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum ^= arr[i];
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
    int *a = aligned_alloc(32, N * sizeof(int));
    int *b = aligned_alloc(32, N * sizeof(int));
    int *c1 = aligned_alloc(32, N * sizeof(int));
    int *c2 = aligned_alloc(32, N * sizeof(int));
    int *c3 = aligned_alloc(32, N * sizeof(int));
    int *c4 = aligned_alloc(32, N * sizeof(int));
    int *c5 = aligned_alloc(32, N * sizeof(int));
    
    float *fa = aligned_alloc(32, N * sizeof(float));
    float *fb = aligned_alloc(32, N * sizeof(float));
    float *fc1 = aligned_alloc(32, N * sizeof(float));
    float *fc2 = aligned_alloc(32, N * sizeof(float));
    
    if (!a || !b || !c1 || !c2 || !c3 || !c4 || !c5 || !fa || !fb || !fc1 || !fc2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(a, b, fa, fb);
    
    /* Execute all test functions to trigger each comparison case */
    test_gt(a, b, c1);
    test_ge(a, b, c2);
    test_lt(a, b, c3);
    test_le(a, b, c4);
    test_gt_float(fa, fb, fc1);
    test_ge_float(fa, fb, fc2);
    test_mixed_comparisons(a, b, c1, c5);
    
    /* Compute and print checksums to ensure code executes */
    printf("Checksums:\n");
    printf("test_gt: %d\n", compute_checksum(c1));
    printf("test_ge: %d\n", compute_checksum(c2));
    printf("test_lt: %d\n", compute_checksum(c3));
    printf("test_le: %d\n", compute_checksum(c4));
    printf("test_mixed: %d\n", compute_checksum(c5));
    printf("test_gt_float: %f\n", compute_checksum_float(fc1));
    printf("test_ge_float: %f\n", compute_checksum_float(fc2));
    
    /* Cleanup */
    free(a); free(b); free(c1); free(c2); free(c3); free(c4); free(c5);
    free(fa); free(fb); free(fc1); free(fc2);
    
    return 0;
}

/* test_vectorized_comparisons.c
 * 
 * This program contains vectorizable loops with comparison operations
 * (>, >=, <, <=) that should trigger the transformation logic in
 * tree-vect-stmts.cc lines 12216-12233.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test function for GT_EXPR (>) */
void test_gt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        /* This should trigger GT_EXPR case */
        if (a[i] > b[i]) {
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

/* Test function for GE_EXPR (>=) */
void test_ge(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        /* This should trigger GE_EXPR case */
        if (a[i] >= b[i]) {
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] / (b[i] + 1.0f);
        }
    }
}

/* Test function for LT_EXPR (<) */
void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        /* This should trigger LT_EXPR case with operand swap */
        if (a[i] < b[i]) {
            c[i] = b[i] - a[i];
        } else {
            c[i] = a[i] + b[i] * 2;
        }
    }
}

/* Test function for LE_EXPR (<=) */
void test_le(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        /* This should trigger LE_EXPR case with operand swap */
        if (a[i] <= b[i]) {
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

/* Alternative test using ternary operator (also vectorizable) */
void test_gt_ternary(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        /* Ternary with > comparison */
        c[i] = (a[i] > b[i]) ? (a[i] * 2) : (b[i] * 3);
    }
}

/* Test with mixed comparisons in same loop */
void test_mixed_comparisons(int *restrict a, int *restrict b, int *restrict c, 
                           int *restrict d) {
    for (int i = 0; i < N; i++) {
        /* Multiple comparisons that should all be vectorized */
        int gt_mask = (a[i] > b[i]) ? 1 : 0;
        int ge_mask = (a[i] >= b[i]) ? 1 : 0;
        int lt_mask = (a[i] < b[i]) ? 1 : 0;
        int le_mask = (a[i] <= b[i]) ? 1 : 0;
        
        c[i] = gt_mask * a[i] + ge_mask * b[i];
        d[i] = lt_mask * (a[i] - b[i]) + le_mask * (a[i] + b[i]);
    }
}

/* Initialize arrays with pattern that creates varied comparison results */
void init_arrays(int *a, int *b, float *fa, float *fb) {
    for (int i = 0; i < N; i++) {
        a[i] = i;                    /* 0, 1, 2, 3, ... */
        b[i] = N - i;                /* 1024, 1023, 1022, ... */
        fa[i] = (float)i / 10.0f;    /* 0.0, 0.1, 0.2, ... */
        fb[i] = (float)(N - i) / 5.0f; /* 204.8, 204.6, 204.4, ... */
    }
}

/* Compute checksum to ensure loops execute and produce results */
uint64_t compute_checksum_int(int *arr) {
    uint64_t sum = 0;
    for (int i = 0; i < N; i++) {
        sum += (uint64_t)arr[i];
    }
    return sum;
}

float compute_checksum_float(float *arr) {
    double sum = 0.0;
    for (int i = 0; i < N; i++) {
        sum += (double)arr[i];
    }
    return (float)sum;
}

int main() {
    /* Use aligned allocations for better vectorization */
    int *a_int = (int*)aligned_alloc(32, N * sizeof(int));
    int *b_int = (int*)aligned_alloc(32, N * sizeof(int));
    int *c_int = (int*)aligned_alloc(32, N * sizeof(int));
    int *d_int = (int*)aligned_alloc(32, N * sizeof(int));
    int *e_int = (int*)aligned_alloc(32, N * sizeof(int));
    
    float *a_float = (float*)aligned_alloc(32, N * sizeof(float));
    float *b_float = (float*)aligned_alloc(32, N * sizeof(float));
    float *c_float = (float*)aligned_alloc(32, N * sizeof(float));
    float *d_float = (float*)aligned_alloc(32, N * sizeof(float));
    
    if (!a_int || !b_int || !c_int || !d_int || !e_int ||
        !a_float || !b_float || !c_float || !d_float) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with patterns that ensure all comparisons are exercised */
    init_arrays(a_int, b_int, a_float, b_float);
    
    /* Execute all test functions to trigger different comparison cases */
    test_gt(a_int, b_int, c_int);
    test_ge(a_float, b_float, c_float);
    test_lt(a_int, b_int, d_int);
    test_le(a_float, b_float, d_float);
    test_gt_ternary(a_int, b_int, e_int);
    
    /* Test with mixed comparisons */
    int *f_int = (int*)aligned_alloc(32, N * sizeof(int));
    int *g_int = (int*)aligned_alloc(32, N * sizeof(int));
    if (f_int && g_int) {
        test_mixed_comparisons(a_int, b_int, f_int, g_int);
    }
    
    /* Compute and print checksums to ensure code executed */
    printf("Checksum GT: %lu\n", compute_checksum_int(c_int));
    printf("Checksum GE: %f\n", compute_checksum_float(c_float));
    printf("Checksum LT: %lu\n", compute_checksum_int(d_int));
    printf("Checksum LE: %f\n", compute_checksum_float(d_float));
    printf("Checksum GT ternary: %lu\n", compute_checksum_int(e_int));
    
    if (f_int && g_int) {
        printf("Checksum mixed 1: %lu\n", compute_checksum_int(f_int));
        printf("Checksum mixed 2: %lu\n", compute_checksum_int(g_int));
        free(f_int);
        free(g_int);
    }
    
    /* Cleanup */
    free(a_int); free(b_int); free(c_int); free(d_int); free(e_int);
    free(a_float); free(b_float); free(c_float); free(d_float);
    
    return 0;
}

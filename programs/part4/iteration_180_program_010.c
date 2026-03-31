/* test_vectorized_comparisons.c
 * Designed to trigger vectorization of conditional expressions
 * and hit the bit-operation transformation in tree-vect-stmts.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGN 32

/* Aligned allocations for better vectorization */
static void* aligned_alloc(size_t align, size_t size) {
    void* ptr;
    if (posix_memalign(&ptr, align, size) != 0) {
        return NULL;
    }
    return ptr;
}

/* GT_EXPR (>) test with conditional assignment */
void test_gt_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict d) {
    for (int i = 0; i < N; ++i) {
        /* This should generate GT_EXPR comparison */
        if (a[i] > b[i]) {
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] / 2;
        }
        /* Additional operation to prevent optimization */
        d[i] = (a[i] > b[i]) ? a[i] + b[i] : a[i] - b[i];
    }
}

/* GE_EXPR (>=) test with conditional reduction */
int test_ge_expr(int* restrict a, int* restrict b, int* restrict c) {
    int sum = 0;
    for (int i = 0; i < N; ++i) {
        /* This should generate GE_EXPR comparison */
        if (a[i] >= b[i]) {
            c[i] = a[i] * b[i];
            sum += c[i];
        } else {
            c[i] = a[i] + b[i];
            sum -= c[i];
        }
    }
    return sum;
}

/* LT_EXPR (<) test with masked store */
void test_lt_expr(float* restrict a, float* restrict b, float* restrict c) {
    for (int i = 0; i < N; ++i) {
        /* This should generate LT_EXPR comparison */
        if (a[i] < b[i]) {
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] + b[i];
        }
    }
}

/* LE_EXPR (<=) test with conditional blend pattern */
void test_le_expr(double* restrict a, double* restrict b, double* restrict c, double* restrict d) {
    for (int i = 0; i < N; ++i) {
        /* This should generate LE_EXPR comparison */
        double temp = (a[i] <= b[i]) ? a[i] * 3.0 : b[i] * 2.0;
        c[i] = temp;
        d[i] = (a[i] <= b[i]) ? a[i] - b[i] : b[i] - a[i];
    }
}

/* Mixed comparisons to ensure all paths are exercised */
void test_mixed_comparisons(int* restrict a, int* restrict b, int* restrict c) {
    for (int i = 0; i < N; ++i) {
        /* Mix of all four comparison types */
        if (a[i] > b[i]) {
            c[i] += 1;
        }
        if (a[i] >= b[i]) {
            c[i] += 2;
        }
        if (a[i] < b[i]) {
            c[i] += 4;
        }
        if (a[i] <= b[i]) {
            c[i] += 8;
        }
    }
}

/* Initialize arrays with pattern that creates mix of true/false comparisons */
void init_arrays(int* a, int* b, float* fa, float* fb, double* da, double* db) {
    for (int i = 0; i < N; ++i) {
        /* Create varying patterns to ensure all comparison results occur */
        a[i] = i;
        b[i] = N/2 - i % 100;  /* Creates mix of >, <, == cases */
        
        fa[i] = (float)i * 1.5f;
        fb[i] = (float)(N - i) * 0.7f;
        
        da[i] = (double)i * 2.5;
        db[i] = (double)(i % 50) * 3.0;
    }
}

/* Verify results to prevent dead code elimination */
int verify_results(int* c1, int* c2, float* c3, double* c4, double* c5, int* c6) {
    int checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += c1[i] + c2[i] + (int)c3[i] + (int)c4[i] + (int)c5[i] + c6[i];
    }
    return checksum;
}

int main() {
    /* Allocate aligned arrays for better vectorization */
    int* a_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* b_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c1 = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c2 = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* d_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c6 = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    
    float* a_float = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* b_float = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* c3 = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    
    double* a_double = (double*)aligned_alloc(ALIGN, N * sizeof(double));
    double* b_double = (double*)aligned_alloc(ALIGN, N * sizeof(double));
    double* c4 = (double*)aligned_alloc(ALIGN, N * sizeof(double));
    double* c5 = (double*)aligned_alloc(ALIGN, N * sizeof(double));
    
    if (!a_int || !b_int || !c1 || !c2 || !d_int || !c6 ||
        !a_float || !b_float || !c3 ||
        !a_double || !b_double || !c4 || !c5) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with patterns that create varied comparison results */
    init_arrays(a_int, b_int, a_float, b_float, a_double, b_double);
    memset(c1, 0, N * sizeof(int));
    memset(c2, 0, N * sizeof(int));
    memset(c6, 0, N * sizeof(int));
    
    /* Execute all test functions to trigger different comparison types */
    test_gt_expr(a_int, b_int, c1, d_int);
    
    int sum = test_ge_expr(a_int, b_int, c2);
    
    test_lt_expr(a_float, b_float, c3);
    
    test_le_expr(a_double, b_double, c4, c5);
    
    test_mixed_comparisons(a_int, b_int, c6);
    
    /* Verify results to prevent optimization */
    int checksum = verify_results(c1, c2, c3, c4, c5, c6);
    
    /* Use results to affect program output */
    printf("Test completed. Checksum: %d (sum from GE: %d)\n", checksum, sum);
    
    /* Cleanup */
    free(a_int); free(b_int); free(c1); free(c2); free(d_int); free(c6);
    free(a_float); free(b_float); free(c3);
    free(a_double); free(b_double); free(c4); free(c5);
    
    return 0;
}

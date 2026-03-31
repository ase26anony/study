/* test_vector_cond_bitops.c
 * 
 * This program creates vectorizable loops with conditional operations
 * using all four comparison operators (>, >=, <, <=) to trigger
 * the transformation of comparisons to bit operations in GCC's
 * tree vectorization passes (tree-vect-stmts.cc lines 12216-12233).
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define N 1024
#define ALIGN 32

/* Aligned allocation for better vectorization */
static void* aligned_alloc(size_t size) {
    void* ptr;
    if (posix_memalign(&ptr, ALIGN, size) != 0) {
        return NULL;
    }
    return ptr;
}

/* GT_EXPR (>) - Conditional assignment pattern */
void test_gt_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict d) {
    for (int i = 0; i < N; ++i) {
        /* This should generate GT_EXPR comparison */
        if (a[i] > b[i]) {
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] / 2;
        }
        /* Additional operation to prevent dead code elimination */
        d[i] = (a[i] > b[i]) ? a[i] + b[i] : a[i] - b[i];
    }
}

/* GE_EXPR (>=) - Conditional reduction pattern */
int test_ge_expr(int* restrict a, int* restrict b, int* restrict c) {
    int sum = 0;
    for (int i = 0; i < N; ++i) {
        /* This should generate GE_EXPR comparison */
        if (a[i] >= b[i]) {
            c[i] = a[i] + b[i];
            sum += c[i];
        } else {
            c[i] = a[i] - b[i];
            sum -= c[i];
        }
    }
    return sum;
}

/* LT_EXPR (<) - Masked store pattern */
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

/* LE_EXPR (<=) - Conditional blend pattern */
void test_le_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict d, int* restrict out) {
    for (int i = 0; i < N; ++i) {
        /* This should generate LE_EXPR comparison */
        int mask = (a[i] <= b[i]);
        /* Blend operation using conditional */
        out[i] = mask ? c[i] : d[i];
        /* Additional computation to ensure vectorization */
        out[i] += (a[i] <= b[i]) ? 1 : -1;
    }
}

/* Mixed comparisons to ensure all paths are exercised */
void test_mixed_comparisons(int* restrict a, int* restrict b, int* restrict c, 
                           float* restrict fa, float* restrict fb, float* restrict fc) {
    /* Test GT and GE together */
    for (int i = 0; i < N/2; ++i) {
        if (a[i] > b[i]) {
            c[i] = a[i] * 3;
        }
        if (a[i] >= b[i]) {
            c[i] += b[i];
        }
    }
    
    /* Test LT and LE together */
    for (int i = N/2; i < N; ++i) {
        if (fa[i] < fb[i]) {
            fc[i] = fa[i] * fb[i];
        }
        if (fa[i] <= fb[i]) {
            fc[i] += 1.0f;
        }
    }
}

/* Initialize arrays with patterns that create mixed true/false results */
void init_arrays(int* a, int* b, int* c, int* d,
                 float* fa, float* fb, float* fc,
                 int* out) {
    for (int i = 0; i < N; ++i) {
        /* Create varying patterns for integer arrays */
        a[i] = i;
        b[i] = N/2 - i % 100;  /* Creates mix of > and < conditions */
        c[i] = i * 2;
        d[i] = i * 3;
        out[i] = 0;
        
        /* Create varying patterns for float arrays */
        fa[i] = (float)i * 1.5f;
        fb[i] = (float)(N/2) + (i % 50) * 0.5f;
        fc[i] = 0.0f;
    }
}

/* Verify results to prevent optimization */
int verify_results(int* c, float* fc, int* out, int sum_ge) {
    int checksum = 0;
    float fchecksum = 0.0f;
    
    for (int i = 0; i < N; ++i) {
        checksum += c[i] + out[i];
        fchecksum += fc[i];
    }
    
    checksum += sum_ge;
    return checksum + (int)fchecksum;
}

int main() {
    /* Allocate aligned arrays for better vectorization */
    int* a = (int*)aligned_alloc(N * sizeof(int));
    int* b = (int*)aligned_alloc(N * sizeof(int));
    int* c = (int*)aligned_alloc(N * sizeof(int));
    int* d = (int*)aligned_alloc(N * sizeof(int));
    int* out = (int*)aligned_alloc(N * sizeof(int));
    float* fa = (float*)aligned_alloc(N * sizeof(float));
    float* fb = (float*)aligned_alloc(N * sizeof(float));
    float* fc = (float*)aligned_alloc(N * sizeof(float));
    
    if (!a || !b || !c || !d || !out || !fa || !fb || !fc) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with data that will trigger all comparison results */
    init_arrays(a, b, c, d, fa, fb, fc, out);
    
    /* Execute all test functions to trigger different comparison operators */
    test_gt_expr(a, b, c, d);
    
    int sum_ge = test_ge_expr(a, b, c);
    
    test_lt_expr(fa, fb, fc);
    
    test_le_expr(a, b, c, d, out);
    
    /* Additional test with mixed comparisons */
    test_mixed_comparisons(a, b, c, fa, fb, fc);
    
    /* Verify results to prevent dead code elimination */
    int final_checksum = verify_results(c, fc, out, sum_ge);
    
    printf("Test completed. Checksum: %d\n", final_checksum);
    
    /* Clean up */
    free(a); free(b); free(c); free(d); free(out);
    free(fa); free(fb); free(fc);
    
    return 0;
}

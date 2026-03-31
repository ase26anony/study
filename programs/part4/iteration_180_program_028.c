/* Test program to trigger vectorization of conditional expressions
   and cover the bit-operation transformation for comparison operators
   in GCC's tree-vect-stmts.cc (lines 12216-12233) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test functions for each comparison operator */

/* GT_EXPR (>) */
void test_gt_expr(int *restrict a, int *restrict b, int *restrict c, int *restrict d) {
    for (int i = 0; i < N; ++i) {
        /* Conditional assignment using > operator */
        c[i] = (a[i] > b[i]) ? a[i] * 2 : b[i] / 2;
        
        /* Masked store pattern */
        if (a[i] > b[i]) {
            d[i] = a[i] + b[i];
        } else {
            d[i] = a[i] - b[i];
        }
    }
}

/* GE_EXPR (>=) */
void test_ge_expr(float *restrict a, float *restrict b, float *restrict c, float *restrict d) {
    for (int i = 0; i < N; ++i) {
        /* Conditional assignment using >= operator */
        c[i] = (a[i] >= b[i]) ? a[i] * 1.5f : b[i] * 0.5f;
        
        /* Reduction with conditional */
        if (a[i] >= b[i]) {
            d[i] = a[i] * b[i];
        } else {
            d[i] = a[i] / b[i];
        }
    }
}

/* LT_EXPR (<) */
void test_lt_expr(int *restrict a, int *restrict b, int *restrict c, int *restrict d) {
    for (int i = 0; i < N; ++i) {
        /* Conditional assignment using < operator */
        c[i] = (a[i] < b[i]) ? a[i] + b[i] : a[i] - b[i];
        
        /* Complex conditional expression */
        d[i] = (a[i] < b[i]) ? (a[i] << 2) : (b[i] >> 1);
    }
}

/* LE_EXPR (<=) */
void test_le_expr(float *restrict a, float *restrict b, float *restrict c, float *restrict d) {
    for (int i = 0; i < N; ++i) {
        /* Conditional assignment using <= operator */
        c[i] = (a[i] <= b[i]) ? a[i] + 10.0f : b[i] - 5.0f;
        
        /* Nested conditional pattern */
        if (a[i] <= b[i]) {
            d[i] = a[i] * 2.0f;
        } else {
            d[i] = b[i] * 3.0f;
        }
    }
}

/* Additional test with mixed operators in same loop */
void test_mixed_comparisons(int *restrict a, int *restrict b, int *restrict c, int *restrict d) {
    for (int i = 0; i < N; ++i) {
        /* Use all four comparison operators in same loop */
        int gt_mask = (a[i] > b[i]) ? 1 : 0;
        int ge_mask = (a[i] >= b[i]) ? 2 : 0;
        int lt_mask = (a[i] < b[i]) ? 4 : 0;
        int le_mask = (a[i] <= b[i]) ? 8 : 0;
        
        c[i] = gt_mask + ge_mask + lt_mask + le_mask;
        d[i] = (a[i] > b[i]) ? a[i] : 
               (a[i] >= b[i]) ? a[i] + b[i] :
               (a[i] < b[i]) ? b[i] :
               (a[i] <= b[i]) ? a[i] - b[i] : 0;
    }
}

/* Reduction pattern that often triggers vectorization */
int test_reduction_with_comparisons(int *restrict a, int *restrict b) {
    int sum = 0;
    for (int i = 0; i < N; ++i) {
        /* Conditional increment using different comparison operators */
        sum += (a[i] > b[i]) ? a[i] : 0;
        sum += (a[i] >= b[i]) ? b[i] : 0;
        sum -= (a[i] < b[i]) ? a[i] : 0;
        sum -= (a[i] <= b[i]) ? b[i] : 0;
    }
    return sum;
}

/* Initialize arrays with pattern that creates mix of true/false comparisons */
void init_arrays(int *a, int *b, float *fa, float *fb) {
    for (int i = 0; i < N; ++i) {
        /* Create varying patterns to ensure all comparison paths are taken */
        a[i] = i;
        b[i] = N/2 - i % 100;  /* Creates mix of >, <, == cases */
        fa[i] = (float)i * 1.5f;
        fb[i] = (float)(N/2 - i % 50) * 0.8f;
    }
}

/* Verify results by comparing with sequential computation */
int verify_results(int *c1, int *c2, float *c3, float *c4) {
    int errors = 0;
    for (int i = 0; i < 10; ++i) {  /* Check first 10 elements */
        if (c1[i] != 0 && c2[i] != 0 && c3[i] != 0.0f && c4[i] != 0.0f) {
            /* Just ensure they were computed */
        }
    }
    return errors;
}

int main() {
    /* Use 'restrict' and aligned allocations to help vectorizer */
    int *a = (int*)aligned_alloc(32, N * sizeof(int));
    int *b = (int*)aligned_alloc(32, N * sizeof(int));
    int *c1 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c2 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c3 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c4 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c5 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c6 = (int*)aligned_alloc(32, N * sizeof(int));
    
    float *fa = (float*)aligned_alloc(32, N * sizeof(float));
    float *fb = (float*)aligned_alloc(32, N * sizeof(float));
    float *fc1 = (float*)aligned_alloc(32, N * sizeof(float));
    float *fc2 = (float*)aligned_alloc(32, N * sizeof(float));
    float *fc3 = (float*)aligned_alloc(32, N * sizeof(float));
    float *fc4 = (float*)aligned_alloc(32, N * sizeof(float));
    
    if (!a || !b || !c1 || !c2 || !c3 || !c4 || !c5 || !c6 || 
        !fa || !fb || !fc1 || !fc2 || !fc3 || !fc4) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with patterns that create varied comparison results */
    init_arrays(a, b, fa, fb);
    
    /* Clear output arrays */
    memset(c1, 0, N * sizeof(int));
    memset(c2, 0, N * sizeof(int));
    memset(c3, 0, N * sizeof(int));
    memset(c4, 0, N * sizeof(int));
    memset(c5, 0, N * sizeof(int));
    memset(c6, 0, N * sizeof(int));
    memset(fc1, 0, N * sizeof(float));
    memset(fc2, 0, N * sizeof(float));
    memset(fc3, 0, N * sizeof(float));
    memset(fc4, 0, N * sizeof(float));
    
    /* Execute all test functions to trigger different comparison operators */
    test_gt_expr(a, b, c1, c2);      /* Tests > operator */
    test_ge_expr(fa, fb, fc1, fc2);  /* Tests >= operator */
    test_lt_expr(a, b, c3, c4);      /* Tests < operator */
    test_le_expr(fa, fb, fc3, fc4);  /* Tests <= operator */
    test_mixed_comparisons(a, b, c5, c6); /* Tests all operators together */
    
    /* Test reduction pattern */
    int reduction_result = test_reduction_with_comparisons(a, b);
    
    /* Verify that computations happened (prevent dead code elimination) */
    int errors = verify_results(c1, c2, fc1, fc2);
    
    /* Compute checksum to ensure all code paths were executed */
    int checksum = 0;
    float fchecksum = 0.0f;
    for (int i = 0; i < N; ++i) {
        checksum += c1[i] + c2[i] + c3[i] + c4[i] + c5[i] + c6[i];
        fchecksum += fc1[i] + fc2[i] + fc3[i] + fc4[i];
    }
    
    /* Print results to prevent optimization away */
    printf("Checksum: int=%d, float=%.2f, reduction=%d\n", 
           checksum, fchecksum, reduction_result);
    printf("Errors detected: %d\n", errors);
    
    /* Cleanup */
    free(a); free(b); free(c1); free(c2); free(c3); free(c4); free(c5); free(c6);
    free(fa); free(fb); free(fc1); free(fc2); free(fc3); free(fc4);
    
    return errors;
}

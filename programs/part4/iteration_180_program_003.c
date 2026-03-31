/* test_vector_cond_bitops.c
 * Designed to trigger vectorizer transformation of comparison operations
 * to bit operations (BIT_NOT_EXPR, BIT_AND_EXPR, BIT_IOR_EXPR) in GCC's
 * tree-vect-stmts.cc lines 12216-12233
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGN 32

/* Aligned allocations for better vectorization */
static float *aligned_alloc(size_t size) {
    void *ptr;
    if (posix_memalign(&ptr, ALIGN, size) != 0) return NULL;
    return (float*)ptr;
}

/* GT_EXPR (>) test - should trigger BIT_NOT_EXPR + BIT_AND_EXPR */
void test_gt_expr(float *restrict a, float *restrict b, float *restrict c, float *restrict d) {
    for (int i = 0; i < N; ++i) {
        /* Conditional assignment using > operator */
        c[i] = (a[i] > b[i]) ? a[i] * b[i] : d[i];
    }
}

/* GE_EXPR (>=) test - should trigger BIT_NOT_EXPR + BIT_IOR_EXPR */
void test_ge_expr(float *restrict a, float *restrict b, float *restrict c, float *restrict d) {
    for (int i = 0; i < N; ++i) {
        /* Conditional assignment using >= operator */
        c[i] = (a[i] >= b[i]) ? a[i] + b[i] : d[i] - a[i];
    }
}

/* LT_EXPR (<) test - should trigger BIT_NOT_EXPR + BIT_AND_EXPR with operand swap */
void test_lt_expr(float *restrict a, float *restrict b, float *restrict c, float *restrict d) {
    for (int i = 0; i < N; ++i) {
        /* Conditional assignment using < operator */
        c[i] = (a[i] < b[i]) ? a[i] - b[i] : d[i] * 2.0f;
    }
}

/* LE_EXPR (<=) test - should trigger BIT_NOT_EXPR + BIT_IOR_EXPR with operand swap */
void test_le_expr(float *restrict a, float *restrict b, float *restrict c, float *restrict d) {
    for (int i = 0; i < N; ++i) {
        /* Conditional assignment using <= operator */
        c[i] = (a[i] <= b[i]) ? a[i] / (b[i] + 1.0f) : d[i] + 1.0f;
    }
}

/* Additional test with reduction pattern for GE_EXPR */
float test_ge_reduction(float *restrict a, float *restrict b) {
    float sum = 0.0f;
    for (int i = 0; i < N; ++i) {
        /* Conditional reduction using >= operator */
        sum += (a[i] >= b[i]) ? a[i] : 0.0f;
    }
    return sum;
}

/* Additional test with masked store for LT_EXPR */
void test_lt_masked_store(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; ++i) {
        /* Masked store using < operator */
        if (a[i] < b[i]) {
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i];
        }
    }
}

/* Verification function */
int verify_results(float *c1, float *c2, float *c3, float *c4) {
    float checksum1 = 0.0f, checksum2 = 0.0f, checksum3 = 0.0f, checksum4 = 0.0f;
    
    for (int i = 0; i < N; ++i) {
        checksum1 += c1[i];
        checksum2 += c2[i];
        checksum3 += c3[i];
        checksum4 += c4[i];
    }
    
    printf("Checksums: GT=%.2f, GE=%.2f, LT=%.2f, LE=%.2f\n", 
           checksum1, checksum2, checksum3, checksum4);
    
    /* Simple validation - just ensure they're not all zero */
    return !(checksum1 == 0.0f && checksum2 == 0.0f && 
             checksum3 == 0.0f && checksum4 == 0.0f);
}

int main() {
    /* Allocate aligned arrays for better vectorization */
    float *a = aligned_alloc(N * sizeof(float));
    float *b = aligned_alloc(N * sizeof(float));
    float *c1 = aligned_alloc(N * sizeof(float));
    float *c2 = aligned_alloc(N * sizeof(float));
    float *c3 = aligned_alloc(N * sizeof(float));
    float *c4 = aligned_alloc(N * sizeof(float));
    float *d = aligned_alloc(N * sizeof(float));
    
    if (!a || !b || !c1 || !c2 || !c3 || !c4 || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with varying data to create mix of true/false comparisons */
    for (int i = 0; i < N; ++i) {
        a[i] = (float)(i - N/2);          /* Range: -512 to 511 */
        b[i] = (float)(i % 100);          /* Range: 0 to 99 */
        d[i] = (float)(i * 0.5f);         /* Different pattern */
        c1[i] = c2[i] = c3[i] = c4[i] = 0.0f;
    }
    
    /* Test all four comparison operators */
    test_gt_expr(a, b, c1, d);    /* > operator */
    test_ge_expr(a, b, c2, d);    /* >= operator */
    test_lt_expr(a, b, c3, d);    /* < operator */
    test_le_expr(a, b, c4, d);    /* <= operator */
    
    /* Additional patterns that might trigger the transformation */
    float reduction_sum = test_ge_reduction(a, b);
    test_lt_masked_store(a, b, c1);  /* Reuse c1 array */
    
    printf("Reduction sum from GE pattern: %.2f\n", reduction_sum);
    
    /* Verify results to prevent dead code elimination */
    int valid = verify_results(c1, c2, c3, c4);
    
    /* Cleanup */
    free(a); free(b); free(c1); free(c2); free(c3); free(c4); free(d);
    
    return valid ? 0 : 1;
}

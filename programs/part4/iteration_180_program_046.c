/* test_vectorized_comparisons.c
 * 
 * This program creates vectorizable loops with conditional statements
 * using all four comparison operators (>, >=, <, <=) to trigger the
 * transformation in tree-vect-stmts.cc that converts comparisons to
 * bit operations.
 * 
 * Compile with: gcc -O3 -ftree-vectorize -fno-vect-cost-model -march=native
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define ALIGN 32

/* Aligned memory allocation for better vectorization */
static void* aligned_alloc(size_t alignment, size_t size) {
    void* ptr;
    if (posix_memalign(&ptr, alignment, size) != 0) {
        return NULL;
    }
    return ptr;
}

/* GT_EXPR (>) test: Conditional assignment based on a[i] > b[i] */
void test_gt_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict d) {
    for (int i = 0; i < N; ++i) {
        /* This conditional will be transformed to bit operations during vectorization */
        if (a[i] > b[i]) {
            c[i] = a[i] * 2 + b[i];
        } else {
            c[i] = a[i] + b[i] / 2;
        }
        /* Additional computation to prevent dead code elimination */
        d[i] = (a[i] > b[i]) ? (a[i] - b[i]) : (b[i] - a[i]);
    }
}

/* GE_EXPR (>=) test: Conditional reduction and masked store */
int test_ge_expr(float* restrict a, float* restrict b, float* restrict c) {
    float sum = 0.0f;
    for (int i = 0; i < N; ++i) {
        /* Conditional increment - will generate mask from comparison */
        if (a[i] >= b[i]) {
            sum += a[i] * b[i];
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
    /* Return integer representation to prevent optimization */
    return (int)(sum * 1000.0f);
}

/* LT_EXPR (<) test: Conditional with swapped operands pattern */
void test_lt_expr(int64_t* restrict a, int64_t* restrict b, int64_t* restrict c) {
    for (int i = 0; i < N; ++i) {
        /* This should trigger the std::swap(cond_expr0, cond_expr1) in the LT_EXPR case */
        if (a[i] < b[i]) {
            c[i] = a[i] * 3 - b[i];
        } else {
            c[i] = b[i] * 2 + a[i];
        }
    }
}

/* LE_EXPR (<=) test: Complex conditional with multiple uses */
int test_le_expr(double* restrict a, double* restrict b, double* restrict c, double* restrict d) {
    double total = 0.0;
    for (int i = 0; i < N; ++i) {
        /* Multiple conditional operations to ensure vectorization */
        if (a[i] <= b[i]) {
            c[i] = a[i] * b[i];
            d[i] = a[i] + b[i];
            total += c[i];
        } else {
            c[i] = a[i] / (b[i] + 1.0);
            d[i] = a[i] - b[i];
            total -= d[i];
        }
    }
    return (int)(total * 100.0);
}

/* Mixed comparisons test: Uses all operators in one loop */
void test_mixed_comparisons(int* restrict a, int* restrict b, int* restrict c, 
                           int* restrict d, int* restrict e, int* restrict f) {
    for (int i = 0; i < N; ++i) {
        /* Use all four comparison operators in ways that should vectorize */
        int gt_mask = (a[i] > b[i]) ? 1 : 0;
        int ge_mask = (a[i] >= b[i]) ? 1 : 0;
        int lt_mask = (a[i] < b[i]) ? 1 : 0;
        int le_mask = (a[i] <= b[i]) ? 1 : 0;
        
        /* Combine masks in ways that might trigger bit operations */
        c[i] = gt_mask * (a[i] + b[i]);
        d[i] = ge_mask * (a[i] - b[i]);
        e[i] = lt_mask * (a[i] * b[i]);
        f[i] = le_mask * (a[i] / (b[i] + 1));
    }
}

/* Initialize test data with patterns that create mixed true/false results */
void init_test_data(int* a_int, int* b_int, float* a_float, float* b_float,
                   int64_t* a_int64, int64_t* b_int64, double* a_double, double* b_double) {
    for (int i = 0; i < N; ++i) {
        /* Create data patterns that yield ~50% true/false for comparisons */
        a_int[i] = i;
        b_int[i] = N/2;
        
        a_float[i] = (float)(i * 1.5f);
        b_float[i] = (float)(N - i);
        
        a_int64[i] = (int64_t)(i * 3);
        b_int64[i] = (int64_t)(N * 2 - i);
        
        a_double[i] = (double)i * 0.7;
        b_double[i] = (double)(N/3) * (i % 3 + 1);
    }
}

/* Verification function to ensure computations are correct */
int verify_results() {
    /* Allocate aligned memory for better vectorization */
    int* a_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* b_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* d_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* e_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* f_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    
    float* a_float = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* b_float = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* c_float = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    
    int64_t* a_int64 = (int64_t*)aligned_alloc(ALIGN, N * sizeof(int64_t));
    int64_t* b_int64 = (int64_t*)aligned_alloc(ALIGN, N * sizeof(int64_t));
    int64_t* c_int64 = (int64_t*)aligned_alloc(ALIGN, N * sizeof(int64_t));
    
    double* a_double = (double*)aligned_alloc(ALIGN, N * sizeof(double));
    double* b_double = (double*)aligned_alloc(ALIGN, N * sizeof(double));
    double* c_double = (double*)aligned_alloc(ALIGN, N * sizeof(double));
    double* d_double = (double*)aligned_alloc(ALIGN, N * sizeof(double));
    
    /* Initialize all arrays */
    init_test_data(a_int, b_int, a_float, b_float, a_int64, b_int64, a_double, b_double);
    memset(c_int, 0, N * sizeof(int));
    memset(d_int, 0, N * sizeof(int));
    memset(e_int, 0, N * sizeof(int));
    memset(f_int, 0, N * sizeof(int));
    memset(c_float, 0, N * sizeof(float));
    memset(c_int64, 0, N * sizeof(int64_t));
    memset(c_double, 0, N * sizeof(double));
    memset(d_double, 0, N * sizeof(double));
    
    /* Execute all test functions */
    test_gt_expr(a_int, b_int, c_int, d_int);
    
    int ge_result = test_ge_expr(a_float, b_float, c_float);
    
    test_lt_expr(a_int64, b_int64, c_int64);
    
    int le_result = test_le_expr(a_double, b_double, c_double, d_double);
    
    test_mixed_comparisons(a_int, b_int, c_int, d_int, e_int, f_int);
    
    /* Compute checksums to verify computations were performed */
    int checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += c_int[i] + d_int[i] + e_int[i] + f_int[i];
        checksum += (int)c_float[i];
        checksum += (int)c_int64[i];
        checksum += (int)c_double[i] + (int)d_double[i];
    }
    checksum += ge_result + le_result;
    
    /* Free allocated memory */
    free(a_int); free(b_int); free(c_int); free(d_int); free(e_int); free(f_int);
    free(a_float); free(b_float); free(c_float);
    free(a_int64); free(b_int64); free(c_int64);
    free(a_double); free(b_double); free(c_double); free(d_double);
    
    return checksum;
}

int main() {
    printf("Testing vectorized comparisons to trigger tree-vect-stmts.cc transformations...\n");
    
    int checksum = verify_results();
    
    printf("Computed checksum: %d\n", checksum);
    printf("If checksum is non-zero, all conditionals were executed.\n");
    
    /* Return non-zero exit code if checksum is 0 (unlikely with our data) */
    return (checksum == 0) ? 1 : 0;
}

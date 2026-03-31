/* test_vector_cond_bitops.c
 * 
 * This program is designed to trigger the transformation of comparison
 * operations (GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR) to bit operations
 * (BIT_NOT_EXPR, BIT_AND_EXPR, BIT_IOR_EXPR) during auto-vectorization.
 * 
 * Compile with: gcc -O3 -ftree-vectorize -fno-vect-cost-model -march=native -fno-tree-slp-vectorize test_vector_cond_bitops.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGN 32

/* Aligned allocations to help vectorization */
static void* aligned_alloc(size_t alignment, size_t size) {
    void* ptr;
    if (posix_memalign(&ptr, alignment, size) != 0) {
        return NULL;
    }
    return ptr;
}

/* GT_EXPR (>) test with conditional assignment pattern */
void test_gt_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict d) {
    for (int i = 0; i < N; ++i) {
        /* This conditional assignment should be transformed to use bit operations */
        d[i] = (a[i] > b[i]) ? c[i] * 2 : c[i] / 2;
    }
}

/* GE_EXPR (>=) test with masked store pattern */
void test_ge_expr(float* restrict a, float* restrict b, float* restrict c) {
    for (int i = 0; i < N; ++i) {
        /* Conditional store that may use mask-based vectorization */
        if (a[i] >= b[i]) {
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] + b[i];
        }
    }
}

/* LT_EXPR (<) test with conditional reduction pattern */
int test_lt_expr(int* restrict a, int* restrict b) {
    int sum = 0;
    for (int i = 0; i < N; ++i) {
        /* Conditional reduction that creates mask from comparison */
        sum += (a[i] < b[i]) ? a[i] : b[i];
    }
    return sum;
}

/* LE_EXPR (<=) test with blend/select pattern */
void test_le_expr(double* restrict a, double* restrict b, double* restrict c, double* restrict d) {
    for (int i = 0; i < N; ++i) {
        /* Complex conditional that may trigger bit operation transformation */
        d[i] = (a[i] <= b[i]) ? (c[i] * 1.5) : (c[i] * 0.5);
    }
}

/* Additional test: Mixed comparisons in same loop to ensure all paths are exercised */
void test_mixed_comparisons(int* restrict a, int* restrict b, int* restrict out1, 
                           int* restrict out2, int* restrict out3, int* restrict out4) {
    for (int i = 0; i < N; ++i) {
        /* All four comparison types in one loop */
        out1[i] = (a[i] > b[i]) ? a[i] : b[i];      // GT
        out2[i] = (a[i] >= b[i]) ? a[i] + b[i] : a[i] - b[i];  // GE
        out3[i] = (a[i] < b[i]) ? a[i] * b[i] : a[i];          // LT
        out4[i] = (a[i] <= b[i]) ? b[i] : a[i] * 2;            // LE
    }
}

/* Reference implementation for validation */
void compute_reference(int* a, int* b, int* ref_gt, int* ref_ge, 
                      int* ref_lt, int* ref_le, double* ref_le_dbl) {
    for (int i = 0; i < N; ++i) {
        ref_gt[i] = (a[i] > b[i]) ? a[i] * 2 : b[i] * 2;
        ref_ge[i] = (a[i] >= b[i]) ? a[i] + b[i] : a[i] - b[i];
        ref_lt[i] = (a[i] < b[i]) ? a[i] : b[i];
        ref_le[i] = (a[i] <= b[i]) ? a[i] * 3 : b[i] * 3;
        ref_le_dbl[i] = (a[i] <= b[i]) ? a[i] * 1.5 : b[i] * 1.5;
    }
}

int main() {
    /* Allocate aligned memory for better vectorization */
    int* a_int = aligned_alloc(ALIGN, N * sizeof(int));
    int* b_int = aligned_alloc(ALIGN, N * sizeof(int));
    int* c_int = aligned_alloc(ALIGN, N * sizeof(int));
    int* d_int = aligned_alloc(ALIGN, N * sizeof(int));
    int* out1 = aligned_alloc(ALIGN, N * sizeof(int));
    int* out2 = aligned_alloc(ALIGN, N * sizeof(int));
    int* out3 = aligned_alloc(ALIGN, N * sizeof(int));
    int* out4 = aligned_alloc(ALIGN, N * sizeof(int));
    
    float* a_float = aligned_alloc(ALIGN, N * sizeof(float));
    float* b_float = aligned_alloc(ALIGN, N * sizeof(float));
    float* c_float = aligned_alloc(ALIGN, N * sizeof(float));
    
    double* a_double = aligned_alloc(ALIGN, N * sizeof(double));
    double* b_double = aligned_alloc(ALIGN, N * sizeof(double));
    double* c_double = aligned_alloc(ALIGN, N * sizeof(double));
    double* d_double = aligned_alloc(ALIGN, N * sizeof(double));
    
    /* Initialize data with patterns that ensure all comparison results (true/false) */
    for (int i = 0; i < N; ++i) {
        /* Create varying patterns to exercise both branches of conditionals */
        a_int[i] = i;
        b_int[i] = N/2 - i % 100;  /* Creates mix of >, <, == cases */
        c_int[i] = i * 3 + 7;
        
        a_float[i] = (float)i * 1.5f;
        b_float[i] = (float)(N/2) * (i % 3 == 0 ? 0.5f : 2.0f);
        
        a_double[i] = (double)i * 0.7;
        b_double[i] = (double)(N/4) + (i % 50);
        c_double[i] = (double)i * 1.1;
    }
    
    /* Execute all test functions */
    test_gt_expr(a_int, b_int, c_int, d_int);
    
    test_ge_expr(a_float, b_float, c_float);
    
    int sum_lt = test_lt_expr(a_int, b_int);
    
    test_le_expr(a_double, b_double, c_double, d_double);
    
    test_mixed_comparisons(a_int, b_int, out1, out2, out3, out4);
    
    /* Compute checksums to prevent dead code elimination */
    int checksum_int = 0;
    float checksum_float = 0.0f;
    double checksum_double = 0.0;
    
    for (int i = 0; i < N; ++i) {
        checksum_int += d_int[i] + out1[i] + out2[i] + out3[i] + out4[i];
        checksum_float += c_float[i];
        checksum_double += d_double[i];
    }
    
    checksum_int += sum_lt;
    
    /* Print results to ensure code isn't optimized away */
    printf("Checksums (to prevent optimization):\n");
    printf("  Integer: %d\n", checksum_int);
    printf("  Float: %f\n", checksum_float);
    printf("  Double: %f\n", checksum_double);
    
    /* Free allocated memory */
    free(a_int); free(b_int); free(c_int); free(d_int);
    free(out1); free(out2); free(out3); free(out4);
    free(a_float); free(b_float); free(c_float);
    free(a_double); free(b_double); free(c_double); free(d_double);
    
    return 0;
}

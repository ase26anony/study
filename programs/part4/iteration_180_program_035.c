/* test_vectorization_conditions.c
 * 
 * This program is designed to trigger auto-vectorization of loops containing
 * conditional statements with comparison operators (>, >=, <, <=) to cover
 * the transformation to bit operations in GCC's tree-vect-stmts.cc.
 * 
 * Compile with: gcc -O3 -ftree-vectorize -fno-vect-cost-model -march=native -fopt-info-vec
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

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

/* Initialize test arrays with varying data to create mixed true/false conditions */
static void init_arrays(int* a, int* b, int* c, int* d, float* fa, float* fb, float* fc) {
    for (int i = 0; i < N; i++) {
        a[i] = i;                     /* 0, 1, 2, ..., N-1 */
        b[i] = N/2 - i;               /* N/2, N/2-1, ..., -N/2+1 */
        c[i] = 0;                     /* Output array */
        d[i] = (i % 3) * 10;          /* Pattern: 0, 10, 20, 0, 10, 20, ... */
        
        fa[i] = (float)i * 0.5f;      /* 0.0, 0.5, 1.0, ... */
        fb[i] = (float)(N/4) - (float)i * 0.25f; /* Mixed values */
        fc[i] = 0.0f;
    }
}

/* GT_EXPR (>) - Conditional assignment based on greater-than comparison */
static void test_gt_expr(int* a, int* b, int* c) {
    /* Pattern: if (a[i] > b[i]) c[i] = a[i] * b[i] else c[i] = a[i] + b[i] */
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] + b[i];
        }
    }
}

/* GE_EXPR (>=) - Conditional reduction with greater-or-equal comparison */
static int test_ge_expr(int* a, int* b, int* d) {
    /* Pattern: sum += (a[i] >= b[i]) ? d[i] : 0 */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += d[i];
        }
    }
    return sum;
}

/* LT_EXPR (<) - Masked store with less-than comparison */
static void test_lt_expr(float* fa, float* fb, float* fc) {
    /* Pattern: if (fa[i] < fb[i]) fc[i] = fa[i] * fb[i] */
    for (int i = 0; i < N; i++) {
        if (fa[i] < fb[i]) {
            fc[i] = fa[i] * fb[i];
        } else {
            fc[i] = fa[i];
        }
    }
}

/* LE_EXPR (<=) - Complex conditional with less-or-equal comparison */
static void test_le_expr(int* a, int* b, int* c, int* d) {
    /* Pattern: c[i] = (a[i] <= b[i]) ? (a[i] & d[i]) : (b[i] | d[i]) */
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            c[i] = a[i] & d[i];
        } else {
            c[i] = b[i] | d[i];
        }
    }
}

/* Additional test: Mixed comparisons in same loop to force multiple transformations */
static void test_mixed_comparisons(int* a, int* b, int* c, int* d) {
    /* This loop contains multiple comparison types that might be vectorized together */
    for (int i = 0; i < N; i++) {
        /* GT_EXPR */
        if (a[i] > b[i]) {
            c[i] += a[i];
        }
        
        /* GE_EXPR */
        if (a[i] >= N/4) {
            d[i] *= 2;
        }
        
        /* LT_EXPR */
        if (b[i] < 0) {
            c[i] -= b[i];
        }
        
        /* LE_EXPR */
        if (a[i] <= b[i]) {
            d[i] = d[i] >> 1;
        }
    }
}

/* Floating-point comparisons which also trigger the transformations */
static void test_float_comparisons(float* fa, float* fb, float* fc) {
    /* Test all four comparison operators with floats */
    for (int i = 0; i < N; i++) {
        /* GT_EXPR */
        if (fa[i] > fb[i]) {
            fc[i] = fa[i] - fb[i];
        }
        
        /* GE_EXPR */
        if (fa[i] >= fb[i] * 0.5f) {
            fc[i] += fa[i];
        }
        
        /* LT_EXPR */
        if (fa[i] < fb[i] + 10.0f) {
            fc[i] *= 1.5f;
        }
        
        /* LE_EXPR */
        if (fa[i] <= fb[i] - 5.0f) {
            fc[i] = sqrtf(fc[i] + 1.0f);
        }
    }
}

/* Verification function to ensure computations are correct and not optimized away */
static int verify_results(int* a, int* b, int* c, int* d, float* fa, float* fb, float* fc) {
    int checksum = 0;
    
    /* Verify GT_EXPR results */
    for (int i = 0; i < N; i++) {
        int expected = (a[i] > b[i]) ? a[i] * b[i] : a[i] + b[i];
        checksum += (c[i] == expected) ? 1 : 0;
    }
    
    /* Verify GE_EXPR results were computed (check that sum is non-zero) */
    int ge_sum = test_ge_expr(a, b, d);
    checksum += (ge_sum != 0) ? N : 0;
    
    /* Verify LT_EXPR results */
    for (int i = 0; i < N; i++) {
        float expected = (fa[i] < fb[i]) ? fa[i] * fb[i] : fa[i];
        checksum += (fabsf(fc[i] - expected) < 0.001f) ? 1 : 0;
    }
    
    /* Verify LE_EXPR results */
    for (int i = 0; i < N; i++) {
        int expected = (a[i] <= b[i]) ? (a[i] & d[i]) : (b[i] | d[i]);
        checksum += (c[i] == expected) ? 1 : 0;
    }
    
    return checksum;
}

int main() {
    /* Allocate aligned memory for better vectorization */
    int* a = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* b = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* d = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    float* fa = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* fb = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* fc = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    
    if (!a || !b || !c || !d || !fa || !fb || !fc) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize test data */
    init_arrays(a, b, c, d, fa, fb, fc);
    
    printf("Testing auto-vectorization of conditional expressions...\n");
    
    /* Execute tests for each comparison type */
    test_gt_expr(a, b, c);           /* Triggers GT_EXPR case */
    
    int ge_result = test_ge_expr(a, b, d);  /* Triggers GE_EXPR case */
    printf("GE_EXPR reduction result: %d\n", ge_result);
    
    test_lt_expr(fa, fb, fc);        /* Triggers LT_EXPR case */
    
    test_le_expr(a, b, c, d);        /* Triggers LE_EXPR case */
    
    /* Additional tests with mixed comparisons */
    test_mixed_comparisons(a, b, c, d);
    test_float_comparisons(fa, fb, fc);
    
    /* Verify results to ensure computations aren't optimized away */
    int verification = verify_results(a, b, c, d, fa, fb, fc);
    printf("Verification checksum: %d (expected: %d)\n", verification, N * 4 + N);
    
    /* Clean up */
    free(a);
    free(b);
    free(c);
    free(d);
    free(fa);
    free(fb);
    free(fc);
    
    return 0;
}

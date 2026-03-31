/* test_vector_cond_bitops.c
 * 
 * This program creates vectorizable loops with conditional operations
 * using all four comparison operators (>, >=, <, <=) to trigger the
 * transformation in tree-vect-stmts.cc that converts comparisons to
 * bit operations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGN 32

/* Aligned allocation to help vectorization */
static void* aligned_alloc(size_t alignment, size_t size) {
    void* ptr;
    if (posix_memalign(&ptr, alignment, size) != 0) {
        return NULL;
    }
    return ptr;
}

/* GT_EXPR (>) test: conditional assignment based on a[i] > b[i] */
void test_gt_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict d) {
    for (int i = 0; i < N; ++i) {
        /* This conditional pattern should trigger GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR */
        if (a[i] > b[i]) {
            c[i] = a[i] * 2 + b[i];
        } else {
            c[i] = a[i] + b[i] / 2;
        }
        
        /* Additional pattern: conditional increment with mask */
        d[i] += (a[i] > b[i]) ? 3 : 1;
    }
}

/* GE_EXPR (>=) test: conditional reduction and assignment */
int test_ge_expr(int* restrict a, int* restrict b, int* restrict c) {
    int sum = 0;
    for (int i = 0; i < N; ++i) {
        /* This should trigger GE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR */
        if (a[i] >= b[i]) {
            c[i] = a[i] - b[i];
            sum += c[i];
        } else {
            c[i] = b[i] - a[i];
            sum -= c[i];
        }
    }
    return sum;
}

/* LT_EXPR (<) test: conditional assignment with swapped logic */
void test_lt_expr(float* restrict a, float* restrict b, float* restrict c) {
    for (int i = 0; i < N; ++i) {
        /* This should trigger LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR with swap */
        if (a[i] < b[i]) {
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] + b[i];
        }
    }
}

/* LE_EXPR (<=) test: multiple conditional operations */
float test_le_expr(float* restrict a, float* restrict b, float* restrict c, float* restrict d) {
    float max_val = 0.0f;
    for (int i = 0; i < N; ++i) {
        /* This should trigger LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
        if (a[i] <= b[i]) {
            c[i] = a[i];
            d[i] = b[i] - a[i];
        } else {
            c[i] = b[i];
            d[i] = a[i] - b[i];
        }
        
        /* Conditional max reduction */
        if (c[i] <= d[i]) {
            if (d[i] > max_val) max_val = d[i];
        } else {
            if (c[i] > max_val) max_val = c[i];
        }
    }
    return max_val;
}

/* Mixed comparisons in same loop to ensure all paths are exercised */
void test_mixed_comparisons(int* restrict a, int* restrict b, 
                           int* restrict c, int* restrict d) {
    for (int i = 0; i < N; ++i) {
        /* Mix all four comparison types in one loop */
        int temp = 0;
        
        if (a[i] > b[i])  temp += 1;   /* GT_EXPR */
        if (a[i] >= b[i]) temp += 2;   /* GE_EXPR */
        if (a[i] < b[i])  temp += 4;   /* LT_EXPR */
        if (a[i] <= b[i]) temp += 8;   /* LE_EXPR */
        
        c[i] = temp;
        d[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
}

/* Initialize arrays with pattern that creates mix of true/false comparisons */
void init_arrays(int* a, int* b, float* fa, float* fb) {
    for (int i = 0; i < N; ++i) {
        /* Create varying patterns to ensure all comparison results occur */
        a[i] = i;
        b[i] = (i % 3 == 0) ? N/2 : (i % 5 == 0) ? i*2 : i/2;
        
        fa[i] = (float)(i * 1.5f);
        fb[i] = (float)((i % 7 == 0) ? i * 2.0f : i * 0.5f);
    }
}

/* Verify results to prevent dead code elimination */
int verify_results(int* c1, int* c2, float* c3, float* c4, int* c5, int* c6) {
    int checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += c1[i] + c2[i] + (int)c3[i] + (int)c4[i] + c5[i] + c6[i];
    }
    return checksum;
}

int main() {
    /* Allocate aligned arrays for better vectorization */
    int* a_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* b_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c1 = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c2 = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c3 = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c4 = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* d1 = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* d2 = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    
    float* a_float = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* b_float = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* c_float1 = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* c_float2 = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* d_float = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    
    if (!a_int || !b_int || !c1 || !c2 || !c3 || !c4 || !d1 || !d2 ||
        !a_float || !b_float || !c_float1 || !c_float2 || !d_float) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize d arrays */
    memset(d1, 0, N * sizeof(int));
    for (int i = 0; i < N; ++i) d2[i] = i;
    
    /* Initialize all arrays */
    init_arrays(a_int, b_int, a_float, b_float);
    
    /* Test each comparison type separately */
    test_gt_expr(a_int, b_int, c1, d1);
    
    int sum_ge = test_ge_expr(a_int, b_int, c2);
    
    test_lt_expr(a_float, b_float, c_float1);
    
    float max_le = test_le_expr(a_float, b_float, c_float2, d_float);
    
    test_mixed_comparisons(a_int, b_int, c3, c4);
    
    /* Verify results to ensure computations aren't optimized away */
    int checksum = verify_results(c1, c2, c_float1, c_float2, c3, c4);
    
    /* Use results to affect program output */
    printf("Results: GE sum = %d, LE max = %.2f, checksum = %d\n", 
           sum_ge, max_le, checksum);
    
    /* Free allocated memory */
    free(a_int); free(b_int); free(c1); free(c2); free(c3); free(c4);
    free(d1); free(d2); free(a_float); free(b_float);
    free(c_float1); free(c_float2); free(d_float);
    
    return 0;
}

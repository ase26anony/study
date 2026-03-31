/* test_vector_cond_bitops.c
 * 
 * This program creates vectorizable loops with conditional statements
 * using all four comparison operators (>, >=, <, <=) to trigger the
 * transformation of comparisons to bit operations in GCC's tree vectorizer.
 * 
 * Compile with: gcc -O3 -ftree-vectorize -fno-vect-cost-model -march=native test_vector_cond_bitops.c -o test_vector_cond_bitops
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define ALIGN 32

/* Aligned allocations for better vectorization */
static float* alloc_aligned(size_t size) {
    float* ptr;
    if (posix_memalign((void**)&ptr, ALIGN, size * sizeof(float)) != 0) {
        return NULL;
    }
    return ptr;
}

/* GT_EXPR (>) - Conditional assignment pattern */
void test_gt_expr(float* __restrict a, float* __restrict b, 
                  float* __restrict c, float* __restrict d, int n) {
    for (int i = 0; i < n; ++i) {
        /* This conditional assignment should be transformed to bit operations */
        c[i] = (a[i] > b[i]) ? a[i] * b[i] : d[i];
    }
}

/* GE_EXPR (>=) - Conditional reduction pattern */
float test_ge_expr(float* __restrict a, float* __restrict b, 
                   float* __restrict c, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; ++i) {
        /* Conditional increment - may use mask-based reduction */
        sum += (a[i] >= b[i]) ? c[i] : 0.0f;
    }
    return sum;
}

/* LT_EXPR (<) - Masked store pattern */
void test_lt_expr(float* __restrict a, float* __restrict b, 
                  float* __restrict c, int n) {
    for (int i = 0; i < n; ++i) {
        /* Conditional store - should generate mask */
        if (a[i] < b[i]) {
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

/* LE_EXPR (<=) - Complex conditional pattern */
void test_le_expr(float* __restrict a, float* __restrict b,
                  float* __restrict c, float* __restrict d, int n) {
    for (int i = 0; i < n; ++i) {
        /* Multiple uses of the same comparison */
        float temp = (a[i] <= b[i]) ? a[i] : b[i];
        c[i] = temp * d[i];
        d[i] = (a[i] <= b[i]) ? temp + 1.0f : temp - 1.0f;
    }
}

/* Mixed comparisons to ensure all paths are exercised */
void test_mixed_comparisons(float* __restrict a, float* __restrict b,
                           float* __restrict c, int n) {
    for (int i = 0; i < n; ++i) {
        /* Use all four comparison operators in one loop */
        if (a[i] > b[i]) {
            c[i] = a[i];
        } else if (a[i] >= b[i] + 1.0f) {
            c[i] = b[i];
        } else if (a[i] < b[i] - 1.0f) {
            c[i] = a[i] + b[i];
        } else if (a[i] <= b[i]) {
            c[i] = a[i] - b[i];
        } else {
            c[i] = 0.0f;
        }
    }
}

/* Integer version to test with different data types */
void test_int_comparisons(int* __restrict a, int* __restrict b,
                         int* __restrict c, int n) {
    for (int i = 0; i < n; ++i) {
        /* Integer comparisons should also trigger the transformation */
        c[i] = (a[i] > b[i]) ? a[i] & b[i] : a[i] | b[i];
        c[i] += (a[i] >= b[i]) ? 1 : -1;
        c[i] *= (a[i] < b[i]) ? 2 : 3;
        c[i] = (a[i] <= b[i]) ? c[i] << 1 : c[i] >> 1;
    }
}

/* Verify results against a reference implementation */
int verify_results(float* a, float* b, float* c, float* d, int n) {
    float* c_ref = alloc_aligned(n);
    float* d_ref = alloc_aligned(n);
    float sum_ref = 0.0f;
    
    if (!c_ref || !d_ref) {
        free(c_ref);
        free(d_ref);
        return 0;
    }
    
    /* Reference implementation (sequential) */
    for (int i = 0; i < n; ++i) {
        c_ref[i] = (a[i] > b[i]) ? a[i] * b[i] : d[i];
    }
    
    for (int i = 0; i < n; ++i) {
        sum_ref += (a[i] >= b[i]) ? c[i] : 0.0f;
    }
    
    for (int i = 0; i < n; ++i) {
        if (a[i] < b[i]) {
            d_ref[i] = a[i] + b[i];
        } else {
            d_ref[i] = a[i] - b[i];
        }
    }
    
    /* Compare results */
    int errors = 0;
    for (int i = 0; i < n; ++i) {
        if (c[i] != c_ref[i]) errors++;
        if (d[i] != d_ref[i]) errors++;
    }
    
    free(c_ref);
    free(d_ref);
    return errors;
}

int main() {
    /* Allocate aligned arrays for better vectorization */
    float* a = alloc_aligned(N);
    float* b = alloc_aligned(N);
    float* c = alloc_aligned(N);
    float* d = alloc_aligned(N);
    
    int* a_int = alloc_aligned(N);
    int* b_int = alloc_aligned(N);
    int* c_int = alloc_aligned(N);
    
    if (!a || !b || !c || !d || !a_int || !b_int || !c_int) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with varying data to create mix of true/false comparisons */
    for (int i = 0; i < N; ++i) {
        a[i] = (float)(i - N/2);          /* Range: -512 to 511 */
        b[i] = (float)(i % 100);          /* Range: 0 to 99 */
        c[i] = (float)(i * 0.5f);
        d[i] = (float)(i * 2.0f);
        
        a_int[i] = i - N/2;
        b_int[i] = i % 100;
        c_int[i] = 0;
    }
    
    printf("Testing GT_EXPR (>)...\n");
    test_gt_expr(a, b, c, d, N);
    
    printf("Testing GE_EXPR (>=)...\n");
    float sum = test_ge_expr(a, b, c, N);
    printf("  Sum from GE_EXPR: %f\n", sum);
    
    printf("Testing LT_EXPR (<)...\n");
    test_lt_expr(a, b, d, N);
    
    printf("Testing LE_EXPR (<=)...\n");
    test_le_expr(a, b, c, d, N);
    
    printf("Testing mixed comparisons...\n");
    test_mixed_comparisons(a, b, c, N);
    
    printf("Testing integer comparisons...\n");
    test_int_comparisons(a_int, b_int, c_int, N);
    
    /* Verify results */
    int errors = verify_results(a, b, c, d, N);
    if (errors == 0) {
        printf("All tests passed successfully!\n");
    } else {
        printf("Found %d errors in verification\n", errors);
    }
    
    /* Compute and print checksum to prevent dead code elimination */
    float checksum = 0.0f;
    int int_checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += c[i] + d[i];
        int_checksum += c_int[i];
    }
    printf("Final checksum: float=%f, int=%d\n", checksum, int_checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(a_int); free(b_int); free(c_int);
    
    return 0;
}

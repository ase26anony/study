/* test_vectorized_comparisons.c
 * 
 * This program creates vectorizable loops with >, >=, <, <= comparisons
 * to trigger the bit-operation transformation in GCC's tree-vect-stmts.cc
 * lines 12216-12233.
 *
 * Compile with: gcc -O3 -ftree-vectorize -fno-vect-cost-model -march=native -o test test_vectorized_comparisons.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define N 1024
#define ALIGN 32

/* Aligned allocations for better vectorization */
static float* alloc_aligned(size_t size) {
    void* ptr;
    if (posix_memalign(&ptr, ALIGN, size * sizeof(float)) != 0) {
        return NULL;
    }
    return (float*)ptr;
}

/* GT_EXPR (>) test with conditional assignment */
void test_gt_expr(float* restrict a, float* restrict b, float* restrict c, float* restrict d) {
    for (int i = 0; i < N; ++i) {
        /* This conditional will be transformed to bit operations when vectorized */
        if (a[i] > b[i]) {
            c[i] = a[i] * b[i];  /* Use both operands to prevent optimization */
        } else {
            c[i] = a[i] + b[i];
        }
        /* Additional computation to ensure the result is used */
        d[i] = c[i] * 2.0f;
    }
}

/* GE_EXPR (>=) test with conditional reduction */
float test_ge_expr(float* restrict a, float* restrict b, float* restrict c) {
    float sum = 0.0f;
    for (int i = 0; i < N; ++i) {
        /* Conditional increment - vectorizer may use mask-based reduction */
        if (a[i] >= b[i]) {
            sum += c[i] * 2.0f;
        } else {
            sum += c[i] * 0.5f;
        }
    }
    return sum;
}

/* LT_EXPR (<) test with conditional blend pattern */
void test_lt_expr(float* restrict a, float* restrict b, float* restrict c, float* restrict d) {
    for (int i = 0; i < N; ++i) {
        /* Ternary operator often gets converted to bit operations */
        float temp = (a[i] < b[i]) ? (a[i] - b[i]) : (b[i] - a[i]);
        c[i] = temp * 2.0f;
        d[i] = c[i] + i;  /* Add index to prevent loop-invariant optimization */
    }
}

/* LE_EXPR (<=) test with masked store pattern */
void test_le_expr(float* restrict a, float* restrict b, float* restrict c) {
    for (int i = 0; i < N; ++i) {
        /* Direct conditional assignment that should generate masks */
        if (a[i] <= b[i]) {
            c[i] = a[i] * 3.0f + b[i] * 2.0f;
        } else {
            c[i] = a[i] * 0.5f - b[i];
        }
    }
}

/* Mixed comparisons to ensure all paths are exercised */
void test_mixed_comparisons(float* restrict a, float* restrict b, 
                           float* restrict c, float* restrict d) {
    for (int i = 0; i < N; ++i) {
        /* Mix different comparison operators in the same loop */
        if (a[i] > b[i]) {
            c[i] = a[i] * 2.0f;
        } else if (a[i] >= b[i] - 1.0f) {
            c[i] = b[i] * 3.0f;
        } else if (a[i] < b[i] - 2.0f) {
            c[i] = a[i] + b[i];
        } else if (a[i] <= b[i] + 1.0f) {
            c[i] = a[i] - b[i];
        } else {
            c[i] = 0.0f;
        }
        d[i] = c[i] * (i % 10 + 1);  /* Data-dependent computation */
    }
}

/* Initialize arrays with patterns that create mixed true/false results */
void init_arrays(float* a, float* b, float* c, float* d) {
    for (int i = 0; i < N; ++i) {
        /* Create varying patterns to ensure all comparison paths are taken */
        a[i] = (float)(i - N/2);          /* Range: -512 to 511 */
        b[i] = (float)((i % 100) - 50);   /* Range: -50 to 49, repeating */
        c[i] = (float)(i * 0.1f);         /* Increasing values */
        d[i] = (float)((i % 20) * 0.5f);  /* Patterned values */
    }
}

/* Compute checksum to verify results and prevent dead code elimination */
float compute_checksum(float* arr, int size) {
    float sum = 0.0f;
    for (int i = 0; i < size; ++i) {
        sum += arr[i];
    }
    return sum;
}

int main() {
    /* Allocate aligned memory for better vectorization */
    float* a = alloc_aligned(N);
    float* b = alloc_aligned(N);
    float* c = alloc_aligned(N);
    float* d = alloc_aligned(N);
    float* e = alloc_aligned(N);
    float* f = alloc_aligned(N);
    
    if (!a || !b || !c || !d || !e || !f) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with varying data */
    init_arrays(a, b, c, d);
    
    printf("Testing vectorized comparisons...\n");
    
    /* Test each comparison operator separately */
    test_gt_expr(a, b, e, f);
    float sum1 = compute_checksum(e, N);
    printf("GT_EXPR (>): checksum = %f\n", sum1);
    
    float sum2 = test_ge_expr(a, b, c);
    printf("GE_EXPR (>=): sum = %f\n", sum2);
    
    test_lt_expr(a, b, e, f);
    float sum3 = compute_checksum(e, N);
    printf("LT_EXPR (<): checksum = %f\n", sum3);
    
    test_le_expr(a, b, e);
    float sum4 = compute_checksum(e, N);
    printf("LE_EXPR (<=): checksum = %f\n", sum4);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(a, b, e, f);
    float sum5 = compute_checksum(e, N);
    printf("Mixed comparisons: checksum = %f\n", sum5);
    
    /* Final validation - compute total checksum */
    float total = sum1 + sum2 + sum3 + sum4 + sum5;
    printf("Total checksum: %f\n", total);
    
    /* Use result to affect return value (prevent optimization) */
    if (total > 1000000.0f) {
        printf("Result is large\n");
    }
    
    /* Cleanup */
    free(a); free(b); free(c); free(d); free(e); free(f);
    
    return 0;
}

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

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test function for GT_EXPR (>) */
void test_gt_expr(int *a, int *b, int *c, int *d) {
    /* Pattern: Conditional assignment based on > comparison
     * This should generate vector mask using BIT_NOT_EXPR and BIT_AND_EXPR */
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] / 2;
        }
    }
    
    /* Another pattern: Masked store with > comparison */
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            d[i] = a[i] + b[i];
        }
    }
}

/* Test function for GE_EXPR (>=) */
void test_ge_expr(int *a, int *b, int *c, int *d) {
    /* Pattern: Conditional assignment based on >= comparison
     * This should generate vector mask using BIT_NOT_EXPR and BIT_IOR_EXPR */
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            c[i] = a[i] * 3;
        } else {
            c[i] = b[i] * 2;
        }
    }
    
    /* Pattern: Conditional reduction with >= */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += c[i];
        }
    }
    d[0] = sum;  /* Store to prevent elimination */
}

/* Test function for LT_EXPR (<) */
void test_lt_expr(int *a, int *b, int *c, int *d) {
    /* Pattern: Conditional assignment based on < comparison
     * This should generate vector mask using BIT_NOT_EXPR and BIT_AND_EXPR
     * with swapped operands */
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            c[i] = a[i] - b[i];
        } else {
            c[i] = b[i] - a[i];
        }
    }
    
    /* Pattern: Ternary operator with < comparison */
    for (int i = 0; i < N; i++) {
        d[i] = (a[i] < b[i]) ? a[i] : b[i];
    }
}

/* Test function for LE_EXPR (<=) */
void test_le_expr(int *a, int *b, int *c, int *d) {
    /* Pattern: Conditional assignment based on <= comparison
     * This should generate vector mask using BIT_NOT_EXPR and BIT_IOR_EXPR
     * with swapped operands */
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            c[i] = a[i] + 100;
        } else {
            c[i] = b[i] - 100;
        }
    }
    
    /* Pattern: Complex conditional with <= */
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i] && a[i] > 0) {
            d[i] = a[i] * b[i];
        }
    }
}

/* Additional test with floating point to ensure different data types */
void test_float_comparisons(float *fa, float *fb, float *fc) {
    /* Test all four operators with float */
    for (int i = 0; i < N; i++) {
        if (fa[i] > fb[i]) {
            fc[i] = fa[i] * 1.5f;
        }
    }
    
    for (int i = 0; i < N; i++) {
        if (fa[i] >= fb[i]) {
            fc[i] += fb[i] * 0.5f;
        }
    }
    
    for (int i = 0; i < N; i++) {
        if (fa[i] < fb[i]) {
            fc[i] = fa[i] - fb[i];
        }
    }
    
    for (int i = 0; i < N; i++) {
        if (fa[i] <= fb[i]) {
            fc[i] *= 2.0f;
        }
    }
}

/* Initialize arrays with varying patterns to ensure mix of true/false comparisons */
void init_arrays(int *a, int *b, int *c, int *d, 
                 float *fa, float *fb, float *fc) {
    for (int i = 0; i < N; i++) {
        /* Create data that will produce mix of comparison results */
        a[i] = i;
        b[i] = N/2 - i % 100;  /* Varying pattern */
        c[i] = 0;
        d[i] = 0;
        
        fa[i] = (float)i * 1.5f;
        fb[i] = (float)(N/2 - i % 50) * 0.8f;
        fc[i] = 0.0f;
    }
}

/* Compute checksum to verify results and prevent dead code elimination */
int compute_checksum(int *c, int *d, float *fc) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += c[i] + d[i];
        sum += (int)fc[i];
    }
    return sum;
}

int main() {
    /* Use aligned allocation for better vectorization */
    int *a = (int*)aligned_alloc(32, N * sizeof(int));
    int *b = (int*)aligned_alloc(32, N * sizeof(int));
    int *c = (int*)aligned_alloc(32, N * sizeof(int));
    int *d = (int*)aligned_alloc(32, N * sizeof(int));
    float *fa = (float*)aligned_alloc(32, N * sizeof(float));
    float *fb = (float*)aligned_alloc(32, N * sizeof(float));
    float *fc = (float*)aligned_alloc(32, N * sizeof(float));
    
    if (!a || !b || !c || !d || !fa || !fb || !fc) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with varying data patterns */
    init_arrays(a, b, c, d, fa, fb, fc);
    
    /* Execute all test functions to trigger different comparison transformations */
    test_gt_expr(a, b, c, d);
    test_ge_expr(a, b, c, d);
    test_lt_expr(a, b, c, d);
    test_le_expr(a, b, c, d);
    test_float_comparisons(fa, fb, fc);
    
    /* Compute and print checksum to ensure code isn't eliminated */
    int checksum = compute_checksum(c, d, fc);
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(fa); free(fb); free(fc);
    
    return 0;
}

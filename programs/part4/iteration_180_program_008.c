/* test_vectorize_comparisons.c
 * Designed to trigger vectorization of comparison operations
 * and hit the bit-operation transformation in tree-vect-stmts.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test functions for each comparison operator */

/* GT_EXPR (>) */
void test_gt_expr(int *a, int *b, int *c, int *d) {
    for (int i = 0; i < N; i++) {
        // Conditional assignment using > operator
        // This should generate vectorized mask operations
        c[i] = (a[i] > b[i]) ? a[i] * 2 : b[i];
        
        // Additional operation to prevent optimization
        d[i] = (a[i] > b[i]) ? d[i] + 1 : d[i] - 1;
    }
}

/* GE_EXPR (>=) */
void test_ge_expr(float *a, float *b, float *c, float *d) {
    for (int i = 0; i < N; i++) {
        // Conditional assignment using >= operator
        c[i] = (a[i] >= b[i]) ? a[i] + b[i] : a[i] - b[i];
        
        // Masked store pattern
        if (a[i] >= b[i]) {
            d[i] = a[i] * b[i];
        }
    }
}

/* LT_EXPR (<) */
void test_lt_expr(int *a, int *b, int *c, int *result) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        // Conditional increment using < operator
        sum += (a[i] < b[i]) ? c[i] : 0;
        
        // Another conditional operation
        c[i] = (a[i] < b[i]) ? a[i] : b[i];
    }
    *result = sum;
}

/* LE_EXPR (<=) */
void test_le_expr(double *a, double *b, double *c, double *d) {
    for (int i = 0; i < N; i++) {
        // Complex conditional using <= operator
        // This should force mask generation
        if (a[i] <= b[i]) {
            c[i] = a[i] * 3.0;
            d[i] = b[i] * 2.0;
        } else {
            c[i] = b[i];
            d[i] = a[i];
        }
    }
}

/* Mixed comparisons in same loop */
void test_mixed_comparisons(int *a, int *b, int *c, int *d) {
    for (int i = 0; i < N; i++) {
        // Mix of different comparison operators
        if (a[i] > b[i]) {
            c[i] = a[i] - b[i];
        } else if (a[i] >= b[i]) {
            c[i] = a[i] + b[i];
        } else if (a[i] < b[i]) {
            c[i] = b[i] - a[i];
        } else if (a[i] <= b[i]) {
            c[i] = a[i] * b[i];
        }
        
        // Additional operation to use result
        d[i] = c[i] % 256;
    }
}

/* Helper function to initialize arrays with varying patterns */
void init_arrays(int *a, int *b, int *c, int *d,
                 float *fa, float *fb, float *fc, float *fd,
                 double *da, double *db, double *dc, double *dd) {
    for (int i = 0; i < N; i++) {
        // Create varying patterns to ensure mix of true/false comparisons
        a[i] = i;
        b[i] = N/2 - i % 100;  // Creates crossing points
        c[i] = i * 2;
        d[i] = i % 50;
        
        fa[i] = (float)i * 1.5f;
        fb[i] = (float)(N/2 - i % 75) * 0.8f;
        fc[i] = 0.0f;
        fd[i] = (float)i * 0.25f;
        
        da[i] = (double)i * 0.75;
        db[i] = (double)(i % 150) * 1.2;
        dc[i] = 0.0;
        dd[i] = (double)i * 0.33;
    }
}

/* Verification function */
int verify_results(int *c1, int *c2, float *fc, double *dc) {
    int checksum = 0;
    
    for (int i = 0; i < N; i++) {
        checksum += c1[i] + c2[i];
        checksum += (int)fc[i];
        checksum += (int)dc[i];
    }
    
    return checksum;
}

int main() {
    /* Use aligned allocations for better vectorization */
    int *a = (int*)aligned_alloc(32, N * sizeof(int));
    int *b = (int*)aligned_alloc(32, N * sizeof(int));
    int *c1 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c2 = (int*)aligned_alloc(32, N * sizeof(int));
    int *d1 = (int*)aligned_alloc(32, N * sizeof(int));
    int *d2 = (int*)aligned_alloc(32, N * sizeof(int));
    
    float *fa = (float*)aligned_alloc(32, N * sizeof(float));
    float *fb = (float*)aligned_alloc(32, N * sizeof(float));
    float *fc = (float*)aligned_alloc(32, N * sizeof(float));
    float *fd = (float*)aligned_alloc(32, N * sizeof(float));
    
    double *da = (double*)aligned_alloc(32, N * sizeof(double));
    double *db = (double*)aligned_alloc(32, N * sizeof(double));
    double *dc = (double*)aligned_alloc(32, N * sizeof(double));
    double *dd = (double*)aligned_alloc(32, N * sizeof(double));
    
    int reduction_result = 0;
    
    if (!a || !b || !c1 || !c2 || !d1 || !d2 || 
        !fa || !fb || !fc || !fd || 
        !da || !db || !dc || !dd) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize all arrays */
    init_arrays(a, b, c1, d1, fa, fb, fc, fd, da, db, dc, dd);
    
    /* Execute all test functions to trigger different comparison operators */
    
    // GT_EXPR test
    test_gt_expr(a, b, c1, d1);
    
    // GE_EXPR test
    test_ge_expr(fa, fb, fc, fd);
    
    // LT_EXPR test
    test_lt_expr(a, b, c2, &reduction_result);
    
    // LE_EXPR test
    test_le_expr(da, db, dc, dd);
    
    // Mixed comparisons test
    test_mixed_comparisons(a, b, c2, d2);
    
    /* Verify results to prevent dead code elimination */
    int final_checksum = verify_results(c1, c2, fc, dc);
    
    printf("Test completed. Checksum: %d, Reduction result: %d\n", 
           final_checksum, reduction_result);
    
    /* Cleanup */
    free(a); free(b); free(c1); free(c2); free(d1); free(d2);
    free(fa); free(fb); free(fc); free(fd);
    free(da); free(db); free(dc); free(dd);
    
    return 0;
}

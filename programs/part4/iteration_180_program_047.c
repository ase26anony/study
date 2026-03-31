/* test_vectorized_comparisons.c
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
void test_gt_expr(int *restrict a, int *restrict b, int *restrict c, int *restrict d) {
    /* Conditional assignment using > operator */
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] / 2;
        }
    }
    
    /* Masked operation using > operator */
    for (int i = 0; i < N; i++) {
        d[i] = (a[i] > b[i]) ? (a[i] + b[i]) : (a[i] - b[i]);
    }
}

/* Test function for GE_EXPR (>=) */
void test_ge_expr(int *restrict a, int *restrict b, int *restrict c, int *restrict d) {
    int sum = 0;
    
    /* Conditional reduction using >= operator */
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    
    /* Store sum to prevent optimization */
    d[0] = sum;
    
    /* Conditional blend using >= operator */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? a[i] : b[i];
    }
}

/* Test function for LT_EXPR (<) */
void test_lt_expr(int *restrict a, int *restrict b, int *restrict c, int *restrict d) {
    /* Conditional assignment using < operator */
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            c[i] = a[i] * 3;
        } else {
            c[i] = b[i] * 2;
        }
    }
    
    /* Complex conditional using < operator */
    for (int i = 0; i < N; i++) {
        d[i] = (a[i] < b[i]) ? (a[i] << 2) : (b[i] >> 1);
    }
}

/* Test function for LE_EXPR (<=) */
void test_le_expr(int *restrict a, int *restrict b, int *restrict c, int *restrict d) {
    /* Conditional assignment using <= operator */
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
    
    /* Masked store with <= operator */
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            d[i] = a[i] * b[i];
        }
    }
}

/* Mixed test with all operators in one loop */
void test_mixed_comparisons(float *restrict fa, float *restrict fb, 
                           float *restrict fc, float *restrict fd) {
    /* Use different comparison types in the same loop */
    for (int i = 0; i < N; i++) {
        if (fa[i] > fb[i]) {
            fc[i] = fa[i] * 2.0f;
        } else if (fa[i] >= fb[i]) {
            fc[i] = fa[i] + fb[i];
        } else if (fa[i] < fb[i]) {
            fc[i] = fb[i] - fa[i];
        } else if (fa[i] <= fb[i]) {
            fc[i] = fa[i] / 2.0f;
        } else {
            fc[i] = 0.0f;
        }
    }
}

/* Initialize arrays with patterns that create mixed true/false results */
void init_arrays(int *a, int *b, float *fa, float *fb) {
    for (int i = 0; i < N; i++) {
        /* Create alternating patterns for integer arrays */
        a[i] = i;
        b[i] = (i % 3 == 0) ? i + 5 : 
               (i % 3 == 1) ? i - 2 : 
               N/2;
        
        /* Create patterns for floating point arrays */
        fa[i] = (float)i * 1.5f;
        fb[i] = (float)(i % 10) * 3.0f;
    }
}

/* Compute checksum to ensure computations aren't optimized away */
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
    int *c1 = (int*)aligned_alloc(32, N * sizeof(int));
    int *d1 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c2 = (int*)aligned_alloc(32, N * sizeof(int));
    int *d2 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c3 = (int*)aligned_alloc(32, N * sizeof(int));
    int *d3 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c4 = (int*)aligned_alloc(32, N * sizeof(int));
    int *d4 = (int*)aligned_alloc(32, N * sizeof(int));
    
    float *fa = (float*)aligned_alloc(32, N * sizeof(float));
    float *fb = (float*)aligned_alloc(32, N * sizeof(float));
    float *fc = (float*)aligned_alloc(32, N * sizeof(float));
    float *fd = (float*)aligned_alloc(32, N * sizeof(float));
    
    if (!a || !b || !c1 || !d1 || !c2 || !d2 || !c3 || !d3 || !c4 || !d4 ||
        !fa || !fb || !fc || !fd) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with data patterns */
    init_arrays(a, b, fa, fb);
    
    /* Clear output arrays */
    memset(c1, 0, N * sizeof(int));
    memset(d1, 0, N * sizeof(int));
    memset(c2, 0, N * sizeof(int));
    memset(d2, 0, N * sizeof(int));
    memset(c3, 0, N * sizeof(int));
    memset(d3, 0, N * sizeof(int));
    memset(c4, 0, N * sizeof(int));
    memset(d4, 0, N * sizeof(int));
    memset(fc, 0, N * sizeof(float));
    memset(fd, 0, N * sizeof(float));
    
    /* Execute all test functions */
    test_gt_expr(a, b, c1, d1);      /* Tests > operator */
    test_ge_expr(a, b, c2, d2);      /* Tests >= operator */
    test_lt_expr(a, b, c3, d3);      /* Tests < operator */
    test_le_expr(a, b, c4, d4);      /* Tests <= operator */
    test_mixed_comparisons(fa, fb, fc, fd); /* Tests all operators */
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = compute_checksum(c1, d1, fc);
    checksum += compute_checksum(c2, d2, fc);
    checksum += compute_checksum(c3, d3, fc);
    checksum += compute_checksum(c4, d4, fc);
    
    printf("Test completed successfully. Checksum: %d\n", checksum);
    
    /* Free allocated memory */
    free(a); free(b);
    free(c1); free(d1);
    free(c2); free(d2);
    free(c3); free(d3);
    free(c4); free(d4);
    free(fa); free(fb);
    free(fc); free(fd);
    
    return 0;
}

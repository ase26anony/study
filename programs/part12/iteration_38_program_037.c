/* test_vectorized_comparisons.c
 * Designed to trigger vectorization of GT_EXPR, GE_EXPR, LT_EXPR, and LE_EXPR
 * comparisons in GCC's tree-vect-stmts.cc (lines 12216-12233)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test function for GT_EXPR (>) */
void test_gt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  /* GT_EXPR - should trigger case GT_EXPR */
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i];
        }
    }
}

/* Test function for GE_EXPR (>=) */
void test_ge(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {  /* GE_EXPR - should trigger case GE_EXPR */
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

/* Test function for LT_EXPR (<) */
void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {  /* LT_EXPR - should trigger case LT_EXPR with swap */
            c[i] = a[i] * 3;
        } else {
            c[i] = b[i] * 2;
        }
    }
}

/* Test function for LE_EXPR (<=) */
void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {  /* LE_EXPR - should trigger case LE_EXPR with swap */
            c[i] = a[i] + 1;
        } else {
            c[i] = b[i] - 1;
        }
    }
}

/* Additional test with floating point to ensure different type handling */
void test_gt_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  /* GT_EXPR with floats */
            c[i] = a[i] * 2.0f;
        } else {
            c[i] = b[i];
        }
    }
}

/* Test with mixed comparisons in same loop */
void test_mixed_comparisons(int *restrict a, int *restrict b, int *restrict c, int *restrict d) {
    for (int i = 0; i < N; i++) {
        /* Multiple comparisons to potentially trigger different cases */
        int cond1 = a[i] > b[i];   /* GT_EXPR */
        int cond2 = a[i] >= b[i];  /* GE_EXPR */
        int cond3 = a[i] < b[i];   /* LT_EXPR */
        int cond4 = a[i] <= b[i];  /* LE_EXPR */
        
        /* Use comparisons to compute result */
        c[i] = (cond1 ? a[i] : b[i]) + (cond2 ? 1 : -1);
        d[i] = (cond3 ? a[i] : 0) + (cond4 ? b[i] : 0);
    }
}

/* Test with ternary operator (?:) which also generates comparisons */
void test_ternary(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] > b[i]) ? a[i] : b[i];  /* GT_EXPR in ternary */
    }
}

/* Initialize arrays with pattern that creates varied comparison results */
void init_arrays(int *a, int *b) {
    for (int i = 0; i < N; i++) {
        a[i] = i;           /* 0, 1, 2, 3, ... */
        b[i] = N/2 - i;     /* 512, 511, 510, ... */
    }
}

void init_float_arrays(float *a, float *b) {
    for (int i = 0; i < N; i++) {
        a[i] = i * 0.5f;
        b[i] = (N/2 - i) * 0.5f;
    }
}

/* Compute checksum to ensure loops execute and produce results */
int compute_checksum(int *arr) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr[i];
    }
    return sum;
}

float compute_checksum_float(float *arr) {
    float sum = 0.0f;
    for (int i = 0; i < N; i++) {
        sum += arr[i];
    }
    return sum;
}

int main() {
    /* Use aligned allocation for better vectorization */
    int *a = aligned_alloc(32, N * sizeof(int));
    int *b = aligned_alloc(32, N * sizeof(int));
    int *c1 = aligned_alloc(32, N * sizeof(int));
    int *c2 = aligned_alloc(32, N * sizeof(int));
    int *c3 = aligned_alloc(32, N * sizeof(int));
    int *c4 = aligned_alloc(32, N * sizeof(int));
    int *c5 = aligned_alloc(32, N * sizeof(int));
    int *d = aligned_alloc(32, N * sizeof(int));
    
    float *fa = aligned_alloc(32, N * sizeof(float));
    float *fb = aligned_alloc(32, N * sizeof(float));
    float *fc = aligned_alloc(32, N * sizeof(float));
    
    if (!a || !b || !c1 || !c2 || !c3 || !c4 || !c5 || !d || !fa || !fb || !fc) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(a, b);
    init_float_arrays(fa, fb);
    
    /* Execute all test functions */
    test_gt(a, b, c1);
    test_ge(a, b, c2);
    test_lt(a, b, c3);
    test_le(a, b, c4);
    test_mixed_comparisons(a, b, c5, d);
    test_ternary(a, b, c1);  /* Reuse c1 */
    test_gt_float(fa, fb, fc);
    
    /* Compute checksums to ensure code executed */
    int checksum1 = compute_checksum(c1);
    int checksum2 = compute_checksum(c2);
    int checksum3 = compute_checksum(c3);
    int checksum4 = compute_checksum(c4);
    int checksum5 = compute_checksum(c5);
    int checksum_d = compute_checksum(d);
    float checksum_f = compute_checksum_float(fc);
    
    printf("Checksums (for verification):\n");
    printf("  test_gt: %d\n", checksum1);
    printf("  test_ge: %d\n", checksum2);
    printf("  test_lt: %d\n", checksum3);
    printf("  test_le: %d\n", checksum4);
    printf("  test_mixed: %d, %d\n", checksum5, checksum_d);
    printf("  test_gt_float: %.2f\n", checksum_f);
    
    /* Cleanup */
    free(a); free(b); free(c1); free(c2); free(c3); free(c4); free(c5); free(d);
    free(fa); free(fb); free(fc);
    
    return 0;
}

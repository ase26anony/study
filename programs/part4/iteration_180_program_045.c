#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test functions for each comparison operator */

void test_gt_expr(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c, ALIGNED int *d) {
    /* GT_EXPR: if (a[i] > b[i]) c[i] = d[i] * 2 else c[i] = d[i] */
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            c[i] = d[i] * 2;
        } else {
            c[i] = d[i];
        }
    }
}

void test_ge_expr(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c, ALIGNED int *d) {
    /* GE_EXPR: if (a[i] >= b[i]) c[i] = a[i] + b[i] else c[i] = a[i] - b[i] */
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

void test_lt_expr(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c, ALIGNED int *d) {
    /* LT_EXPR: if (a[i] < b[i]) c[i] = d[i] else c[i] = b[i] */
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            c[i] = d[i];
        } else {
            c[i] = b[i];
        }
    }
}

void test_le_expr(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c, ALIGNED int *d) {
    /* LE_EXPR: if (a[i] <= b[i]) c[i] = a[i] * b[i] else c[i] = a[i] */
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i];
        }
    }
}

/* Additional test with floating point to ensure different data types */
void test_gt_expr_float(ALIGNED float *a, ALIGNED float *b, ALIGNED float *c) {
    /* GT_EXPR with floats: conditional assignment */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
}

void test_ge_expr_float(ALIGNED float *a, ALIGNED float *b, ALIGNED float *c) {
    /* GE_EXPR with floats: conditional operation */
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            c[i] = a[i] * 2.0f;
        } else {
            c[i] = b[i] * 3.0f;
        }
    }
}

/* Reduction pattern that uses comparisons */
int test_reduction_lt(ALIGNED int *a, ALIGNED int *b) {
    /* LT_EXPR in reduction: sum += (a[i] < b[i]) ? a[i] : 0 */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += (a[i] < b[i]) ? a[i] : 0;
    }
    return sum;
}

int test_reduction_le(ALIGNED int *a, ALIGNED int *b) {
    /* LE_EXPR in reduction: sum += (a[i] <= b[i]) ? b[i] : 0 */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += (a[i] <= b[i]) ? b[i] : 0;
    }
    return sum;
}

/* Helper function to initialize arrays with varying patterns */
void init_arrays(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c, ALIGNED int *d) {
    for (int i = 0; i < N; i++) {
        a[i] = i;                     /* 0, 1, 2, 3, ... */
        b[i] = N/2 - i;               /* 512, 511, 510, ... */
        c[i] = 0;                     /* Will be overwritten */
        d[i] = (i % 3) * 10 + 5;      /* Pattern: 5, 15, 25, 5, ... */
    }
}

void init_float_arrays(ALIGNED float *a, ALIGNED float *b) {
    for (int i = 0; i < N; i++) {
        a[i] = (float)i * 0.5f;
        b[i] = (float)(N - i) * 0.3f;
    }
}

/* Verification function to ensure computations are correct */
int verify_results(ALIGNED int *c1, ALIGNED int *c2, ALIGNED int *c3, ALIGNED int *c4) {
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += c1[i] + c2[i] + c3[i] + c4[i];
    }
    return checksum;
}

int main() {
    /* Allocate aligned memory for better vectorization */
    ALIGNED int *a = (ALIGNED int*)aligned_alloc(32, N * sizeof(int));
    ALIGNED int *b = (ALIGNED int*)aligned_alloc(32, N * sizeof(int));
    ALIGNED int *c1 = (ALIGNED int*)aligned_alloc(32, N * sizeof(int));
    ALIGNED int *c2 = (ALIGNED int*)aligned_alloc(32, N * sizeof(int));
    ALIGNED int *c3 = (ALIGNED int*)aligned_alloc(32, N * sizeof(int));
    ALIGNED int *c4 = (ALIGNED int*)aligned_alloc(32, N * sizeof(int));
    ALIGNED int *d = (ALIGNED int*)aligned_alloc(32, N * sizeof(int));
    
    ALIGNED float *fa = (ALIGNED float*)aligned_alloc(32, N * sizeof(float));
    ALIGNED float *fb = (ALIGNED float*)aligned_alloc(32, N * sizeof(float));
    ALIGNED float *fc1 = (ALIGNED float*)aligned_alloc(32, N * sizeof(float));
    ALIGNED float *fc2 = (ALIGNED float*)aligned_alloc(32, N * sizeof(float));
    
    if (!a || !b || !c1 || !c2 || !c3 || !c4 || !d || !fa || !fb || !fc1 || !fc2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with varying patterns to ensure mix of true/false comparisons */
    init_arrays(a, b, c1, d);
    init_float_arrays(fa, fb);
    
    /* Execute all test functions to trigger different comparison operators */
    test_gt_expr(a, b, c1, d);
    test_ge_expr(a, b, c2, d);
    test_lt_expr(a, b, c3, d);
    test_le_expr(a, b, c4, d);
    
    test_gt_expr_float(fa, fb, fc1);
    test_ge_expr_float(fa, fb, fc2);
    
    int red1 = test_reduction_lt(a, b);
    int red2 = test_reduction_le(a, b);
    
    /* Verify results by computing checksums (prevents dead code elimination) */
    int int_checksum = verify_results(c1, c2, c3, c4);
    
    float float_checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        float_checksum += fc1[i] + fc2[i];
    }
    
    printf("Integer checksum: %d\n", int_checksum);
    printf("Float checksum: %.2f\n", float_checksum);
    printf("Reduction results: %d, %d\n", red1, red2);
    
    /* Clean up */
    free(a); free(b); free(c1); free(c2); free(c3); free(c4); free(d);
    free(fa); free(fb); free(fc1); free(fc2);
    
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test functions for each comparison type */
void test_gt_expr(int *a, int *b, int *c, int *d) {
    /* GT_EXPR (>): if (a[i] > b[i]) c[i] = d[i] * 2 else c[i] = d[i] */
    for (int i = 0; i < N; ++i) {
        if (a[i] > b[i]) {
            c[i] = d[i] * 2;
        } else {
            c[i] = d[i];
        }
    }
}

void test_ge_expr(int *a, int *b, int *c, int *d) {
    /* GE_EXPR (>=): Conditional reduction pattern */
    int sum = 0;
    for (int i = 0; i < N; ++i) {
        if (a[i] >= b[i]) {
            sum += d[i];
        }
    }
    /* Store result to prevent elimination */
    c[0] = sum;
}

void test_lt_expr(float *a, float *b, float *c) {
    /* LT_EXPR (<): Masked store with floating point */
    for (int i = 0; i < N; ++i) {
        if (a[i] < b[i]) {
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] + b[i];
        }
    }
}

void test_le_expr(short *a, short *b, short *c) {
    /* LE_EXPR (<=): Ternary conditional assignment */
    for (int i = 0; i < N; ++i) {
        c[i] = (a[i] <= b[i]) ? (a[i] + b[i]) : (a[i] - b[i]);
    }
}

/* Additional test with mixed operations to increase coverage */
void test_mixed_comparisons(int *a, int *b, int *c, int *d) {
    /* Mix of different comparison types in same function */
    for (int i = 0; i < N; ++i) {
        /* GT and LT in same loop */
        if (a[i] > b[i]) {
            c[i] = d[i] << 1;
        } else if (a[i] < b[i]) {
            c[i] = d[i] >> 1;
        } else {
            c[i] = d[i];
        }
    }
}

/* Helper function to verify results */
int verify_results(int *ref, int *test, int size) {
    for (int i = 0; i < size; ++i) {
        if (ref[i] != test[i]) {
            return 0;
        }
    }
    return 1;
}

int main() {
    /* Aligned allocations for better vectorization */
    int *a_int ALIGNED = (int*)aligned_alloc(32, N * sizeof(int));
    int *b_int ALIGNED = (int*)aligned_alloc(32, N * sizeof(int));
    int *c_int ALIGNED = (int*)aligned_alloc(32, N * sizeof(int));
    int *d_int ALIGNED = (int*)aligned_alloc(32, N * sizeof(int));
    int *ref_int ALIGNED = (int*)aligned_alloc(32, N * sizeof(int));
    
    float *a_float ALIGNED = (float*)aligned_alloc(32, N * sizeof(float));
    float *b_float ALIGNED = (float*)aligned_alloc(32, N * sizeof(float));
    float *c_float ALIGNED = (float*)aligned_alloc(32, N * sizeof(float));
    
    short *a_short ALIGNED = (short*)aligned_alloc(32, N * sizeof(short));
    short *b_short ALIGNED = (short*)aligned_alloc(32, N * sizeof(short));
    short *c_short ALIGNED = (short*)aligned_alloc(32, N * sizeof(short));
    
    /* Initialize with varying patterns to ensure mix of true/false comparisons */
    for (int i = 0; i < N; ++i) {
        /* Integer arrays: create pattern where comparisons will be both true and false */
        a_int[i] = i;
        b_int[i] = N/2;  /* Half will be greater, half less */
        d_int[i] = i % 100;
        
        /* Floating point arrays */
        a_float[i] = (float)(i - N/2) * 0.5f;
        b_float[i] = (float)(i % 10) * 1.5f;
        
        /* Short arrays */
        a_short[i] = (short)(i % 256);
        b_short[i] = (short)(128 - i % 256);
    }
    
    /* Test GT_EXPR pattern */
    printf("Testing GT_EXPR (> operator)...\n");
    test_gt_expr(a_int, b_int, c_int, d_int);
    
    /* Test GE_EXPR pattern */
    printf("Testing GE_EXPR (>= operator)...\n");
    memset(c_int, 0, N * sizeof(int));
    test_ge_expr(a_int, b_int, c_int, d_int);
    
    /* Test LT_EXPR pattern */
    printf("Testing LT_EXPR (< operator)...\n");
    test_lt_expr(a_float, b_float, c_float);
    
    /* Test LE_EXPR pattern */
    printf("Testing LE_EXPR (<= operator)...\n");
    test_le_expr(a_short, b_short, c_short);
    
    /* Test mixed comparisons */
    printf("Testing mixed comparisons...\n");
    test_mixed_comparisons(a_int, b_int, ref_int, d_int);
    
    /* Compute checksum to ensure computations aren't optimized away */
    int checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += c_int[i] + ref_int[i] + (int)c_float[i] + c_short[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed. If compiled with vectorization flags,\n");
    printf("the uncovered lines in tree-vect-stmts.cc should be exercised.\n");
    
    /* Cleanup */
    free(a_int);
    free(b_int);
    free(c_int);
    free(d_int);
    free(ref_int);
    free(a_float);
    free(b_float);
    free(c_float);
    free(a_short);
    free(b_short);
    free(c_short);
    
    return 0;
}

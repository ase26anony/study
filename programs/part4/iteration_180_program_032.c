#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test functions for each comparison operator */
void test_gt_expr(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c, ALIGNED int *d) {
    /* GT_EXPR: if (a[i] > b[i]) c[i] = d[i] * 2 else c[i] = d[i] */
    for (int i = 0; i < N; ++i) {
        if (a[i] > b[i]) {
            c[i] = d[i] * 2;
        } else {
            c[i] = d[i];
        }
    }
}

void test_ge_expr(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c, ALIGNED int *d) {
    /* GE_EXPR: Conditional reduction using >= */
    int sum = 0;
    for (int i = 0; i < N; ++i) {
        if (a[i] >= b[i]) {
            sum += d[i];
        }
    }
    c[0] = sum; /* Store result to prevent elimination */
}

void test_lt_expr(ALIGNED float *a, ALIGNED float *b, ALIGNED float *c) {
    /* LT_EXPR: Masked store with < comparison */
    for (int i = 0; i < N; ++i) {
        if (a[i] < b[i]) {
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] + b[i];
        }
    }
}

void test_le_expr(ALIGNED short *a, ALIGNED short *b, ALIGNED short *c) {
    /* LE_EXPR: Ternary conditional with <= */
    for (int i = 0; i < N; ++i) {
        c[i] = (a[i] <= b[i]) ? (a[i] + b[i]) : (a[i] - b[i]);
    }
}

/* Additional test with mixed operations to increase coverage */
void test_mixed_comparisons(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    /* Mix of comparisons in different loops */
    for (int i = 0; i < N; ++i) {
        if (a[i] > b[i]) {
            c[i] = a[i] * 3;
        }
    }
    
    for (int i = 0; i < N; ++i) {
        if (a[i] >= b[i]) {
            c[i] += b[i] * 2;
        }
    }
    
    for (int i = 0; i < N; ++i) {
        if (a[i] < b[i]) {
            c[i] = a[i] - b[i];
        }
    }
    
    for (int i = 0; i < N; ++i) {
        if (a[i] <= b[i]) {
            c[i] += 1;
        }
    }
}

/* Reference computation for validation */
int compute_reference_gt(ALIGNED int *a, ALIGNED int *b, ALIGNED int *d) {
    int sum = 0;
    for (int i = 0; i < N; ++i) {
        sum += (a[i] > b[i]) ? (d[i] * 2) : d[i];
    }
    return sum;
}

int compute_reference_ge(ALIGNED int *a, ALIGNED int *b, ALIGNED int *d) {
    int sum = 0;
    for (int i = 0; i < N; ++i) {
        if (a[i] >= b[i]) {
            sum += d[i];
        }
    }
    return sum;
}

float compute_reference_lt(ALIGNED float *a, ALIGNED float *b) {
    float sum = 0.0f;
    for (int i = 0; i < N; ++i) {
        sum += (a[i] < b[i]) ? (a[i] * b[i]) : (a[i] + b[i]);
    }
    return sum;
}

int compute_reference_le(ALIGNED short *a, ALIGNED short *b) {
    int sum = 0;
    for (int i = 0; i < N; ++i) {
        sum += (a[i] <= b[i]) ? (a[i] + b[i]) : (a[i] - b[i]);
    }
    return sum;
}

int main() {
    /* Allocate aligned memory for better vectorization */
    ALIGNED int *a_int = (ALIGNED int*)aligned_alloc(32, N * sizeof(int));
    ALIGNED int *b_int = (ALIGNED int*)aligned_alloc(32, N * sizeof(int));
    ALIGNED int *c_int = (ALIGNED int*)aligned_alloc(32, N * sizeof(int));
    ALIGNED int *d_int = (ALIGNED int*)aligned_alloc(32, N * sizeof(int));
    
    ALIGNED float *a_float = (ALIGNED float*)aligned_alloc(32, N * sizeof(float));
    ALIGNED float *b_float = (ALIGNED float*)aligned_alloc(32, N * sizeof(float));
    ALIGNED float *c_float = (ALIGNED float*)aligned_alloc(32, N * sizeof(float));
    
    ALIGNED short *a_short = (ALIGNED short*)aligned_alloc(32, N * sizeof(short));
    ALIGNED short *b_short = (ALIGNED short*)aligned_alloc(32, N * sizeof(short));
    ALIGNED short *c_short = (ALIGNED short*)aligned_alloc(32, N * sizeof(short));
    
    /* Initialize with varying patterns to ensure mix of true/false comparisons */
    for (int i = 0; i < N; ++i) {
        a_int[i] = i;
        b_int[i] = N/2;
        d_int[i] = i % 10 + 1;
        c_int[i] = 0;
        
        a_float[i] = (float)(i - N/2) * 0.5f;
        b_float[i] = (float)(i % 20) * 0.25f;
        c_float[i] = 0.0f;
        
        a_short[i] = (short)(i % 100);
        b_short[i] = (short)(50 - i % 100);
        c_short[i] = 0;
    }
    
    /* Execute all test functions */
    test_gt_expr(a_int, b_int, c_int, d_int);
    test_ge_expr(a_int, b_int, c_int, d_int);
    test_lt_expr(a_float, b_float, c_float);
    test_le_expr(a_short, b_short, c_short);
    
    /* Test with mixed comparisons */
    ALIGNED int *mixed_a = (ALIGNED int*)aligned_alloc(32, N * sizeof(int));
    ALIGNED int *mixed_b = (ALIGNED int*)aligned_alloc(32, N * sizeof(int));
    ALIGNED int *mixed_c = (ALIGNED int*)aligned_alloc(32, N * sizeof(int));
    
    for (int i = 0; i < N; ++i) {
        mixed_a[i] = i * 2;
        mixed_b[i] = i * 3 / 2;
        mixed_c[i] = 0;
    }
    
    test_mixed_comparisons(mixed_a, mixed_b, mixed_c);
    
    /* Verify results against reference computations */
    int ref_gt = compute_reference_gt(a_int, b_int, d_int);
    int ref_ge = compute_reference_ge(a_int, b_int, d_int);
    float ref_lt = compute_reference_lt(a_float, b_float);
    int ref_le = compute_reference_le(a_short, b_short);
    
    /* Compute actual results from test outputs */
    int actual_gt = 0;
    for (int i = 0; i < N; ++i) {
        actual_gt += c_int[i];
    }
    
    float actual_lt = 0.0f;
    for (int i = 0; i < N; ++i) {
        actual_lt += c_float[i];
    }
    
    int actual_le = 0;
    for (int i = 0; i < N; ++i) {
        actual_le += c_short[i];
    }
    
    /* Check results (tolerance for floating point) */
    int errors = 0;
    
    if (actual_gt != ref_gt) {
        printf("GT_EXPR test failed: %d != %d\n", actual_gt, ref_gt);
        errors++;
    }
    
    if (c_int[0] != ref_ge) {
        printf("GE_EXPR test failed: %d != %d\n", c_int[0], ref_ge);
        errors++;
    }
    
    if (abs(actual_lt - ref_lt) > 0.001f) {
        printf("LT_EXPR test failed: %f != %f\n", actual_lt, ref_lt);
        errors++;
    }
    
    if (actual_le != ref_le) {
        printf("LE_EXPR test failed: %d != %d\n", actual_le, ref_le);
        errors++;
    }
    
    /* Check mixed results */
    int mixed_sum = 0;
    for (int i = 0; i < N; ++i) {
        mixed_sum += mixed_c[i];
    }
    printf("Mixed comparisons sum: %d\n", mixed_sum);
    
    if (errors == 0) {
        printf("All tests passed successfully!\n");
    } else {
        printf("%d test(s) failed\n", errors);
    }
    
    /* Free allocated memory */
    free(a_int); free(b_int); free(c_int); free(d_int);
    free(a_float); free(b_float); free(c_float);
    free(a_short); free(b_short); free(c_short);
    free(mixed_a); free(mixed_b); free(mixed_c);
    
    return errors;
}

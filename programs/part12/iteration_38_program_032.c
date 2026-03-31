/* test_vectorized_comparisons.c
 * Designed to trigger vectorization of comparison operations
 * to cover lines 12216-12233 in tree-vect-stmts.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test function for GT_EXPR (>) */
void test_gt(int *restrict a, int *restrict b, int *restrict c, int *restrict out) {
    for (int i = 0; i < N; i++) {
        /* This comparison should trigger GT_EXPR case */
        if (a[i] > b[i]) {
            out[i] = c[i] * 2;
        } else {
            out[i] = c[i] / 2;
        }
    }
}

/* Test function for GE_EXPR (>=) */
void test_ge(int *restrict a, int *restrict b, int *restrict c, int *restrict out) {
    for (int i = 0; i < N; i++) {
        /* This comparison should trigger GE_EXPR case */
        if (a[i] >= b[i]) {
            out[i] = c[i] + a[i];
        } else {
            out[i] = c[i] - b[i];
        }
    }
}

/* Test function for LT_EXPR (<) */
void test_lt(int *restrict a, int *restrict b, int *restrict c, int *restrict out) {
    for (int i = 0; i < N; i++) {
        /* This comparison should trigger LT_EXPR case */
        if (a[i] < b[i]) {
            out[i] = c[i] * 3;
        } else {
            out[i] = c[i];
        }
    }
}

/* Test function for LE_EXPR (<=) */
void test_le(int *restrict a, int *restrict b, int *restrict c, int *restrict out) {
    for (int i = 0; i < N; i++) {
        /* This comparison should trigger LE_EXPR case */
        if (a[i] <= b[i]) {
            out[i] = c[i] - a[i];
        } else {
            out[i] = c[i] + b[i];
        }
    }
}

/* Additional test with floating point to ensure different type handling */
void test_gt_float(float *restrict a, float *restrict b, float *restrict c, float *restrict out) {
    for (int i = 0; i < N; i++) {
        /* Floating point GT_EXPR */
        if (a[i] > b[i]) {
            out[i] = c[i] * 2.0f;
        } else {
            out[i] = c[i] / 2.0f;
        }
    }
}

void test_lt_float(float *restrict a, float *restrict b, float *restrict c, float *restrict out) {
    for (int i = 0; i < N; i++) {
        /* Floating point LT_EXPR with std::swap in the transformation */
        if (a[i] < b[i]) {
            out[i] = c[i] * 3.0f;
        } else {
            out[i] = c[i];
        }
    }
}

/* Initialize arrays with patterned data to create varied comparison results */
void init_arrays(int *a, int *b, int *c) {
    for (int i = 0; i < N; i++) {
        a[i] = i;              /* 0, 1, 2, 3, ... */
        b[i] = N - i - 1;      /* 1023, 1022, 1021, ... */
        c[i] = (i % 3 == 0) ? i * 2 : i;  /* Patterned data */
    }
}

void init_float_arrays(float *a, float *b, float *c) {
    for (int i = 0; i < N; i++) {
        a[i] = (float)i * 1.5f;
        b[i] = (float)(N - i) * 0.5f;
        c[i] = (float)(i % 5) * 2.0f;
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
    /* Aligned arrays for better vectorization */
    ALIGNED int a[N], b[N], c[N];
    ALIGNED int out_gt[N], out_ge[N], out_lt[N], out_le[N];
    ALIGNED float fa[N], fb[N], fc[N];
    ALIGNED float fout_gt[N], fout_lt[N];
    
    /* Initialize data */
    init_arrays(a, b, c);
    init_float_arrays(fa, fb, fc);
    
    /* Execute all test functions */
    test_gt(a, b, c, out_gt);
    test_ge(a, b, c, out_ge);
    test_lt(a, b, c, out_lt);
    test_le(a, b, c, out_le);
    test_gt_float(fa, fb, fc, fout_gt);
    test_lt_float(fa, fb, fc, fout_lt);
    
    /* Compute and print checksums to prevent dead code elimination */
    int checksum_gt = compute_checksum(out_gt);
    int checksum_ge = compute_checksum(out_ge);
    int checksum_lt = compute_checksum(out_lt);
    int checksum_le = compute_checksum(out_le);
    float checksum_fgt = compute_checksum_float(fout_gt);
    float checksum_flt = compute_checksum_float(fout_lt);
    
    printf("Checksums (to verify execution):\n");
    printf("GT: %d\n", checksum_gt);
    printf("GE: %d\n", checksum_ge);
    printf("LT: %d\n", checksum_lt);
    printf("LE: %d\n", checksum_le);
    printf("Float GT: %f\n", checksum_fgt);
    printf("Float LT: %f\n", checksum_flt);
    
    /* Simple validation - just check that something was computed */
    if (checksum_gt != 0 && checksum_ge != 0 && 
        checksum_lt != 0 && checksum_le != 0) {
        printf("All tests executed successfully.\n");
        return 0;
    } else {
        printf("Error: Some tests may not have executed properly.\n");
        return 1;
    }
}

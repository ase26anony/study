/* test_vectorized_comparisons.c
 * 
 * This program contains loops with comparison operations that should trigger
 * GCC's vectorizer to convert comparison tree codes to bitwise operations.
 * Each function tests one of the four comparison operators: >, >=, <, <=
 * in a vectorizable loop context.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test functions for each comparison operator */

/* GT_EXPR case */
void test_gt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  /* This should trigger GT_EXPR case */
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

/* GE_EXPR case */
void test_ge(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {  /* This should trigger GE_EXPR case */
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] * 3;
        }
    }
}

/* LT_EXPR case */
void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {  /* This should trigger LT_EXPR case */
            c[i] = a[i] << 1;
        } else {
            c[i] = b[i] >> 1;
        }
    }
}

/* LE_EXPR case */
void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {  /* This should trigger LE_EXPR case */
            c[i] = a[i] | b[i];
        } else {
            c[i] = a[i] & b[i];
        }
    }
}

/* Additional test with floating point to ensure different data types */
void test_gt_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  /* GT_EXPR with floats */
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

/* Test with mixed comparisons in same loop */
void test_mixed_comparisons(int *restrict a, int *restrict b, int *restrict c, 
                           int *restrict d, int *restrict e) {
    for (int i = 0; i < N; i++) {
        /* Multiple comparisons to potentially trigger different cases */
        if (a[i] > b[i]) {   /* GT_EXPR */
            c[i] = 1;
        }
        if (a[i] >= b[i]) {  /* GE_EXPR */
            d[i] = 2;
        }
        if (a[i] < b[i]) {   /* LT_EXPR */
            e[i] = 3;
        }
    }
}

/* Helper function to initialize arrays with patterned data */
void init_arrays(int *a, int *b) {
    for (int i = 0; i < N; i++) {
        a[i] = i;           /* 0, 1, 2, 3, ... */
        b[i] = N - i - 1;   /* 1023, 1022, 1021, ... */
    }
}

void init_float_arrays(float *a, float *b) {
    for (int i = 0; i < N; i++) {
        a[i] = (float)i * 1.5f;
        b[i] = (float)(N - i) * 0.7f;
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
    /* Aligned arrays to help vectorization */
    ALIGNED int a[N], b[N], c[N], d[N], e[N], f[N], g[N], h[N];
    ALIGNED int mixed_c[N], mixed_d[N], mixed_e[N];
    ALIGNED float fa[N], fb[N], fc[N];
    
    /* Initialize arrays with patterned data */
    init_arrays(a, b);
    init_float_arrays(fa, fb);
    
    /* Test each comparison operator separately */
    test_gt(a, b, c);
    test_ge(a, b, d);
    test_lt(a, b, e);
    test_le(a, b, f);
    
    /* Test with different array pairs to vary comparison results */
    test_gt(b, a, g);  /* Swapped operands */
    test_ge(b, a, h);  /* Swapped operands */
    
    /* Test floating point comparisons */
    test_gt_float(fa, fb, fc);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(a, b, mixed_c, mixed_d, mixed_e);
    
    /* Compute and print checksums to ensure code executes */
    printf("GT checksum: %d\n", compute_checksum(c));
    printf("GE checksum: %d\n", compute_checksum(d));
    printf("LT checksum: %d\n", compute_checksum(e));
    printf("LE checksum: %d\n", compute_checksum(f));
    printf("GT swapped checksum: %d\n", compute_checksum(g));
    printf("GE swapped checksum: %d\n", compute_checksum(h));
    printf("Float GT checksum: %f\n", compute_checksum_float(fc));
    printf("Mixed C checksum: %d\n", compute_checksum(mixed_c));
    printf("Mixed D checksum: %d\n", compute_checksum(mixed_d));
    printf("Mixed E checksum: %d\n", compute_checksum(mixed_e));
    
    return 0;
}

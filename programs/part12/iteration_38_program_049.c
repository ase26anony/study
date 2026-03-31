/* test_vector_comparisons.c
 * Designed to trigger vectorization of comparison operations
 * to cover lines 12216-12233 in tree-vect-stmts.cc
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test functions for each comparison operator */

void test_gt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            c[i] = a[i] - b[i];
        } else {
            c[i] = 0;
        }
    }
}

void test_ge(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            c[i] = a[i] + b[i];
        } else {
            c[i] = -1;
        }
    }
}

void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            c[i] = b[i] - a[i];
        } else {
            c[i] = 0;
        }
    }
}

void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i];
        }
    }
}

/* Additional tests with floating point to ensure different data types */
void test_gt_float(float *restrict fa, float *restrict fb, float *restrict fc) {
    for (int i = 0; i < N; i++) {
        if (fa[i] > fb[i]) {
            fc[i] = fa[i] - fb[i];
        } else {
            fc[i] = 0.0f;
        }
    }
}

void test_ge_float(float *restrict fa, float *restrict fb, float *restrict fc) {
    for (int i = 0; i < N; i++) {
        if (fa[i] >= fb[i]) {
            fc[i] = fa[i] + fb[i];
        } else {
            fc[i] = -1.0f;
        }
    }
}

/* Complex pattern with multiple comparisons to force mask creation */
void test_mixed_comparisons(int *restrict a, int *restrict b, int *restrict c, 
                           int *restrict d, int *restrict e) {
    for (int i = 0; i < N; i++) {
        /* This creates a more complex boolean expression that should
           still be vectorizable and use mask operations */
        if ((a[i] > b[i]) && (c[i] <= d[i])) {
            e[i] = a[i] + c[i];
        } else if (a[i] < b[i]) {
            e[i] = b[i] - a[i];
        } else {
            e[i] = 0;
        }
    }
}

/* Helper function to initialize arrays with patterned data */
void init_arrays(int *a, int *b, float *fa, float *fb) {
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i - 1;  /* Creates varied comparison results */
        fa[i] = (float)i * 1.5f;
        fb[i] = (float)(N - i) * 0.75f;
    }
}

/* Compute checksum to ensure loops execute and prevent dead code elimination */
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
    /* Use aligned arrays to help vectorization */
    ALIGNED int a[N], b[N], c1[N], c2[N], c3[N], c4[N];
    ALIGNED int d[N], e[N], f[N];
    ALIGNED float fa[N], fb[N], fc1[N], fc2[N];
    
    /* Initialize with patterned data */
    init_arrays(a, b, fa, fb);
    
    /* Initialize additional arrays for mixed test */
    for (int i = 0; i < N; i++) {
        d[i] = i * 2;
        e[i] = i / 2;
        f[i] = 0;
    }
    
    /* Execute all test functions */
    test_gt(a, b, c1);
    test_ge(a, b, c2);
    test_lt(a, b, c3);
    test_le(a, b, c4);
    test_gt_float(fa, fb, fc1);
    test_ge_float(fa, fb, fc2);
    test_mixed_comparisons(a, b, d, e, f);
    
    /* Compute checksums to ensure execution and prevent optimization */
    int checksum = 0;
    checksum += compute_checksum(c1);
    checksum += compute_checksum(c2);
    checksum += compute_checksum(c3);
    checksum += compute_checksum(c4);
    checksum += compute_checksum(f);
    
    float fchecksum = 0.0f;
    fchecksum += compute_checksum_float(fc1);
    fchecksum += compute_checksum_float(fc2);
    
    /* Print results to prevent dead code elimination */
    printf("Integer checksum: %d\n", checksum);
    printf("Float checksum: %f\n", fchecksum);
    
    /* Verify a few values to ensure correctness */
    printf("Sample verification - c1[10] = %d (expected: %d)\n", 
           c1[10], (a[10] > b[10]) ? (a[10] - b[10]) : 0);
    
    return 0;
}

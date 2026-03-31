/* test_vector_comparisons.c
 * Designed to trigger vectorization of comparison operations
 * to cover lines 12216-12233 in tree-vect-stmts.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test functions for each comparison operator */

void test_gt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  /* GT_EXPR */
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

void test_ge(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {  /* GE_EXPR */
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] * 2;
        }
    }
}

void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {  /* LT_EXPR */
            c[i] = a[i] << 1;
        } else {
            c[i] = b[i] << 1;
        }
    }
}

void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {  /* LE_EXPR */
            c[i] = a[i] | b[i];
        } else {
            c[i] = a[i] & b[i];
        }
    }
}

/* Additional tests with floating point to ensure wider coverage */

void test_gt_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  /* GT_EXPR with floats */
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

void test_ge_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {  /* GE_EXPR with floats */
            c[i] = a[i] * 2.0f;
        } else {
            c[i] = b[i] * 2.0f;
        }
    }
}

/* Test with mixed patterns to create varied comparison results */

void test_mixed_patterns(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        /* Use all comparison operators in different contexts */
        int temp = 0;
        if (a[i] > b[i]) temp += 1;    /* GT_EXPR */
        if (a[i] >= b[i]) temp += 2;   /* GE_EXPR */
        if (a[i] < b[i]) temp += 4;    /* LT_EXPR */
        if (a[i] <= b[i]) temp += 8;   /* LE_EXPR */
        c[i] = temp;
    }
}

/* Helper function to initialize arrays with patterns */

void init_arrays(int *a, int *b, float *fa, float *fb) {
    for (int i = 0; i < N; i++) {
        /* Create varying patterns to ensure all comparisons are exercised */
        a[i] = i;
        b[i] = N - i - 1;  /* This creates mixed comparison results */
        fa[i] = (float)i * 1.5f;
        fb[i] = (float)(N - i) * 1.5f;
    }
}

/* Verification function */

int verify_results(int *c1, int *c2, int *c3, int *c4, int *c5) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += c1[i] + c2[i] + c3[i] + c4[i] + c5[i];
    }
    return sum;
}

int main() {
    /* Use aligned arrays for better vectorization */
    ALIGNED int a[N], b[N];
    ALIGNED int c1[N], c2[N], c3[N], c4[N], c5[N];
    ALIGNED float fa[N], fb[N], fc1[N], fc2[N];
    
    /* Initialize with patterns */
    init_arrays(a, b, fa, fb);
    
    /* Execute all test functions */
    test_gt(a, b, c1);        /* Should trigger GT_EXPR case */
    test_ge(a, b, c2);        /* Should trigger GE_EXPR case */
    test_lt(a, b, c3);        /* Should trigger LT_EXPR case */
    test_le(a, b, c4);        /* Should trigger LE_EXPR case */
    test_mixed_patterns(a, b, c5); /* Mixed comparisons */
    
    /* Float tests */
    test_gt_float(fa, fb, fc1);
    test_ge_float(fa, fb, fc2);
    
    /* Verify and use results to prevent dead code elimination */
    int checksum = verify_results(c1, c2, c3, c4, c5);
    
    /* Add float checksum */
    float fsum = 0.0f;
    for (int i = 0; i < N; i++) {
        fsum += fc1[i] + fc2[i];
    }
    
    printf("Integer checksum: %d\n", checksum);
    printf("Float checksum: %f\n", fsum);
    
    /* Return non-zero if any result is suspicious */
    return (checksum == 0 && fsum == 0.0f) ? 1 : 0;
}

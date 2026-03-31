/* test_vectorize_comparisons.c
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
            c[i] = a[i] | b[i];
        } else {
            c[i] = a[i] & b[i];
        }
    }
}

void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {  /* LE_EXPR */
            c[i] = a[i] ^ b[i];
        } else {
            c[i] = a[i] + b[i] * 2;
        }
    }
}

/* Additional tests with floating point to ensure different data types */
void test_gt_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  /* GT_EXPR with floats */
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] + b[i];
        }
    }
}

void test_ge_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {  /* GE_EXPR with floats */
            c[i] = a[i] - b[i];
        } else {
            c[i] = b[i] - a[i];
        }
    }
}

/* Using ternary operator to create mask directly */
void test_lt_ternary(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? a[i] : b[i];  /* LT_EXPR in ternary */
    }
}

void test_le_ternary(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? a[i] + b[i] : a[i] - b[i];  /* LE_EXPR in ternary */
    }
}

/* Complex boolean expression to force mask combination */
void test_complex_condition(int *restrict a, int *restrict b, int *restrict c, 
                           int *restrict d, int *restrict e) {
    for (int i = 0; i < N; i++) {
        /* Combined conditions that may use multiple comparisons */
        if ((a[i] > b[i]) && (c[i] < d[i])) {  /* GT_EXPR and LT_EXPR */
            e[i] = a[i] + c[i];
        } else if (a[i] >= b[i]) {  /* GE_EXPR */
            e[i] = b[i] + d[i];
        } else if (a[i] <= c[i]) {  /* LE_EXPR */
            e[i] = a[i] * d[i];
        } else {
            e[i] = b[i] * c[i];
        }
    }
}

/* Initialize arrays with patterned data to create varied comparison results */
void init_arrays(int *a, int *b, int *c, float *fa, float *fb, float *fc) {
    for (int i = 0; i < N; i++) {
        a[i] = i;                    /* 0, 1, 2, 3, ... */
        b[i] = N - i;                /* 1024, 1023, 1022, ... */
        c[i] = (i % 2 == 0) ? i : -i; /* Mixed positive/negative */
        
        fa[i] = (float)i * 0.5f;
        fb[i] = (float)(N - i) * 0.25f;
        fc[i] = 0.0f;
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
    /* Use aligned arrays for better vectorization */
    ALIGNED int a[N], b[N], c[N], d[N], e[N];
    ALIGNED float fa[N], fb[N], fc[N];
    
    /* Initialize with patterned data */
    init_arrays(a, b, c, fa, fb, fc);
    
    printf("Testing vectorization of comparison operations...\n");
    
    /* Test each comparison operator separately */
    test_gt(a, b, d);
    printf("GT test checksum: %d\n", compute_checksum(d));
    
    test_ge(a, b, d);
    printf("GE test checksum: %d\n", compute_checksum(d));
    
    test_lt(a, b, d);
    printf("LT test checksum: %d\n", compute_checksum(d));
    
    test_le(a, b, d);
    printf("LE test checksum: %d\n", compute_checksum(d));
    
    /* Test floating point comparisons */
    test_gt_float(fa, fb, fc);
    printf("GT float test checksum: %f\n", compute_checksum_float(fc));
    
    test_ge_float(fa, fb, fc);
    printf("GE float test checksum: %f\n", compute_checksum_float(fc));
    
    /* Test ternary operator forms */
    test_lt_ternary(a, b, d);
    printf("LT ternary test checksum: %d\n", compute_checksum(d));
    
    test_le_ternary(a, b, d);
    printf("LE ternary test checksum: %d\n", compute_checksum(d));
    
    /* Test complex condition */
    memset(d, 0, sizeof(d));
    test_complex_condition(a, b, c, d, e);
    printf("Complex condition test checksum: %d\n", compute_checksum(e));
    
    printf("All tests completed.\n");
    
    return 0;
}

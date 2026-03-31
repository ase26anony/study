/* test_vectorize_comparisons.c
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
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

void test_ge(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] * 2;
        }
    }
}

void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            c[i] = a[i] + 1;
        } else {
            c[i] = b[i] + 1;
        }
    }
}

void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            c[i] = a[i] - 1;
        } else {
            c[i] = b[i] - 1;
        }
    }
}

/* Additional tests with floating point to ensure different data types */
void test_gt_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

void test_ge_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            c[i] = a[i] * 2.0f;
        } else {
            c[i] = b[i] * 2.0f;
        }
    }
}

/* Test with ternary operator (another pattern that creates masks) */
void test_lt_ternary(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? a[i] : b[i];
    }
}

void test_le_ternary(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? a[i] + b[i] : a[i] - b[i];
    }
}

/* Complex boolean expression to force mask combination */
void test_complex_bool(int *restrict a, int *restrict b, int *restrict c, int *restrict d) {
    for (int i = 0; i < N; i++) {
        if ((a[i] > b[i]) && (c[i] < d[i])) {
            a[i] = b[i] + c[i];
        }
    }
}

int main() {
    /* Aligned arrays to help vectorization */
    int a_int[N] ALIGNED;
    int b_int[N] ALIGNED;
    int c_int[N] ALIGNED;
    int d_int[N] ALIGNED;
    
    float a_float[N] ALIGNED;
    float b_float[N] ALIGNED;
    float c_float[N] ALIGNED;
    
    /* Initialize with patterned data to create varied comparison results */
    for (int i = 0; i < N; i++) {
        a_int[i] = i;
        b_int[i] = N - i;
        c_int[i] = i % 100;
        d_int[i] = (i + 50) % 100;
        
        a_float[i] = i * 0.5f;
        b_float[i] = (N - i) * 0.5f;
    }
    
    /* Execute all test functions to trigger vectorization of each comparison type */
    test_gt(a_int, b_int, c_int);
    test_ge(a_int, b_int, c_int);
    test_lt(a_int, b_int, c_int);
    test_le(a_int, b_int, c_int);
    
    test_gt_float(a_float, b_float, c_float);
    test_ge_float(a_float, b_float, c_float);
    
    test_lt_ternary(a_int, b_int, c_int);
    test_le_ternary(a_int, b_int, c_int);
    
    test_complex_bool(a_int, b_int, c_int, d_int);
    
    /* Compute checksum to ensure code isn't eliminated and verify correctness */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += c_int[i];
    }
    
    float fchecksum = 0.0f;
    for (int i = 0; i < N; i++) {
        fchecksum += c_float[i];
    }
    
    printf("Integer checksum: %d\n", checksum);
    printf("Float checksum: %f\n", fchecksum);
    
    return 0;
}

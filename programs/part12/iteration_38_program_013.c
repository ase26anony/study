/* test_vectorized_comparisons.c
 * Designed to trigger vectorization of comparison operations
 * to cover lines 12216-12233 in tree-vect-stmts.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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
            c[i] = a[i] << 1;
        } else {
            c[i] = b[i] << 1;
        }
    }
}

void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            c[i] = a[i] | 0x1;
        } else {
            c[i] = b[i] | 0x1;
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

/* Test with ternary operator (alternative pattern) */
void test_lt_ternary(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? a[i] + 100 : b[i] - 100;
    }
}

void test_le_ternary(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? a[i] * 3 : b[i] * 3;
    }
}

/* Initialize arrays with pattern that creates varied comparison results */
void init_arrays(int *a, int *b) {
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i - 1;  /* Creates mix of true/false comparisons */
    }
}

void init_float_arrays(float *a, float *b) {
    for (int i = 0; i < N; i++) {
        a[i] = (float)i * 1.5f;
        b[i] = (float)(N - i) * 1.5f;
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
    /* Aligned arrays for better vectorization */
    ALIGNED int a_int[N], b_int[N], c_int[N];
    ALIGNED float a_float[N], b_float[N], c_float[N];
    
    int total_checksum = 0;
    float total_checksum_float = 0.0f;
    
    /* Initialize arrays */
    init_arrays(a_int, b_int);
    init_float_arrays(a_float, b_float);
    
    /* Test each comparison operator with integer arrays */
    test_gt(a_int, b_int, c_int);
    total_checksum += compute_checksum(c_int);
    
    test_ge(a_int, b_int, c_int);
    total_checksum += compute_checksum(c_int);
    
    test_lt(a_int, b_int, c_int);
    total_checksum += compute_checksum(c_int);
    
    test_le(a_int, b_int, c_int);
    total_checksum += compute_checksum(c_int);
    
    /* Test with ternary operators */
    test_lt_ternary(a_int, b_int, c_int);
    total_checksum += compute_checksum(c_int);
    
    test_le_ternary(a_int, b_int, c_int);
    total_checksum += compute_checksum(c_int);
    
    /* Test with floating point arrays */
    test_gt_float(a_float, b_float, c_float);
    total_checksum_float += compute_checksum_float(c_float);
    
    test_ge_float(a_float, b_float, c_float);
    total_checksum_float += compute_checksum_float(c_float);
    
    /* Print results to prevent optimization */
    printf("Integer checksum: %d\n", total_checksum);
    printf("Float checksum: %f\n", total_checksum_float);
    
    /* Additional test with mixed patterns to ensure all paths are taken */
    ALIGNED int d_int[N], e_int[N], f_int[N];
    for (int i = 0; i < N; i++) {
        d_int[i] = i % 100;
        e_int[i] = (i * 3) % 100;
    }
    
    /* Test all comparisons in one complex loop */
    for (int i = 0; i < N; i++) {
        int val = 0;
        if (d_int[i] > e_int[i]) val += 1;
        if (d_int[i] >= e_int[i]) val += 2;
        if (d_int[i] < e_int[i]) val += 4;
        if (d_int[i] <= e_int[i]) val += 8;
        f_int[i] = val;
    }
    
    int final_checksum = compute_checksum(f_int);
    printf("Final mixed checksum: %d\n", final_checksum);
    
    return 0;
}

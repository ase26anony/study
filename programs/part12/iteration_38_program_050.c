/* test_vectorize_comparisons.c
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
            c[i] = a[i] + 100;
        } else {
            c[i] = b[i] - 100;
        }
    }
}

void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] / (b[i] + 1);  // +1 to avoid division by zero
        }
    }
}

/* Additional tests with floating point for completeness */
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

void test_lt_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            c[i] = a[i] + 10.0f;
        } else {
            c[i] = b[i] - 10.0f;
        }
    }
}

void test_le_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] / (b[i] + 1.0f);
        }
    }
}

/* Helper function to compute checksum */
int compute_checksum_int(int *arr) {
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
    ALIGNED int a_int[N], b_int[N], c_int[N];
    ALIGNED float a_float[N], b_float[N], c_float[N];
    
    /* Initialize with patterned data to create varied comparison results */
    for (int i = 0; i < N; i++) {
        a_int[i] = i;
        b_int[i] = N - i;
        a_float[i] = (float)i * 1.5f;
        b_float[i] = (float)(N - i) * 0.7f;
    }
    
    int checksum = 0;
    float fchecksum = 0.0f;
    
    /* Test all integer comparison operators */
    test_gt(a_int, b_int, c_int);
    checksum += compute_checksum_int(c_int);
    
    test_ge(a_int, b_int, c_int);
    checksum += compute_checksum_int(c_int);
    
    test_lt(a_int, b_int, c_int);
    checksum += compute_checksum_int(c_int);
    
    test_le(a_int, b_int, c_int);
    checksum += compute_checksum_int(c_int);
    
    /* Test all floating point comparison operators */
    test_gt_float(a_float, b_float, c_float);
    fchecksum += compute_checksum_float(c_float);
    
    test_ge_float(a_float, b_float, c_float);
    fchecksum += compute_checksum_float(c_float);
    
    test_lt_float(a_float, b_float, c_float);
    fchecksum += compute_checksum_float(c_float);
    
    test_le_float(a_float, b_float, c_float);
    fchecksum += compute_checksum_float(c_float);
    
    printf("Integer checksum: %d\n", checksum);
    printf("Float checksum: %f\n", fchecksum);
    
    /* Verify results are non-zero to ensure execution */
    if (checksum != 0 && fchecksum != 0.0f) {
        printf("All tests executed successfully.\n");
        return 0;
    } else {
        printf("Error: Unexpected checksum values.\n");
        return 1;
    }
}

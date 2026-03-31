#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test functions for each comparison operator */

void test_gt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  // GT_EXPR
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

void test_ge(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {  // GE_EXPR
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] * 3;
        }
    }
}

void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {  // LT_EXPR
            c[i] = a[i] << 1;
        } else {
            c[i] = b[i] >> 1;
        }
    }
}

void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {  // LE_EXPR
            c[i] = a[i] | b[i];
        } else {
            c[i] = a[i] & b[i];
        }
    }
}

/* Floating point versions to ensure different type handling */
void test_gt_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  // GT_EXPR with floats
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

void test_ge_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {  // GE_EXPR with floats
            c[i] = a[i] * 2.0f;
        } else {
            c[i] = b[i] * 3.0f;
        }
    }
}

/* Alternative pattern using ternary operator */
void test_lt_ternary(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? a[i] + 1 : b[i] - 1;  // LT_EXPR in ternary
    }
}

void test_le_ternary(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? a[i] * b[i] : a[i] / (b[i] + 1);  // LE_EXPR in ternary
    }
}

/* Complex condition with multiple comparisons */
void test_mixed_comparisons(int *restrict a, int *restrict b, int *restrict c, int *restrict d) {
    for (int i = 0; i < N; i++) {
        // This should trigger both GT_EXPR and LT_EXPR
        if (a[i] > b[i] && a[i] < d[i]) {
            c[i] = a[i] + b[i] + d[i];
        } else if (a[i] >= c[i] || b[i] <= d[i]) {
            c[i] = a[i] - b[i];
        } else {
            c[i] = 0;
        }
    }
}

int main() {
    // Aligned arrays to help vectorization
    ALIGNED int a_int[N], b_int[N], c_int[N], d_int[N];
    ALIGNED float a_float[N], b_float[N], c_float[N];
    
    // Initialize with pattern data to create varied comparison results
    for (int i = 0; i < N; i++) {
        a_int[i] = i;
        b_int[i] = N - i;
        d_int[i] = i % 100;
        a_float[i] = (float)i * 1.5f;
        b_float[i] = (float)(N - i) * 0.7f;
    }
    
    // Test all comparison types
    test_gt(a_int, b_int, c_int);
    test_ge(a_int, b_int, c_int);
    test_lt(a_int, b_int, c_int);
    test_le(a_int, b_int, c_int);
    
    test_gt_float(a_float, b_float, c_float);
    test_ge_float(a_float, b_float, c_float);
    
    test_lt_ternary(a_int, b_int, c_int);
    test_le_ternary(a_int, b_int, c_int);
    
    test_mixed_comparisons(a_int, b_int, c_int, d_int);
    
    // Compute checksum to ensure code isn't optimized away
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += c_int[i];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("All comparison tests completed.\n");
    
    return 0;
}

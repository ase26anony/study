#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

// Test functions for each comparison operator
void test_gt(int *restrict a, int *restrict b, int *restrict mask, int n) {
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] > b[i];  // GT_EXPR
    }
}

void test_ge(float *restrict a, float *restrict b, float *restrict x, 
             float *restrict y, float *restrict dst, int n) {
    for (int i = 0; i < n; ++i) {
        // GE_EXPR with conditional blend
        dst[i] = (a[i] >= b[i]) ? x[i] : y[i];
    }
}

int test_lt(unsigned int *restrict a, unsigned int *restrict b, int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        // LT_EXPR with conditional accumulation
        if (a[i] < b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

void test_le(double *restrict a, double *restrict b, int *restrict mask, 
             int *restrict result, int n) {
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] <= b[i];  // LE_EXPR
    }
    
    // Use mask in bitwise operations to encourage transformation
    for (int i = 0; i < n; ++i) {
        result[i] = mask[i] & (int)a[i];
    }
}

void test_gt_float(float *restrict a, float *restrict b, int *restrict mask, int n) {
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] > b[i];  // GT_EXPR with floats
    }
}

void test_ge_double(double *restrict a, double *restrict b, double *restrict x,
                    double *restrict y, double *restrict dst, int n) {
    for (int i = 0; i < n; ++i) {
        dst[i] = (a[i] >= b[i]) ? x[i] : y[i];  // GE_EXPR with doubles
    }
}

int main() {
    // Aligned arrays for different data types
    int a_int[N] ALIGNED;
    int b_int[N] ALIGNED;
    int mask_gt[N] ALIGNED;
    int mask_le[N] ALIGNED;
    int result_le[N] ALIGNED;
    
    float a_float[N] ALIGNED;
    float b_float[N] ALIGNED;
    float x_float[N] ALIGNED;
    float y_float[N] ALIGNED;
    float dst_ge_float[N] ALIGNED;
    int mask_gt_float[N] ALIGNED;
    
    double a_double[N] ALIGNED;
    double b_double[N] ALIGNED;
    double x_double[N] ALIGNED;
    double y_double[N] ALIGNED;
    double dst_ge_double[N] ALIGNED;
    
    unsigned int a_uint[N] ALIGNED;
    unsigned int b_uint[N] ALIGNED;
    
    // Initialize arrays with pattern data
    for (int i = 0; i < N; ++i) {
        a_int[i] = i;
        b_int[i] = N - i;
        a_float[i] = i * 0.5f;
        b_float[i] = (N - i) * 0.5f;
        x_float[i] = i * 1.0f;
        y_float[i] = i * 2.0f;
        a_double[i] = i * 0.25;
        b_double[i] = (N - i) * 0.25;
        x_double[i] = i * 1.0;
        y_double[i] = i * 2.0;
        a_uint[i] = i * 3;
        b_uint[i] = i * 2;
    }
    
    int total_sum = 0;
    
    // Test GT_EXPR with integers
    test_gt(a_int, b_int, mask_gt, N);
    for (int i = 0; i < N; ++i) total_sum += mask_gt[i];
    
    // Test GE_EXPR with floats (conditional blend)
    test_ge(a_float, b_float, x_float, y_float, dst_ge_float, N);
    for (int i = 0; i < N; ++i) total_sum += (int)dst_ge_float[i];
    
    // Test LT_EXPR with unsigned integers (conditional accumulation)
    total_sum += test_lt(a_uint, b_uint, N);
    
    // Test LE_EXPR with doubles
    test_le(a_double, b_double, mask_le, result_le, N);
    for (int i = 0; i < N; ++i) total_sum += result_le[i];
    
    // Additional tests to ensure coverage
    test_gt_float(a_float, b_float, mask_gt_float, N);
    for (int i = 0; i < N; ++i) total_sum += mask_gt_float[i];
    
    test_ge_double(a_double, b_double, x_double, y_double, dst_ge_double, N);
    for (int i = 0; i < N; ++i) total_sum += (int)dst_ge_double[i];
    
    printf("Total sum: %d\n", total_sum);
    
    return 0;
}

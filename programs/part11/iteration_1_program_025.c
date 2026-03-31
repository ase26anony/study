#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void test_lt(unsigned int *restrict a, unsigned int *restrict b, 
             unsigned int *restrict sum_ptr, int n) {
    unsigned int sum = 0;
    for (int i = 0; i < n; ++i) {
        // LT_EXPR with conditional accumulation
        if (a[i] < b[i]) {
            sum += a[i];
        }
    }
    *sum_ptr = sum;
}

void test_le(double *restrict a, double *restrict b, int *restrict mask, 
             int *restrict result, int n) {
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] <= b[i];  // LE_EXPR
    }
    
    // Use mask in bitwise operations to encourage transformation
    for (int i = 0; i < n; ++i) {
        result[i] = mask[i] & (i & 0xFF);
    }
}

// Additional test with mixed types to stress different code paths
void test_mixed_gt_ge(int *restrict a, int *restrict b, 
                      int *restrict mask_gt, int *restrict mask_ge, int n) {
    for (int i = 0; i < n; ++i) {
        mask_gt[i] = a[i] > b[i];   // GT_EXPR
    }
    
    for (int i = 0; i < n; ++i) {
        mask_ge[i] = a[i] >= b[i];  // GE_EXPR
    }
}

int main() {
    // Aligned arrays for different data types
    int a_int[N] ALIGNED;
    int b_int[N] ALIGNED;
    int mask_gt[N] ALIGNED;
    int mask_ge[N] ALIGNED;
    int result_int[N] ALIGNED;
    
    float a_float[N] ALIGNED;
    float b_float[N] ALIGNED;
    float x_float[N] ALIGNED;
    float y_float[N] ALIGNED;
    float dst_float[N] ALIGNED;
    
    unsigned int a_uint[N] ALIGNED;
    unsigned int b_uint[N] ALIGNED;
    unsigned int sum_uint = 0;
    
    double a_double[N] ALIGNED;
    double b_double[N] ALIGNED;
    int mask_le[N] ALIGNED;
    int result_le[N] ALIGNED;
    
    // Initialize arrays with pattern data
    for (int i = 0; i < N; ++i) {
        a_int[i] = i;
        b_int[i] = N - i - 1;
        a_float[i] = i * 0.5f;
        b_float[i] = (N - i) * 0.3f;
        x_float[i] = i * 1.0f;
        y_float[i] = i * 2.0f;
        a_uint[i] = i * 2;
        b_uint[i] = i * 3;
        a_double[i] = i * 0.25;
        b_double[i] = (N - i) * 0.15;
    }
    
    // Call test functions to exercise each comparison operator
    test_gt(a_int, b_int, mask_gt, N);
    test_ge(a_float, b_float, x_float, y_float, dst_float, N);
    test_lt(a_uint, b_uint, &sum_uint, N);
    test_le(a_double, b_double, mask_le, result_le, N);
    test_mixed_gt_ge(a_int, b_int, mask_gt, mask_ge, N);
    
    // Final reduction to prevent dead code elimination
    long long total = 0;
    for (int i = 0; i < N; ++i) {
        total += mask_gt[i] + mask_ge[i] + result_int[i] + 
                 (int)dst_float[i] + result_le[i];
    }
    total += sum_uint;
    
    printf("Result: %lld\n", total);
    
    return 0;
}

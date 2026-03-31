#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

// Test functions for different comparison operators
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
    unsigned int sum = *sum_ptr;
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

void test_mixed_lt_le(float *restrict a, float *restrict b,
                      float *restrict x, float *restrict y,
                      float *restrict dst_lt, float *restrict dst_le, int n) {
    for (int i = 0; i < n; ++i) {
        // LT_EXPR with blend
        dst_lt[i] = (a[i] < b[i]) ? x[i] : y[i];
    }
    
    for (int i = 0; i < n; ++i) {
        // LE_EXPR with blend
        dst_le[i] = (a[i] <= b[i]) ? x[i] : y[i];
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
    float dst_ge[N] ALIGNED;
    float dst_lt[N] ALIGNED;
    float dst_le[N] ALIGNED;
    
    unsigned int a_uint[N] ALIGNED;
    unsigned int b_uint[N] ALIGNED;
    
    double a_double[N] ALIGNED;
    double b_double[N] ALIGNED;
    int mask_le[N] ALIGNED;
    
    // Initialize arrays with patterned data
    for (int i = 0; i < N; ++i) {
        a_int[i] = i;
        b_int[i] = N - i - 1;
        a_float[i] = i * 0.5f;
        b_float[i] = (N - i - 1) * 0.5f;
        x_float[i] = i * 1.0f;
        y_float[i] = i * 2.0f;
        a_uint[i] = i * 3;
        b_uint[i] = i * 2;
        a_double[i] = i * 0.25;
        b_double[i] = (N - i) * 0.25;
    }
    
    unsigned int sum = 0;
    
    // Test GT_EXPR with integers
    test_gt(a_int, b_int, mask_gt, N);
    
    // Test GE_EXPR with floats and conditional blend
    test_ge(a_float, b_float, x_float, y_float, dst_ge, N);
    
    // Test LT_EXPR with unsigned integers and conditional accumulation
    test_lt(a_uint, b_uint, &sum, N);
    
    // Test LE_EXPR with doubles and mask usage
    test_le(a_double, b_double, mask_le, result_int, N);
    
    // Additional mixed tests
    test_mixed_gt_ge(a_int, b_int, mask_gt, mask_ge, N);
    test_mixed_lt_le(a_float, b_float, x_float, y_float, dst_lt, dst_le, N);
    
    // Final reduction to prevent dead code elimination
    int final_sum = 0;
    for (int i = 0; i < N; ++i) {
        final_sum += mask_gt[i] + mask_ge[i] + result_int[i];
        final_sum += (int)dst_ge[i] + (int)dst_lt[i] + (int)dst_le[i];
    }
    final_sum += sum;
    
    printf("Result: %d\n", final_sum);
    
    return 0;
}

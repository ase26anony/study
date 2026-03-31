#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

// Test functions for different comparison operators
void test_gt(int *restrict a, int *restrict b, int *restrict mask, int n) {
    // GT_EXPR: a[i] > b[i]
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] > b[i];
    }
}

void test_ge(float *restrict a, float *restrict b, float *restrict x, 
             float *restrict y, float *restrict dst, int n) {
    // GE_EXPR: a[i] >= b[i] with conditional blend
    for (int i = 0; i < n; ++i) {
        dst[i] = (a[i] >= b[i]) ? x[i] : y[i];
    }
}

long long test_lt(unsigned int *restrict a, unsigned int *restrict b, int n) {
    // LT_EXPR: a[i] < b[i] with conditional accumulation
    long long sum = 0;
    for (int i = 0; i < n; ++i) {
        if (a[i] < b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

void test_le(double *restrict a, double *restrict b, int *restrict mask, 
             int *restrict result, int n) {
    // LE_EXPR: a[i] <= b[i] with mask usage
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] <= b[i];
    }
    
    // Use mask in bitwise operation to encourage transformation
    for (int i = 0; i < n; ++i) {
        result[i] = mask[i] & 0x1;
    }
}

// Additional test with mixed types to stress different paths
void test_mixed_gt_ge(int *restrict a, int *restrict b, 
                      int *restrict mask_gt, int *restrict mask_ge, int n) {
    // Separate loops for GT and GE
    for (int i = 0; i < n; ++i) {
        mask_gt[i] = a[i] > b[i];
    }
    
    for (int i = 0; i < n; ++i) {
        mask_ge[i] = a[i] >= b[i];
    }
}

void test_mixed_lt_le(float *restrict a, float *restrict b,
                      int *restrict mask_lt, int *restrict mask_le, int n) {
    // Separate loops for LT and LE
    for (int i = 0; i < n; ++i) {
        mask_lt[i] = a[i] < b[i];
    }
    
    for (int i = 0; i < n; ++i) {
        mask_le[i] = a[i] <= b[i];
    }
}

int main() {
    // Aligned arrays for different data types
    int a_int[N] ALIGNED;
    int b_int[N] ALIGNED;
    int mask_gt[N] ALIGNED;
    int mask_ge[N] ALIGNED;
    int mask_lt[N] ALIGNED;
    int mask_le[N] ALIGNED;
    int result[N] ALIGNED;
    
    float a_float[N] ALIGNED;
    float b_float[N] ALIGNED;
    float x_float[N] ALIGNED;
    float y_float[N] ALIGNED;
    float dst_float[N] ALIGNED;
    
    double a_double[N] ALIGNED;
    double b_double[N] ALIGNED;
    
    unsigned int a_uint[N] ALIGNED;
    unsigned int b_uint[N] ALIGNED;
    
    // Initialize arrays with pattern data
    for (int i = 0; i < N; ++i) {
        a_int[i] = i;
        b_int[i] = N - i;
        a_float[i] = i * 1.0f;
        b_float[i] = (N - i) * 1.0f;
        x_float[i] = i * 2.0f;
        y_float[i] = i * 3.0f;
        a_double[i] = i * 1.0;
        b_double[i] = (N - i) * 1.0;
        a_uint[i] = i;
        b_uint[i] = N - i;
    }
    
    long long total_sum = 0;
    
    // Test GT_EXPR with integers
    test_gt(a_int, b_int, mask_gt, N);
    for (int i = 0; i < N; ++i) total_sum += mask_gt[i];
    
    // Test GE_EXPR with floats and conditional blend
    test_ge(a_float, b_float, x_float, y_float, dst_float, N);
    for (int i = 0; i < N; ++i) total_sum += (int)dst_float[i];
    
    // Test LT_EXPR with unsigned integers and conditional accumulation
    total_sum += test_lt(a_uint, b_uint, N);
    
    // Test LE_EXPR with doubles and mask usage
    test_le(a_double, b_double, mask_le, result, N);
    for (int i = 0; i < N; ++i) total_sum += result[i];
    
    // Additional mixed tests
    test_mixed_gt_ge(a_int, b_int, mask_gt, mask_ge, N);
    for (int i = 0; i < N; ++i) total_sum += mask_gt[i] + mask_ge[i];
    
    test_mixed_lt_le(a_float, b_float, mask_lt, mask_le, N);
    for (int i = 0; i < N; ++i) total_sum += mask_lt[i] + mask_le[i];
    
    // Prevent dead code elimination
    printf("Total sum: %lld\n", total_sum);
    
    return 0;
}

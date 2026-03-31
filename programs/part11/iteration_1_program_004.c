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

void test_lt(unsigned int *restrict a, unsigned int *restrict b, int n) {
    // LT_EXPR: a[i] < b[i] with conditional accumulation
    unsigned int sum = 0;
    for (int i = 0; i < n; ++i) {
        if (a[i] < b[i]) {
            sum += a[i];
        }
    }
    // Use sum to prevent dead code elimination
    volatile unsigned int dummy = sum;
    (void)dummy;
}

void test_le(double *restrict a, double *restrict b, int *restrict mask, 
             int *restrict result, int n) {
    // LE_EXPR: a[i] <= b[i] with mask usage
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] <= b[i];
    }
    
    // Use mask in bitwise operations to encourage transformation
    for (int i = 0; i < n; ++i) {
        result[i] = mask[i] & (int)a[i];
    }
}

// Additional test with mixed types to cover more cases
void test_gt_mixed(int *restrict a, float *restrict b, int *restrict mask, int n) {
    // Mixed type comparison: a[i] > (int)b[i]
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] > (int)b[i];
    }
}

int main() {
    // Aligned arrays for different data types
    int a_int[N] ALIGNED;
    int b_int[N] ALIGNED;
    int mask_gt[N] ALIGNED;
    int mask_le[N] ALIGNED;
    int result[N] ALIGNED;
    
    float a_float[N] ALIGNED;
    float b_float[N] ALIGNED;
    float x_float[N] ALIGNED;
    float y_float[N] ALIGNED;
    float dst_float[N] ALIGNED;
    
    unsigned int a_uint[N] ALIGNED;
    unsigned int b_uint[N] ALIGNED;
    
    double a_double[N] ALIGNED;
    double b_double[N] ALIGNED;
    
    // Initialize arrays with pattern data
    for (int i = 0; i < N; ++i) {
        a_int[i] = i;
        b_int[i] = N - i;
        a_float[i] = i * 0.5f;
        b_float[i] = (N - i) * 0.5f;
        x_float[i] = i * 1.5f;
        y_float[i] = i * 2.5f;
        a_uint[i] = i * 2;
        b_uint[i] = i * 3;
        a_double[i] = i * 0.25;
        b_double[i] = (N - i) * 0.25;
    }
    
    // Call test functions to exercise different comparison operators
    test_gt(a_int, b_int, mask_gt, N);
    test_ge(a_float, b_float, x_float, y_float, dst_float, N);
    test_lt(a_uint, b_uint, N);
    test_le(a_double, b_double, mask_le, result, N);
    
    // Additional test with mixed types
    test_gt_mixed(a_int, a_float, mask_gt, N);
    
    // Final reduction to prevent dead code elimination
    int final_sum = 0;
    for (int i = 0; i < N; ++i) {
        final_sum += mask_gt[i] + mask_le[i] + result[i] + (int)dst_float[i];
    }
    
    printf("Result: %d\n", final_sum);
    
    return 0;
}

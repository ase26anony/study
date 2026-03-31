#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

// Test functions for each comparison operator
// Each function is in a separate compilation unit context

// GT_EXPR (>)
void test_gt(int *restrict a, int *restrict b, int *restrict mask, int n) {
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] > b[i];
    }
}

// GE_EXPR (>=) with conditional blend
void test_ge(float *restrict a, float *restrict b, float *restrict x, 
             float *restrict y, float *restrict dst, int n) {
    for (int i = 0; i < n; ++i) {
        dst[i] = (a[i] >= b[i]) ? x[i] : y[i];
    }
}

// LT_EXPR (<) with conditional accumulation
int test_lt(unsigned int *restrict a, unsigned int *restrict b, int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        if (a[i] < b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

// LE_EXPR (<=) with mask usage
void test_le(double *restrict a, double *restrict b, int *restrict mask, 
             double *restrict result, int n) {
    // First create mask
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] <= b[i];
    }
    
    // Use mask in computation (encourages mask optimization)
    for (int i = 0; i < n; ++i) {
        result[i] = mask[i] ? (a[i] + b[i]) : (a[i] - b[i]);
    }
}

// Additional test with mixed types to stress different paths
void test_mixed_comparisons(int16_t *restrict a16, int16_t *restrict b16,
                           int32_t *restrict a32, int32_t *restrict b32,
                           int *restrict mask_gt, int *restrict mask_le, int n) {
    // GT_EXPR on 16-bit integers
    for (int i = 0; i < n; ++i) {
        mask_gt[i] = a16[i] > b16[i];
    }
    
    // LE_EXPR on 32-bit integers
    for (int i = 0; i < n; ++i) {
        mask_le[i] = a32[i] <= b32[i];
    }
}

int main() {
    // Aligned arrays for different data types
    int a_int[N] ALIGNED;
    int b_int[N] ALIGNED;
    int mask_gt[N] ALIGNED;
    int mask_le[N] ALIGNED;
    
    float a_float[N] ALIGNED;
    float b_float[N] ALIGNED;
    float x_float[N] ALIGNED;
    float y_float[N] ALIGNED;
    float dst_float[N] ALIGNED;
    
    unsigned int a_uint[N] ALIGNED;
    unsigned int b_uint[N] ALIGNED;
    
    double a_double[N] ALIGNED;
    double b_double[N] ALIGNED;
    double result_double[N] ALIGNED;
    
    int16_t a_int16[N] ALIGNED;
    int16_t b_int16[N] ALIGNED;
    int32_t a_int32[N] ALIGNED;
    int32_t b_int32[N] ALIGNED;
    
    // Initialize arrays with pattern data
    for (int i = 0; i < N; ++i) {
        a_int[i] = i;
        b_int[i] = N - i;
        a_float[i] = i * 0.5f;
        b_float[i] = (N - i) * 0.5f;
        x_float[i] = i * 1.0f;
        y_float[i] = i * 2.0f;
        a_uint[i] = i * 2;
        b_uint[i] = i * 3;
        a_double[i] = i * 0.25;
        b_double[i] = (N - i) * 0.25;
        a_int16[i] = i % 256;
        b_int16[i] = (i + 128) % 256;
        a_int32[i] = i * 10;
        b_int32[i] = i * 15;
    }
    
    int total_sum = 0;
    
    // Test GT_EXPR (>)
    test_gt(a_int, b_int, mask_gt, N);
    for (int i = 0; i < N; ++i) total_sum += mask_gt[i];
    
    // Test GE_EXPR (>=) with conditional blend
    test_ge(a_float, b_float, x_float, y_float, dst_float, N);
    for (int i = 0; i < N; ++i) total_sum += (int)dst_float[i];
    
    // Test LT_EXPR (<) with conditional accumulation
    total_sum += test_lt(a_uint, b_uint, N);
    
    // Test LE_EXPR (<=) with mask usage
    test_le(a_double, b_double, mask_le, result_double, N);
    for (int i = 0; i < N; ++i) total_sum += (int)result_double[i];
    
    // Test mixed comparisons
    test_mixed_comparisons(a_int16, b_int16, a_int32, b_int32, mask_gt, mask_le, N);
    for (int i = 0; i < N; ++i) {
        total_sum += mask_gt[i] + mask_le[i];
    }
    
    // Print result to prevent dead code elimination
    printf("Total sum: %d\n", total_sum);
    
    return 0;
}

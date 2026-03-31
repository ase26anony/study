#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

// Test functions for different comparison operators
// Each function tests a specific comparison operator in a vectorizable loop

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
             int *restrict result, int n) {
    // First generate mask
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] <= b[i];
    }
    
    // Use mask in bitwise operation (encourages mask transformation)
    for (int i = 0; i < n; ++i) {
        result[i] = mask[i] & 0x1;
    }
}

// Additional test with mixed types for comprehensive coverage
void test_mixed_comparisons(int *restrict a, int *restrict b, 
                           int *restrict results, int n) {
    // Separate loops for each operator to ensure vectorization
    for (int i = 0; i < n; ++i) {
        results[i] = a[i] > b[i];  // GT_EXPR
    }
    
    for (int i = 0; i < n; ++i) {
        results[i] |= (a[i] >= b[i]) << 1;  // GE_EXPR
    }
    
    for (int i = 0; i < n; ++i) {
        results[i] |= (a[i] < b[i]) << 2;   // LT_EXPR
    }
    
    for (int i = 0; i < n; ++i) {
        results[i] |= (a[i] <= b[i]) << 3;  // LE_EXPR
    }
}

int main() {
    // Aligned arrays for different data types
    int a_int[N] ALIGNED;
    int b_int[N] ALIGNED;
    int mask_int[N] ALIGNED;
    int results[N] ALIGNED;
    
    float a_float[N] ALIGNED;
    float b_float[N] ALIGNED;
    float x_float[N] ALIGNED;
    float y_float[N] ALIGNED;
    float dst_float[N] ALIGNED;
    
    unsigned int a_uint[N] ALIGNED;
    unsigned int b_uint[N] ALIGNED;
    
    double a_double[N] ALIGNED;
    double b_double[N] ALIGNED;
    int mask_double[N] ALIGNED;
    int result_double[N] ALIGNED;
    
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
    }
    
    int total_sum = 0;
    
    // Test GT_EXPR (>)
    test_gt(a_int, b_int, mask_int, N);
    for (int i = 0; i < N; ++i) total_sum += mask_int[i];
    
    // Test GE_EXPR (>=) with conditional blend
    test_ge(a_float, b_float, x_float, y_float, dst_float, N);
    for (int i = 0; i < N; ++i) total_sum += (int)dst_float[i];
    
    // Test LT_EXPR (<) with conditional accumulation
    total_sum += test_lt(a_uint, b_uint, N);
    
    // Test LE_EXPR (<=) with mask usage
    test_le(a_double, b_double, mask_double, result_double, N);
    for (int i = 0; i < N; ++i) total_sum += result_double[i];
    
    // Test all comparisons in separate loops
    test_mixed_comparisons(a_int, b_int, results, N);
    for (int i = 0; i < N; ++i) total_sum += results[i];
    
    // Print result to prevent dead code elimination
    printf("Total sum: %d\n", total_sum);
    
    return 0;
}

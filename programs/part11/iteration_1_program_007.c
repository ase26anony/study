#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

// Test functions for different comparison operators
// Each function tests a specific comparison operator in a vectorizable loop

// GT_EXPR: greater than
void test_gt(int *restrict a, int *restrict b, int *restrict mask, int n) {
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] > b[i];  // GT_EXPR
    }
}

// GE_EXPR: greater than or equal
void test_ge(float *restrict a, float *restrict b, float *restrict x, 
             float *restrict y, float *restrict dst, int n) {
    for (int i = 0; i < n; ++i) {
        // Conditional blend using GE_EXPR
        dst[i] = (a[i] >= b[i]) ? x[i] : y[i];  // GE_EXPR
    }
}

// LT_EXPR: less than
int test_lt(unsigned int *restrict a, unsigned int *restrict b, int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        if (a[i] < b[i]) {  // LT_EXPR
            sum += a[i];
        }
    }
    return sum;
}

// LE_EXPR: less than or equal
void test_le(double *restrict a, double *restrict b, int *restrict mask, 
             int *restrict result, int n) {
    // First generate mask
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] <= b[i];  // LE_EXPR
    }
    
    // Then use mask in bitwise operation to ensure transformation
    for (int i = 0; i < n; ++i) {
        result[i] = mask[i] & (int)a[i];  // Use mask in bitwise operation
    }
}

// Additional test with mixed types to stress different code paths
void test_mixed_comparisons(int *restrict a, int *restrict b, 
                           float *restrict fa, float *restrict fb,
                           int *restrict results, int n) {
    // Multiple independent comparisons in separate loops
    for (int i = 0; i < n; ++i) {
        results[i] = a[i] > b[i];  // GT_EXPR
    }
    
    for (int i = 0; i < n; ++i) {
        results[i] |= (fa[i] >= fb[i]) ? 1 : 0;  // GE_EXPR
    }
    
    for (int i = 0; i < n; ++i) {
        results[i] |= (a[i] < b[i]) ? 2 : 0;  // LT_EXPR
    }
    
    for (int i = 0; i < n; ++i) {
        results[i] |= (fa[i] <= fb[i]) ? 4 : 0;  // LE_EXPR
    }
}

int main() {
    // Aligned arrays for different data types
    int a_int[N] ALIGNED;
    int b_int[N] ALIGNED;
    int mask_gt[N] ALIGNED;
    int mask_le[N] ALIGNED;
    int results[N] ALIGNED;
    
    float a_float[N] ALIGNED;
    float b_float[N] ALIGNED;
    float x_float[N] ALIGNED;
    float y_float[N] ALIGNED;
    float dst_ge[N] ALIGNED;
    
    unsigned int a_uint[N] ALIGNED;
    unsigned int b_uint[N] ALIGNED;
    
    double a_double[N] ALIGNED;
    double b_double[N] ALIGNED;
    
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
        b_double[i] = (N - i - 1) * 0.25;
    }
    
    int total_sum = 0;
    
    // Test GT_EXPR with integers
    test_gt(a_int, b_int, mask_gt, N);
    for (int i = 0; i < N; ++i) total_sum += mask_gt[i];
    
    // Test GE_EXPR with floats (conditional blend)
    test_ge(a_float, b_float, x_float, y_float, dst_ge, N);
    for (int i = 0; i < N; ++i) total_sum += (int)dst_ge[i];
    
    // Test LT_EXPR with unsigned integers (conditional accumulation)
    total_sum += test_lt(a_uint, b_uint, N);
    
    // Test LE_EXPR with doubles
    test_le(a_double, b_double, mask_le, results, N);
    for (int i = 0; i < N; ++i) total_sum += results[i];
    
    // Test mixed comparisons
    test_mixed_comparisons(a_int, b_int, a_float, b_float, results, N);
    for (int i = 0; i < N; ++i) total_sum += results[i];
    
    // Print result to prevent dead code elimination
    printf("Total sum: %d\n", total_sum);
    
    return 0;
}

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

// Additional test with mixed types
void test_mixed_gt_ge(int *restrict a, int *restrict b, 
                      float *restrict fa, float *restrict fb,
                      int *restrict mask1, int *restrict mask2, int n) {
    // GT_EXPR with integers
    for (int i = 0; i < n; ++i) {
        mask1[i] = a[i] > b[i];
    }
    
    // GE_EXPR with floats
    for (int i = 0; i < n; ++i) {
        mask2[i] = fa[i] >= fb[i];
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
    float dst_ge[N] ALIGNED;
    
    unsigned int a_uint[N] ALIGNED;
    unsigned int b_uint[N] ALIGNED;
    
    double a_double[N] ALIGNED;
    double b_double[N] ALIGNED;
    
    int mask_mixed1[N] ALIGNED;
    int mask_mixed2[N] ALIGNED;
    
    // Initialize arrays with pattern data
    for (int i = 0; i < N; ++i) {
        a_int[i] = i;
        b_int[i] = N - i;
        a_float[i] = i * 0.5f;
        b_float[i] = (N - i) * 0.5f;
        x_float[i] = i * 1.5f;
        y_float[i] = i * 2.5f;
        a_uint[i] = i * 3;
        b_uint[i] = i * 2;
        a_double[i] = i * 0.25;
        b_double[i] = (N - i) * 0.25;
    }
    
    unsigned int sum = 0;
    
    // Execute all test functions
    test_gt(a_int, b_int, mask_gt, N);
    test_ge(a_float, b_float, x_float, y_float, dst_ge, N);
    test_lt(a_uint, b_uint, &sum, N);
    test_le(a_double, b_double, mask_le, result, N);
    test_mixed_gt_ge(a_int, b_int, a_float, b_float, mask_mixed1, mask_mixed2, N);
    
    // Final reduction to prevent dead code elimination
    long long total = 0;
    for (int i = 0; i < N; ++i) {
        total += mask_gt[i] + (int)dst_ge[i] + mask_le[i] + result[i] 
                 + mask_mixed1[i] + mask_mixed2[i];
    }
    total += sum;
    
    printf("Result: %lld\n", total);
    
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

// Test functions for each comparison operator
// Each function is defined separately to ensure they're analyzed independently

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

// LE_EXPR (<=) with mask usage in bitwise operations
void test_le(double *restrict a, double *restrict b, int *restrict mask, 
             int *restrict result, int n) {
    // First create the mask
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] <= b[i];
    }
    
    // Then use it in a way that might trigger bitwise transformations
    for (int i = 0; i < n; ++i) {
        // This pattern might encourage mask to bitwise transformation
        result[i] = mask[i] & (int)a[i];
    }
}

// Additional test with mixed types to cover different code paths
void test_mixed_gt(int *restrict a, short *restrict b, int *restrict mask, int n) {
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] > (int)b[i];
    }
}

void test_mixed_le(float *restrict a, int *restrict b, int *restrict mask, int n) {
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] <= (float)b[i];
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
    
    double a_double[N] ALIGNED;
    double b_double[N] ALIGNED;
    
    unsigned int a_uint[N] ALIGNED;
    unsigned int b_uint[N] ALIGNED;
    
    short b_short[N] ALIGNED;
    
    // Initialize arrays with pattern data
    for (int i = 0; i < N; ++i) {
        a_int[i] = i;
        b_int[i] = N - i;
        a_float[i] = i * 0.5f;
        b_float[i] = (N - i) * 0.5f;
        x_float[i] = i * 1.5f;
        y_float[i] = i * 2.5f;
        a_double[i] = i * 0.25;
        b_double[i] = (N - i) * 0.25;
        a_uint[i] = i * 2;
        b_uint[i] = i * 3;
        b_short[i] = (short)(i % 256);
    }
    
    int total_sum = 0;
    
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
    
    // Additional mixed-type tests
    test_mixed_gt(a_int, b_short, mask_gt, N);
    for (int i = 0; i < N; ++i) total_sum += mask_gt[i];
    
    test_mixed_le(a_float, a_int, mask_le, N);
    for (int i = 0; i < N; ++i) total_sum += mask_le[i];
    
    // Print result to prevent dead code elimination
    printf("Total sum: %d\n", total_sum);
    
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

// Test functions for each comparison operator
__attribute__((noinline))
void test_gt(int *restrict a, int *restrict b, int *restrict mask, int n) {
    // GT_EXPR: a[i] > b[i]
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] > b[i];
    }
}

__attribute__((noinline))
void test_ge(float *restrict a, float *restrict b, float *restrict dst, 
             float *restrict x, float *restrict y, int n) {
    // GE_EXPR: a[i] >= b[i] with conditional blend
    for (int i = 0; i < n; ++i) {
        dst[i] = (a[i] >= b[i]) ? x[i] : y[i];
    }
}

__attribute__((noinline))
void test_lt(unsigned int *restrict a, unsigned int *restrict b, 
             unsigned int *restrict sum_ptr, int n) {
    // LT_EXPR: a[i] < b[i] with conditional accumulation
    unsigned int sum = 0;
    for (int i = 0; i < n; ++i) {
        if (a[i] < b[i]) {
            sum += a[i];
        }
    }
    *sum_ptr = sum;
}

__attribute__((noinline))
void test_le(double *restrict a, double *restrict b, 
             int *restrict mask, int *restrict result, int n) {
    // LE_EXPR: a[i] <= b[i] with mask usage
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] <= b[i];
    }
    
    // Use mask in bitwise operation to encourage transformation
    for (int i = 0; i < n; ++i) {
        result[i] = mask[i] & 0x1;
    }
}

__attribute__((noinline))
void test_gt_float(float *restrict a, float *restrict b, 
                   float *restrict dst, int n) {
    // Additional GT_EXPR with floats for floating-point comparison
    for (int i = 0; i < n; ++i) {
        dst[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
}

__attribute__((noinline))
void test_ge_int(int *restrict a, int *restrict b, 
                 int *restrict mask, int n) {
    // Additional GE_EXPR with integers
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] >= b[i];
    }
}

int main() {
    // Aligned arrays for different data types
    int a_int[N] ALIGNED;
    int b_int[N] ALIGNED;
    int mask_gt[N] ALIGNED;
    int mask_ge[N] ALIGNED;
    int result_le[N] ALIGNED;
    
    float a_float[N] ALIGNED;
    float b_float[N] ALIGNED;
    float x_float[N] ALIGNED;
    float y_float[N] ALIGNED;
    float dst_float[N] ALIGNED;
    float dst_gt_float[N] ALIGNED;
    
    unsigned int a_uint[N] ALIGNED;
    unsigned int b_uint[N] ALIGNED;
    unsigned int sum_lt = 0;
    
    double a_double[N] ALIGNED;
    double b_double[N] ALIGNED;
    int mask_le[N] ALIGNED;
    
    // Initialize arrays with pattern data
    for (int i = 0; i < N; ++i) {
        // Integer arrays: alternating patterns
        a_int[i] = i;
        b_int[i] = (i % 3 == 0) ? i + 1 : i - 1;
        
        // Float arrays: sine-like pattern
        a_float[i] = i * 0.1f;
        b_float[i] = (i % 2 == 0) ? i * 0.1f + 0.5f : i * 0.1f - 0.5f;
        x_float[i] = i * 0.2f;
        y_float[i] = i * 0.3f;
        
        // Unsigned arrays: different ranges
        a_uint[i] = i * 2;
        b_uint[i] = i * 3;
        
        // Double arrays: similar to floats but with different scale
        a_double[i] = i * 0.05;
        b_double[i] = (i % 4 == 0) ? i * 0.05 + 1.0 : i * 0.05 - 1.0;
    }
    
    // Call test functions for each comparison operator
    test_gt(a_int, b_int, mask_gt, N);
    test_ge(a_float, b_float, dst_float, x_float, y_float, N);
    test_lt(a_uint, b_uint, &sum_lt, N);
    test_le(a_double, b_double, mask_le, result_le, N);
    
    // Additional tests to ensure coverage
    test_gt_float(a_float, b_float, dst_gt_float, N);
    test_ge_int(a_int, b_int, mask_ge, N);
    
    // Final reduction to prevent dead code elimination
    int final_sum = 0;
    for (int i = 0; i < N; ++i) {
        final_sum += mask_gt[i] + mask_ge[i] + result_le[i];
        final_sum += (int)dst_float[i] + (int)dst_gt_float[i];
    }
    final_sum += sum_lt;
    
    printf("Result: %d\n", final_sum);
    
    return 0;
}

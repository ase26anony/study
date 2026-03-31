#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

// Test functions for each comparison operator
__attribute__((noinline))
void test_gt(int *restrict a, int *restrict b, int *restrict mask, int n) {
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] > b[i];  // GT_EXPR
    }
}

__attribute__((noinline))
void test_ge(float *restrict a, float *restrict b, float *restrict dst, 
             float *restrict x, float *restrict y, int n) {
    for (int i = 0; i < n; ++i) {
        // GE_EXPR with conditional blend
        dst[i] = (a[i] >= b[i]) ? x[i] : y[i];
    }
}

__attribute__((noinline))
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

__attribute__((noinline))
void test_le(double *restrict a, double *restrict b, int *restrict mask, 
             int *restrict processed, int n) {
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] <= b[i];  // LE_EXPR
    }
    
    // Use mask in bitwise operations to encourage transformation
    for (int i = 0; i < n; ++i) {
        processed[i] = mask[i] & 0x1;
    }
}

__attribute__((noinline))
void test_gt_float(float *restrict a, float *restrict b, int *restrict mask, int n) {
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] > b[i];  // GT_EXPR with floats
    }
}

__attribute__((noinline))
void test_ge_int(int *restrict a, int *restrict b, int *restrict dst, 
                 int *restrict x, int *restrict y, int n) {
    for (int i = 0; i < n; ++i) {
        // GE_EXPR with integers, different pattern
        dst[i] = (a[i] >= b[i]) ? (x[i] | y[i]) : (x[i] & y[i]);
    }
}

int main() {
    // Aligned arrays for different data types
    int a_int[N] ALIGNED;
    int b_int[N] ALIGNED;
    int mask_gt[N] ALIGNED;
    int mask_le[N] ALIGNED;
    int processed[N] ALIGNED;
    int x_int[N] ALIGNED;
    int y_int[N] ALIGNED;
    int dst_int[N] ALIGNED;
    
    float a_float[N] ALIGNED;
    float b_float[N] ALIGNED;
    float x_float[N] ALIGNED;
    float y_float[N] ALIGNED;
    float dst_float[N] ALIGNED;
    int mask_float[N] ALIGNED;
    
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
        a_uint[i] = i * 2;
        b_uint[i] = i * 3;
        a_double[i] = i * 0.25;
        b_double[i] = (N - i) * 0.25;
        
        x_int[i] = i * 3;
        y_int[i] = i * 5;
        x_float[i] = i * 1.5f;
        y_float[i] = i * 2.5f;
    }
    
    unsigned int sum_lt = 0;
    int final_sum = 0;
    
    // Test GT_EXPR with integers
    test_gt(a_int, b_int, mask_gt, N);
    
    // Test GE_EXPR with floats and conditional blend
    test_ge(a_float, b_float, dst_float, x_float, y_float, N);
    
    // Test LT_EXPR with unsigned integers and conditional accumulation
    test_lt(a_uint, b_uint, &sum_lt, N);
    
    // Test LE_EXPR with doubles and mask processing
    test_le(a_double, b_double, mask_le, processed, N);
    
    // Additional tests for more coverage
    test_gt_float(a_float, b_float, mask_float, N);
    test_ge_int(a_int, b_int, dst_int, x_int, y_int, N);
    
    // Prevent dead code elimination by using results
    for (int i = 0; i < N; ++i) {
        final_sum += mask_gt[i] + processed[i] + mask_float[i] + dst_int[i];
        final_sum += (int)dst_float[i];
    }
    final_sum += sum_lt;
    
    printf("Final sum: %d\n", final_sum);
    
    return 0;
}

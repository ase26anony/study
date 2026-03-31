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
    unsigned int sum = 0;
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
        result[i] = mask[i] & (int)a[i];
    }
}

void test_gt_float(float *restrict a, float *restrict b, int *restrict mask, int n) {
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] > b[i];  // GT_EXPR with floats
    }
}

void test_ge_double(double *restrict a, double *restrict b, double *restrict dst, int n) {
    for (int i = 0; i < n; ++i) {
        // GE_EXPR with double and blend
        dst[i] = (a[i] >= b[i]) ? a[i] : b[i];
    }
}

int main() {
    // Aligned arrays for different data types
    int a_int[N] ALIGNED;
    int b_int[N] ALIGNED;
    int mask_gt[N] ALIGNED;
    int mask_le[N] ALIGNED;
    int result_le[N] ALIGNED;
    
    float a_float[N] ALIGNED;
    float b_float[N] ALIGNED;
    float x_float[N] ALIGNED;
    float y_float[N] ALIGNED;
    float dst_ge[N] ALIGNED;
    int mask_gt_float[N] ALIGNED;
    
    double a_double[N] ALIGNED;
    double b_double[N] ALIGNED;
    double dst_ge_double[N] ALIGNED;
    
    unsigned int a_uint[N] ALIGNED;
    unsigned int b_uint[N] ALIGNED;
    unsigned int sum_lt = 0;
    
    // Initialize arrays with pattern data
    for (int i = 0; i < N; ++i) {
        a_int[i] = i;
        b_int[i] = N - i;
        
        a_float[i] = i * 1.5f;
        b_float[i] = (N - i) * 1.5f;
        x_float[i] = i * 2.0f;
        y_float[i] = i * 3.0f;
        
        a_double[i] = i * 1.25;
        b_double[i] = (N - i) * 1.25;
        
        a_uint[i] = i * 2;
        b_uint[i] = i * 3;
    }
    
    // Call test functions to exercise different comparison operators
    test_gt(a_int, b_int, mask_gt, N);                     // GT_EXPR with int
    test_ge(a_float, b_float, x_float, y_float, dst_ge, N); // GE_EXPR with float blend
    test_lt(a_uint, b_uint, &sum_lt, N);                   // LT_EXPR with unsigned int
    test_le(a_double, b_double, mask_le, result_le, N);    // LE_EXPR with double
    
    // Additional tests for floating-point comparisons
    test_gt_float(a_float, b_float, mask_gt_float, N);     // GT_EXPR with float
    test_ge_double(a_double, b_double, dst_ge_double, N);  // GE_EXPR with double
    
    // Prevent dead code elimination by computing and printing a checksum
    int checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += mask_gt[i] + (int)dst_ge[i] + result_le[i] + 
                   mask_gt_float[i] + (int)dst_ge_double[i];
    }
    checksum += sum_lt;
    
    printf("Checksum: %d\n", checksum);
    printf("GT mask[0]: %d, GE dst[0]: %.2f, LT sum: %u, LE result[0]: %d\n",
           mask_gt[0], dst_ge[0], sum_lt, result_le[0]);
    
    return 0;
}

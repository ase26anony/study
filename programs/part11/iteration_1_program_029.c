#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

// Test functions for each comparison operator
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
    
    // Use mask in bitwise operation to encourage transformation
    for (int i = 0; i < n; ++i) {
        result[i] = mask[i] & 0x1;
    }
}

// Additional test with mixed types
void test_mixed_gt_ge(int *restrict a, int *restrict b, 
                      int *restrict mask_gt, int *restrict mask_ge, int n) {
    for (int i = 0; i < n; ++i) {
        mask_gt[i] = a[i] > b[i];   // GT_EXPR
    }
    
    for (int i = 0; i < n; ++i) {
        mask_ge[i] = a[i] >= b[i];  // GE_EXPR
    }
}

// Test with loop-carried dependency that should still vectorize
void test_le_with_reduction(double *restrict a, double *restrict b, 
                           double *restrict sum_ptr, int n) {
    double sum = *sum_ptr;
    for (int i = 0; i < n; ++i) {
        if (a[i] <= b[i]) {  // LE_EXPR
            sum += a[i] * b[i];
        }
    }
    *sum_ptr = sum;
}

int main() {
    // Aligned arrays for different data types
    int a_int[N] ALIGNED;
    int b_int[N] ALIGNED;
    int mask_gt[N] ALIGNED;
    int mask_ge[N] ALIGNED;
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
    
    // Initialize arrays with patterned data
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
    
    unsigned int uint_sum = 0;
    double double_sum = 0.0;
    
    // Execute all test functions
    test_gt(a_int, b_int, mask_gt, N);
    test_ge(a_float, b_float, x_float, y_float, dst_float, N);
    test_lt(a_uint, b_uint, &uint_sum, N);
    test_le(a_double, b_double, mask_gt, result, N);
    test_mixed_gt_ge(a_int, b_int, mask_gt, mask_ge, N);
    test_le_with_reduction(a_double, b_double, &double_sum, N);
    
    // Prevent dead code elimination by computing and printing a checksum
    int checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += mask_gt[i] + mask_ge[i] + result[i];
        checksum += (int)dst_float[i];
    }
    checksum += uint_sum + (int)double_sum;
    
    printf("Checksum: %d\n", checksum);
    printf("uint_sum: %u, double_sum: %f\n", uint_sum, double_sum);
    
    return 0;
}

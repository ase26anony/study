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

int test_lt(unsigned int *restrict a, unsigned int *restrict b, int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        // LT_EXPR with conditional accumulation
        if (a[i] < b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

void test_le(double *restrict a, double *restrict b, int *restrict mask, 
             int *restrict result, int n) {
    // LE_EXPR with mask generation and usage
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] <= b[i];  // LE_EXPR
    }
    
    // Use mask in bitwise operation to encourage transformation
    for (int i = 0; i < n; ++i) {
        result[i] = mask[i] & 0x1;  // Ensure mask is used
    }
}

// Additional test with mixed types to stress different paths
void test_mixed_gt_ge(int *restrict a, int *restrict b, 
                      int *restrict gt_mask, int *restrict ge_mask, int n) {
    // Separate loops for each operator
    for (int i = 0; i < n; ++i) {
        gt_mask[i] = a[i] > b[i];   // GT_EXPR
    }
    
    for (int i = 0; i < n; ++i) {
        ge_mask[i] = a[i] >= b[i];  // GE_EXPR
    }
}

void test_mixed_lt_le(float *restrict a, float *restrict b,
                      float *restrict lt_dst, float *restrict le_dst,
                      float *restrict src1, float *restrict src2, int n) {
    // LT_EXPR with blend
    for (int i = 0; i < n; ++i) {
        lt_dst[i] = (a[i] < b[i]) ? src1[i] : src2[i];
    }
    
    // LE_EXPR with blend
    for (int i = 0; i < n; ++i) {
        le_dst[i] = (a[i] <= b[i]) ? src1[i] : src2[i];
    }
}

int main() {
    // Aligned arrays for different data types
    int a_int[N] ALIGNED;
    int b_int[N] ALIGNED;
    int mask_gt[N] ALIGNED;
    int mask_ge[N] ALIGNED;
    int result_int[N] ALIGNED;
    
    float a_float[N] ALIGNED;
    float b_float[N] ALIGNED;
    float x_float[N] ALIGNED;
    float y_float[N] ALIGNED;
    float dst_float[N] ALIGNED;
    float lt_dst[N] ALIGNED;
    float le_dst[N] ALIGNED;
    float src1[N] ALIGNED;
    float src2[N] ALIGNED;
    
    double a_double[N] ALIGNED;
    double b_double[N] ALIGNED;
    int mask_le[N] ALIGNED;
    
    unsigned int a_uint[N] ALIGNED;
    unsigned int b_uint[N] ALIGNED;
    
    // Initialize arrays with pattern to ensure both true and false comparisons
    for (int i = 0; i < N; ++i) {
        a_int[i] = i;
        b_int[i] = i % 3;
        
        a_float[i] = i * 0.5f;
        b_float[i] = (i % 5) * 0.3f;
        x_float[i] = i * 1.1f;
        y_float[i] = i * 0.9f;
        src1[i] = i * 2.0f;
        src2[i] = i * 1.5f;
        
        a_double[i] = i * 0.25;
        b_double[i] = (i % 7) * 0.15;
        
        a_uint[i] = i * 2;
        b_uint[i] = i;
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
    
    // Test LE_EXPR with doubles
    test_le(a_double, b_double, mask_le, result_int, N);
    for (int i = 0; i < N; ++i) total_sum += result_int[i];
    
    // Test mixed GT/GE with integers
    test_mixed_gt_ge(a_int, b_int, mask_gt, mask_ge, N);
    for (int i = 0; i < N; ++i) total_sum += mask_gt[i] + mask_ge[i];
    
    // Test mixed LT/LE with floats
    test_mixed_lt_le(a_float, b_float, lt_dst, le_dst, src1, src2, N);
    for (int i = 0; i < N; ++i) total_sum += (int)lt_dst[i] + (int)le_dst[i];
    
    printf("Total sum: %d\n", total_sum);
    return 0;
}

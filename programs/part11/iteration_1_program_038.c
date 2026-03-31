#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

// Test functions for different comparison operators
// Each function uses a different comparison to trigger different code paths

// GT_EXPR (>)
void test_gt(int *restrict a, int *restrict b, int *restrict mask, int n) {
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] > b[i];  // GT_EXPR
    }
}

// GE_EXPR (>=) with conditional blend
void test_ge(float *restrict a, float *restrict b, float *restrict x, 
             float *restrict y, float *restrict dst, int n) {
    for (int i = 0; i < n; ++i) {
        // GE_EXPR with conditional blend (encourages mask optimization)
        dst[i] = (a[i] >= b[i]) ? x[i] : y[i];
    }
}

// LT_EXPR (<) with conditional accumulation
int test_lt(unsigned int *restrict a, unsigned int *restrict b, int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        if (a[i] < b[i]) {  // LT_EXPR
            sum += a[i];
        }
    }
    return sum;
}

// LE_EXPR (<=) with mask usage
void test_le(double *restrict a, double *restrict b, int *restrict mask, 
             int *restrict result, int n) {
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] <= b[i];  // LE_EXPR
    }
    
    // Use the mask in bitwise operations to encourage the transformation
    for (int i = 0; i < n; ++i) {
        result[i] = mask[i] & (int)a[i];
    }
}

// Additional test with mixed types to cover more cases
void test_mixed_comparisons(short *restrict a, short *restrict b, 
                            int *restrict masks, int n) {
    // Test all four comparisons in separate loops
    for (int i = 0; i < n; ++i) {
        masks[i] = a[i] > b[i];  // GT_EXPR
    }
    
    for (int i = 0; i < n; ++i) {
        masks[n + i] = a[i] >= b[i];  // GE_EXPR
    }
    
    for (int i = 0; i < n; ++i) {
        masks[2*n + i] = a[i] < b[i];  // LT_EXPR
    }
    
    for (int i = 0; i < n; ++i) {
        masks[3*n + i] = a[i] <= b[i];  // LE_EXPR
    }
}

int main() {
    // Aligned arrays for different data types
    int a_int[N] ALIGNED;
    int b_int[N] ALIGNED;
    int mask_gt[N] ALIGNED;
    
    float a_float[N] ALIGNED;
    float b_float[N] ALIGNED;
    float x_float[N] ALIGNED;
    float y_float[N] ALIGNED;
    float dst_ge[N] ALIGNED;
    
    unsigned int a_uint[N] ALIGNED;
    unsigned int b_uint[N] ALIGNED;
    
    double a_double[N] ALIGNED;
    double b_double[N] ALIGNED;
    int mask_le[N] ALIGNED;
    int result_le[N] ALIGNED;
    
    short a_short[N] ALIGNED;
    short b_short[N] ALIGNED;
    int masks_mixed[4 * N] ALIGNED;
    
    // Initialize arrays with pattern data
    for (int i = 0; i < N; ++i) {
        a_int[i] = i;
        b_int[i] = N - i;
        
        a_float[i] = i * 1.5f;
        b_float[i] = (N - i) * 1.0f;
        x_float[i] = i * 2.0f;
        y_float[i] = i * 0.5f;
        
        a_uint[i] = i * 2;
        b_uint[i] = i * 3;
        
        a_double[i] = i * 1.25;
        b_double[i] = i * 1.75;
        
        a_short[i] = i % 100;
        b_short[i] = (i + 50) % 100;
    }
    
    int total_sum = 0;
    
    // Test GT_EXPR (>)
    test_gt(a_int, b_int, mask_gt, N);
    for (int i = 0; i < N; ++i) total_sum += mask_gt[i];
    
    // Test GE_EXPR (>=) with conditional blend
    test_ge(a_float, b_float, x_float, y_float, dst_ge, N);
    for (int i = 0; i < N; ++i) total_sum += (int)dst_ge[i];
    
    // Test LT_EXPR (<) with conditional accumulation
    total_sum += test_lt(a_uint, b_uint, N);
    
    // Test LE_EXPR (<=) with mask usage
    test_le(a_double, b_double, mask_le, result_le, N);
    for (int i = 0; i < N; ++i) total_sum += result_le[i];
    
    // Test mixed comparisons
    test_mixed_comparisons(a_short, b_short, masks_mixed, N);
    for (int i = 0; i < 4 * N; ++i) total_sum += masks_mixed[i];
    
    // Print result to prevent dead code elimination
    printf("Total sum: %d\n", total_sum);
    
    return 0;
}

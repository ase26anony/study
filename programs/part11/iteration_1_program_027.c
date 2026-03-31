#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

// Test functions for each comparison operator
// Each function is in a separate compilation unit context

// GT_EXPR test: simple comparison storing to mask
void test_gt(int *restrict a, int *restrict b, int *restrict mask, int n) {
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] > b[i];  // GT_EXPR
    }
}

// GE_EXPR test: comparison used for conditional blend
void test_ge(float *restrict a, float *restrict b, 
             float *restrict x, float *restrict y, 
             float *restrict dst, int n) {
    for (int i = 0; i < n; ++i) {
        // GE_EXPR used in conditional blend
        dst[i] = (a[i] >= b[i]) ? x[i] : y[i];
    }
}

// LT_EXPR test: comparison used in conditional accumulation
int test_lt(unsigned int *restrict a, unsigned int *restrict b, int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        if (a[i] < b[i]) {  // LT_EXPR
            sum += a[i];
        }
    }
    return sum;
}

// LE_EXPR test: comparison with bitwise manipulation
void test_le(double *restrict a, double *restrict b, 
             int *restrict mask, int *restrict result, int n) {
    // First generate mask
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] <= b[i];  // LE_EXPR
    }
    
    // Then use mask in bitwise operation to ensure transformation
    for (int i = 0; i < n; ++i) {
        result[i] = mask[i] & (int)a[i];
    }
}

// Additional test with mixed types to cover different code paths
void test_mixed_comparisons(int16_t *restrict a, int16_t *restrict b,
                           int *restrict gt_mask, int *restrict le_mask, int n) {
    for (int i = 0; i < n; ++i) {
        gt_mask[i] = a[i] > b[i];   // GT_EXPR with smaller type
    }
    
    for (int i = 0; i < n; ++i) {
        le_mask[i] = a[i] <= b[i];  // LE_EXPR with smaller type
    }
}

int main() {
    // Aligned arrays for different data types
    int a_int[N] ALIGNED;
    int b_int[N] ALIGNED;
    int mask_gt[N] ALIGNED;
    int mask_le[N] ALIGNED;
    int result_int[N] ALIGNED;
    
    float a_float[N] ALIGNED;
    float b_float[N] ALIGNED;
    float x_float[N] ALIGNED;
    float y_float[N] ALIGNED;
    float dst_float[N] ALIGNED;
    
    double a_double[N] ALIGNED;
    double b_double[N] ALIGNED;
    
    unsigned int a_uint[N] ALIGNED;
    unsigned int b_uint[N] ALIGNED;
    
    int16_t a_short[N] ALIGNED;
    int16_t b_short[N] ALIGNED;
    int mask_short_gt[N] ALIGNED;
    int mask_short_le[N] ALIGNED;
    
    // Initialize arrays with pattern data
    for (int i = 0; i < N; ++i) {
        // For integer tests
        a_int[i] = i;
        b_int[i] = N - i - 1;
        
        // For unsigned tests
        a_uint[i] = i * 2;
        b_uint[i] = i * 3;
        
        // For float tests
        a_float[i] = i * 1.5f;
        b_float[i] = i * 2.0f;
        x_float[i] = i * 0.5f;
        y_float[i] = i * 1.0f;
        
        // For double tests
        a_double[i] = i * 1.25;
        b_double[i] = i * 1.75;
        
        // For short tests
        a_short[i] = (i % 256) - 128;
        b_short[i] = (i % 128) - 64;
    }
    
    int total_sum = 0;
    
    // Test GT_EXPR (line 12216-12219)
    test_gt(a_int, b_int, mask_gt, N);
    for (int i = 0; i < N; ++i) total_sum += mask_gt[i];
    
    // Test GE_EXPR (line 12220-12223)
    test_ge(a_float, b_float, x_float, y_float, dst_float, N);
    for (int i = 0; i < N; ++i) total_sum += (int)dst_float[i];
    
    // Test LT_EXPR (line 12224-12228)
    total_sum += test_lt(a_uint, b_uint, N);
    
    // Test LE_EXPR (line 12229-12233)
    test_le(a_double, b_double, mask_le, result_int, N);
    for (int i = 0; i < N; ++i) total_sum += result_int[i];
    
    // Test mixed comparisons
    test_mixed_comparisons(a_short, b_short, mask_short_gt, mask_short_le, N);
    for (int i = 0; i < N; ++i) {
        total_sum += mask_short_gt[i] + mask_short_le[i];
    }
    
    // Prevent dead code elimination
    printf("Total sum: %d\n", total_sum);
    
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

// Test functions for each comparison operator
// Each function uses a different pattern to encourage mask optimization

// GT_EXPR: Simple comparison storing to mask array
void test_gt(int *restrict a, int *restrict b, int *restrict mask, int n) {
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] > b[i];  // GT_EXPR
    }
}

// GE_EXPR: Conditional blend using mask
void test_ge(float *restrict a, float *restrict b, 
             float *restrict x, float *restrict y, 
             float *restrict dst, int n) {
    for (int i = 0; i < n; ++i) {
        // GE_EXPR used in conditional blend
        dst[i] = (a[i] >= b[i]) ? x[i] : y[i];
    }
}

// LT_EXPR: Masked accumulation
int test_lt(unsigned int *restrict a, unsigned int *restrict b, int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        if (a[i] < b[i]) {  // LT_EXPR
            sum += a[i];
        }
    }
    return sum;
}

// LE_EXPR: Comparison with subsequent bitwise operation
void test_le(double *restrict a, double *restrict b, 
             int *restrict mask, int *restrict result, int n) {
    // First create mask
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] <= b[i];  // LE_EXPR
    }
    
    // Then use mask in bitwise operation to encourage optimization
    for (int i = 0; i < n; ++i) {
        result[i] = mask[i] & (int)a[i];
    }
}

// Additional test with mixed types to cover different code paths
void test_mixed_comparisons(int16_t *restrict a, int16_t *restrict b,
                           int32_t *restrict mask_gt, int32_t *restrict mask_le,
                           int n) {
    // GT_EXPR with different width types
    for (int i = 0; i < n; ++i) {
        mask_gt[i] = a[i] > b[i];
    }
    
    // LE_EXPR with different width types
    for (int i = 0; i < n; ++i) {
        mask_le[i] = a[i] <= b[i];
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
    
    unsigned int a_uint[N] ALIGNED;
    unsigned int b_uint[N] ALIGNED;
    
    double a_double[N] ALIGNED;
    double b_double[N] ALIGNED;
    
    int16_t a_short[N] ALIGNED;
    int16_t b_short[N] ALIGNED;
    int32_t mask_short_gt[N] ALIGNED;
    int32_t mask_short_le[N] ALIGNED;
    
    // Initialize arrays with patterned data
    for (int i = 0; i < N; ++i) {
        // For integer tests: alternating pattern
        a_int[i] = i;
        b_int[i] = (i % 3 == 0) ? i + 1 : i - 1;
        mask_gt[i] = 0;
        mask_le[i] = 0;
        result_int[i] = 0;
        
        // For float tests: sine-like pattern
        a_float[i] = i * 0.1f;
        b_float[i] = (i % 2 == 0) ? i * 0.1f + 0.5f : i * 0.1f - 0.5f;
        x_float[i] = i * 0.2f;
        y_float[i] = i * 0.3f;
        dst_float[i] = 0.0f;
        
        // For unsigned tests: different ranges
        a_uint[i] = i * 2;
        b_uint[i] = i * 3;
        
        // For double tests: similar to float but with different scale
        a_double[i] = i * 0.05;
        b_double[i] = (i % 4 == 0) ? i * 0.05 + 1.0 : i * 0.05 - 1.0;
        
        // For mixed type tests
        a_short[i] = (i % 256) - 128;
        b_short[i] = (i % 128) - 64;
        mask_short_gt[i] = 0;
        mask_short_le[i] = 0;
    }
    
    int total_sum = 0;
    
    // Test GT_EXPR with integers
    test_gt(a_int, b_int, mask_gt, N);
    for (int i = 0; i < N; ++i) total_sum += mask_gt[i];
    
    // Test GE_EXPR with floats (conditional blend)
    test_ge(a_float, b_float, x_float, y_float, dst_float, N);
    for (int i = 0; i < N; ++i) total_sum += (int)dst_float[i];
    
    // Test LT_EXPR with unsigned integers (masked accumulation)
    total_sum += test_lt(a_uint, b_uint, N);
    
    // Test LE_EXPR with doubles
    test_le(a_double, b_double, mask_le, result_int, N);
    for (int i = 0; i < N; ++i) total_sum += result_int[i];
    
    // Test mixed type comparisons
    test_mixed_comparisons(a_short, b_short, mask_short_gt, mask_short_le, N);
    for (int i = 0; i < N; ++i) {
        total_sum += mask_short_gt[i] + mask_short_le[i];
    }
    
    // Additional test: Nested loops with different comparison operators
    // This might trigger different optimization paths
    for (int outer = 0; outer < 4; ++outer) {
        int temp_mask[N] ALIGNED;
        for (int i = 0; i < N; ++i) {
            // Use different operators based on outer loop
            switch (outer) {
                case 0: temp_mask[i] = a_int[i] > b_int[i]; break;   // GT_EXPR
                case 1: temp_mask[i] = a_int[i] >= b_int[i]; break;  // GE_EXPR
                case 2: temp_mask[i] = a_int[i] < b_int[i]; break;   // LT_EXPR
                case 3: temp_mask[i] = a_int[i] <= b_int[i]; break;  // LE_EXPR
            }
        }
        for (int i = 0; i < N; ++i) {
            total_sum += temp_mask[i];
        }
    }
    
    printf("Total sum: %d\n", total_sum);
    
    // Prevent dead code elimination
    volatile int prevent_optimization = total_sum;
    
    return 0;
}

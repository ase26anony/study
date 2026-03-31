#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

// Test functions for each comparison operator
// Each function uses a different pattern to encourage mask optimization

// GT_EXPR (>)
void test_gt(int *restrict a, int *restrict b, int *restrict mask, int n) {
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] > b[i];  // Direct mask storage
    }
}

// GE_EXPR (>=) with conditional blend
void test_ge(float *restrict a, float *restrict b, 
             float *restrict x, float *restrict y,
             float *restrict dst, int n) {
    for (int i = 0; i < n; ++i) {
        // Conditional blend using comparison result
        dst[i] = (a[i] >= b[i]) ? x[i] : y[i];
    }
}

// LT_EXPR (<) with masked accumulation
int test_lt(unsigned int *restrict a, unsigned int *restrict b, int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        if (a[i] < b[i]) {  // Conditional accumulation
            sum += a[i];
        }
    }
    return sum;
}

// LE_EXPR (<=) with mask usage in bitwise operations
void test_le(double *restrict a, double *restrict b, 
             uint64_t *restrict mask, int n) {
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] <= b[i];  // Store boolean as integer
    }
    
    // Additional operation to encourage mask optimization
    for (int i = 0; i < n; ++i) {
        // Use mask in bitwise operation
        mask[i] = mask[i] & 0x1;
    }
}

// Additional test with mixed types to cover different paths
void test_mixed_comparisons(int8_t *restrict a8, int8_t *restrict b8,
                            int16_t *restrict a16, int16_t *restrict b16,
                            int *restrict result, int n) {
    for (int i = 0; i < n; ++i) {
        // Multiple comparisons in sequence
        int cmp1 = a8[i] > b8[i];
        int cmp2 = a16[i] >= b16[i];
        result[i] = cmp1 & cmp2;  // Combine masks
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
    uint64_t mask_le[N] ALIGNED;
    
    int8_t a_int8[N] ALIGNED;
    int8_t b_int8[N] ALIGNED;
    int16_t a_int16[N] ALIGNED;
    int16_t b_int16[N] ALIGNED;
    int result_mixed[N] ALIGNED;
    
    // Initialize arrays with pattern data
    for (int i = 0; i < N; ++i) {
        // Integer arrays
        a_int[i] = i;
        b_int[i] = N - i;
        
        // Float arrays
        a_float[i] = i * 0.5f;
        b_float[i] = (N - i) * 0.5f;
        x_float[i] = i * 1.0f;
        y_float[i] = i * 2.0f;
        
        // Unsigned arrays
        a_uint[i] = i * 2;
        b_uint[i] = i * 3;
        
        // Double arrays
        a_double[i] = i * 0.25;
        b_double[i] = (N - i) * 0.25;
        
        // Mixed type arrays
        a_int8[i] = i % 128;
        b_int8[i] = (i + 64) % 128;
        a_int16[i] = i % 32768;
        b_int16[i] = (i + 16384) % 32768;
    }
    
    // Execute all test functions
    test_gt(a_int, b_int, mask_gt, N);
    
    test_ge(a_float, b_float, x_float, y_float, dst_ge, N);
    
    int sum_lt = test_lt(a_uint, b_uint, N);
    
    test_le(a_double, b_double, mask_le, N);
    
    test_mixed_comparisons(a_int8, b_int8, a_int16, b_int16, result_mixed, N);
    
    // Final reduction to prevent dead code elimination
    int final_sum = 0;
    for (int i = 0; i < N; ++i) {
        final_sum += mask_gt[i];
        final_sum += (int)dst_ge[i];
        final_sum += result_mixed[i];
        final_sum += (int)mask_le[i];
    }
    final_sum += sum_lt;
    
    printf("Final result: %d\n", final_sum);
    
    return 0;
}

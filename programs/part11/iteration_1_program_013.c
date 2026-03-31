#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

// Test functions for different comparison operators
// Each function tests a specific comparison operator in a vectorizable loop

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
        // GE_EXPR with conditional blend
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
    // First create the mask
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] <= b[i];  // LE_EXPR
    }
    
    // Then use the mask in bitwise operations
    for (int i = 0; i < n; ++i) {
        // This ensures the mask is materialized and used
        result[i] = mask[i] & (int)a[i];
    }
}

// Additional test with mixed types to ensure coverage
void test_mixed_comparisons(int *restrict a, int *restrict b, 
                           int *restrict results, int n) {
    // Separate loops for each comparison operator
    // GT_EXPR
    for (int i = 0; i < n; ++i) {
        results[i] = a[i] > b[i];
    }
    
    // GE_EXPR  
    for (int i = 0; i < n; ++i) {
        results[i + n] = a[i] >= b[i];
    }
    
    // LT_EXPR
    for (int i = 0; i < n; ++i) {
        results[i + 2*n] = a[i] < b[i];
    }
    
    // LE_EXPR
    for (int i = 0; i < n; ++i) {
        results[i + 3*n] = a[i] <= b[i];
    }
}

int main() {
    // Aligned arrays for different data types
    int a_int[N] ALIGNED;
    int b_int[N] ALIGNED;
    int mask_gt[N] ALIGNED;
    int mask_le[N] ALIGNED;
    int results_mixed[4*N] ALIGNED;
    
    float a_float[N] ALIGNED;
    float b_float[N] ALIGNED;
    float x_float[N] ALIGNED;
    float y_float[N] ALIGNED;
    float dst_ge[N] ALIGNED;
    
    unsigned int a_uint[N] ALIGNED;
    unsigned int b_uint[N] ALIGNED;
    
    double a_double[N] ALIGNED;
    double b_double[N] ALIGNED;
    int result_le[N] ALIGNED;
    
    // Initialize arrays with pattern data
    for (int i = 0; i < N; ++i) {
        // Integer arrays
        a_int[i] = i;
        b_int[i] = N - i - 1;
        mask_gt[i] = 0;
        mask_le[i] = 0;
        
        // Float arrays
        a_float[i] = i * 1.0f;
        b_float[i] = (N - i - 1) * 1.0f;
        x_float[i] = i * 2.0f;
        y_float[i] = i * 0.5f;
        dst_ge[i] = 0.0f;
        
        // Unsigned arrays
        a_uint[i] = i * 2;
        b_uint[i] = i * 3;
        
        // Double arrays
        a_double[i] = i * 1.0;
        b_double[i] = (i % 2 == 0) ? i * 2.0 : i * 0.5;
        result_le[i] = 0;
    }
    
    // Initialize mixed results array
    memset(results_mixed, 0, 4*N*sizeof(int));
    
    int total_sum = 0;
    
    // Test GT_EXPR (>)
    test_gt(a_int, b_int, mask_gt, N);
    for (int i = 0; i < N; ++i) {
        total_sum += mask_gt[i];
    }
    
    // Test GE_EXPR (>=) with float conditional blend
    test_ge(a_float, b_float, x_float, y_float, dst_ge, N);
    for (int i = 0; i < N; ++i) {
        total_sum += (int)dst_ge[i];
    }
    
    // Test LT_EXPR (<) with unsigned int and conditional accumulation
    total_sum += test_lt(a_uint, b_uint, N);
    
    // Test LE_EXPR (<=) with double and mask usage
    test_le(a_double, b_double, mask_le, result_le, N);
    for (int i = 0; i < N; ++i) {
        total_sum += result_le[i];
    }
    
    // Test all comparison operators in separate loops
    test_mixed_comparisons(a_int, b_int, results_mixed, N);
    for (int i = 0; i < 4*N; ++i) {
        total_sum += results_mixed[i];
    }
    
    // Print result to prevent dead code elimination
    printf("Total sum: %d\n", total_sum);
    
    return 0;
}

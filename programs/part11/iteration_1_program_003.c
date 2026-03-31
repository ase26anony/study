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
        mask[i] = a[i] > b[i];
    }
}

// GE_EXPR (>=) with conditional blend
void test_ge(float *restrict a, float *restrict b, float *restrict x, 
             float *restrict y, float *restrict dst, int n) {
    for (int i = 0; i < n; ++i) {
        // This should generate a mask and conditional blend
        dst[i] = (a[i] >= b[i]) ? x[i] : y[i];
    }
}

// LT_EXPR (<) with conditional accumulation
int test_lt(unsigned int *restrict a, unsigned int *restrict b, int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        if (a[i] < b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

// LE_EXPR (<=) with mask usage
void test_le(double *restrict a, double *restrict b, int *restrict mask, 
             double *restrict result, int n) {
    // First create the mask
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] <= b[i];
    }
    
    // Then use it in a way that might trigger bitwise optimization
    for (int i = 0; i < n; ++i) {
        // Using the mask to conditionally update result
        if (mask[i]) {
            result[i] = a[i];
        } else {
            result[i] = b[i];
        }
    }
}

// Additional test with mixed types to ensure coverage
void test_mixed_comparisons(int *restrict a, int *restrict b, 
                           int *restrict results, int n) {
    // Test all comparison operators in separate loops
    // GT
    for (int i = 0; i < n; ++i) {
        results[i] = a[i] > b[i];
    }
    
    // GE
    for (int i = 0; i < n; ++i) {
        results[i + n] = a[i] >= b[i];
    }
    
    // LT
    for (int i = 0; i < n; ++i) {
        results[i + 2*n] = a[i] < b[i];
    }
    
    // LE
    for (int i = 0; i < n; ++i) {
        results[i + 3*n] = a[i] <= b[i];
    }
}

int main() {
    // Aligned arrays for different data types
    int a_int[N] ALIGNED;
    int b_int[N] ALIGNED;
    int mask_int[N] ALIGNED;
    
    float a_float[N] ALIGNED;
    float b_float[N] ALIGNED;
    float x_float[N] ALIGNED;
    float y_float[N] ALIGNED;
    float dst_float[N] ALIGNED;
    
    unsigned int a_uint[N] ALIGNED;
    unsigned int b_uint[N] ALIGNED;
    
    double a_double[N] ALIGNED;
    double b_double[N] ALIGNED;
    double result_double[N] ALIGNED;
    int mask_double[N] ALIGNED;
    
    // Initialize arrays with pattern data
    for (int i = 0; i < N; ++i) {
        a_int[i] = i;
        b_int[i] = N - i;
        
        a_float[i] = i * 1.5f;
        b_float[i] = (N - i) * 1.5f;
        x_float[i] = i * 2.0f;
        y_float[i] = i * 3.0f;
        
        a_uint[i] = i * 2;
        b_uint[i] = i * 3;
        
        a_double[i] = i * 0.5;
        b_double[i] = (N - i) * 0.5;
    }
    
    int total_sum = 0;
    
    // Test GT_EXPR (>)
    test_gt(a_int, b_int, mask_int, N);
    for (int i = 0; i < N; ++i) {
        total_sum += mask_int[i];
    }
    
    // Test GE_EXPR (>=) with conditional blend
    test_ge(a_float, b_float, x_float, y_float, dst_float, N);
    for (int i = 0; i < N; ++i) {
        total_sum += (int)dst_float[i];
    }
    
    // Test LT_EXPR (<) with conditional accumulation
    total_sum += test_lt(a_uint, b_uint, N);
    
    // Test LE_EXPR (<=) with mask usage
    test_le(a_double, b_double, mask_double, result_double, N);
    for (int i = 0; i < N; ++i) {
        total_sum += (int)result_double[i];
        total_sum += mask_double[i];
    }
    
    // Additional comprehensive test
    int *mixed_results = (int*)malloc(4 * N * sizeof(int));
    test_mixed_comparisons(a_int, b_int, mixed_results, N);
    for (int i = 0; i < 4 * N; ++i) {
        total_sum += mixed_results[i];
    }
    free(mixed_results);
    
    printf("Total sum: %d\n", total_sum);
    
    return 0;
}

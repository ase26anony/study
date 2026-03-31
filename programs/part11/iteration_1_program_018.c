#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
    // First create mask
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] <= b[i];  // LE_EXPR
    }
    
    // Use mask in bitwise operation to ensure it's materialized
    for (int i = 0; i < n; ++i) {
        result[i] = mask[i] & 0x1;  // Simple bitwise AND to use the mask
    }
}

// Additional test with mixed types to stress different code paths
void test_mixed_comparisons(int *restrict a, int *restrict b, 
                           float *restrict fa, float *restrict fb,
                           int *restrict results, int n) {
    // Test multiple comparison types in separate loops
    for (int i = 0; i < n; ++i) {
        results[i] = (a[i] > b[i]);  // GT_EXPR
    }
    
    for (int i = 0; i < n; ++i) {
        results[i] |= (fa[i] >= fb[i]) ? 2 : 0;  // GE_EXPR
    }
    
    for (int i = 0; i < n; ++i) {
        results[i] |= (a[i] < b[i]) ? 4 : 0;  // LT_EXPR
    }
    
    for (int i = 0; i < n; ++i) {
        results[i] |= (fa[i] <= fb[i]) ? 8 : 0;  // LE_EXPR
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
    
    int mixed_results[N] ALIGNED;
    
    // Initialize with random data
    srand(time(NULL));
    for (int i = 0; i < N; ++i) {
        a_int[i] = rand() % 1000;
        b_int[i] = rand() % 1000;
        
        a_float[i] = (float)(rand() % 1000) / 10.0f;
        b_float[i] = (float)(rand() % 1000) / 10.0f;
        x_float[i] = (float)(rand() % 1000) / 10.0f;
        y_float[i] = (float)(rand() % 1000) / 10.0f;
        
        a_uint[i] = rand() % 1000;
        b_uint[i] = rand() % 1000;
        
        a_double[i] = (double)(rand() % 1000) / 10.0;
        b_double[i] = (double)(rand() % 1000) / 10.0;
    }
    
    // Call test functions to exercise different comparison operators
    test_gt(a_int, b_int, mask_gt, N);
    
    test_ge(a_float, b_float, x_float, y_float, dst_ge, N);
    
    int sum_lt = test_lt(a_uint, b_uint, N);
    
    test_le(a_double, b_double, mask_le, result_le, N);
    
    test_mixed_comparisons(a_int, b_int, a_float, b_float, mixed_results, N);
    
    // Final reduction to prevent dead code elimination
    int final_sum = 0;
    for (int i = 0; i < N; ++i) {
        final_sum += mask_gt[i];
        final_sum += (int)dst_ge[i];
        final_sum += result_le[i];
        final_sum += mixed_results[i];
    }
    final_sum += sum_lt;
    
    printf("Final sum: %d\n", final_sum);
    
    return 0;
}

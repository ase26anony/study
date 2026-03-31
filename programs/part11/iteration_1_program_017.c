#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGN __attribute__((aligned(32)))

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
        // GE_EXPR with blend operation
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

// LE_EXPR (<=) with mask usage in bitwise operations
void test_le(double *restrict a, double *restrict b, int *restrict mask, 
             int *restrict result, int n) {
    // First generate the mask
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] <= b[i];  // LE_EXPR
    }
    
    // Use the mask in bitwise operations to ensure mask materialization
    for (int i = 0; i < n; ++i) {
        result[i] = mask[i] & (i + 1);
    }
}

// Additional test with mixed types to cover different vectorization paths
void test_mixed_comparisons(int *restrict a, int *restrict b, 
                           float *restrict fa, float *restrict fb,
                           int *restrict int_mask, int *restrict float_mask, int n) {
    // Integer comparisons
    for (int i = 0; i < n; ++i) {
        int_mask[i] = a[i] > b[i];  // GT_EXPR
    }
    
    // Floating point comparisons
    for (int i = 0; i < n; ++i) {
        float_mask[i] = fa[i] <= fb[i];  // LE_EXPR
    }
}

int main() {
    // Aligned arrays for different data types
    int a_int[N] ALIGN;
    int b_int[N] ALIGN;
    int mask_gt[N] ALIGN;
    int mask_le[N] ALIGN;
    int result[N] ALIGN;
    
    float a_float[N] ALIGN;
    float b_float[N] ALIGN;
    float x[N] ALIGN;
    float y[N] ALIGN;
    float dst_ge[N] ALIGN;
    
    unsigned int a_uint[N] ALIGN;
    unsigned int b_uint[N] ALIGN;
    
    double a_double[N] ALIGN;
    double b_double[N] ALIGN;
    
    int int_mask[N] ALIGN;
    int float_mask[N] ALIGN;
    
    // Initialize arrays with pattern data
    for (int i = 0; i < N; ++i) {
        a_int[i] = i;
        b_int[i] = N - i;
        a_float[i] = i * 0.5f;
        b_float[i] = (N - i) * 0.5f;
        x[i] = i * 1.5f;
        y[i] = i * 2.5f;
        a_uint[i] = i * 2;
        b_uint[i] = i * 3;
        a_double[i] = i * 0.25;
        b_double[i] = (N - i) * 0.25;
    }
    
    int total_sum = 0;
    
    // Test GT_EXPR (>)
    test_gt(a_int, b_int, mask_gt, N);
    for (int i = 0; i < N; ++i) {
        total_sum += mask_gt[i];
    }
    
    // Test GE_EXPR (>=) with blend
    test_ge(a_float, b_float, x, y, dst_ge, N);
    for (int i = 0; i < N; ++i) {
        total_sum += (int)dst_ge[i];
    }
    
    // Test LT_EXPR (<) with conditional accumulation
    total_sum += test_lt(a_uint, b_uint, N);
    
    // Test LE_EXPR (<=) with mask usage
    test_le(a_double, b_double, mask_le, result, N);
    for (int i = 0; i < N; ++i) {
        total_sum += result[i];
    }
    
    // Test mixed comparisons
    test_mixed_comparisons(a_int, b_int, a_float, b_float, int_mask, float_mask, N);
    for (int i = 0; i < N; ++i) {
        total_sum += int_mask[i] + float_mask[i];
    }
    
    // Additional test: nested comparisons to stress the transformation
    int temp_mask[N] ALIGN;
    for (int i = 0; i < N; ++i) {
        // This creates a more complex mask expression
        temp_mask[i] = (a_int[i] > b_int[i]) && (a_float[i] <= b_float[i]);
        total_sum += temp_mask[i];
    }
    
    printf("Total sum: %d\n", total_sum);
    
    return 0;
}

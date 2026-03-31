#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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
    
    // Use mask in bitwise operation to ensure transformation
    for (int i = 0; i < n; ++i) {
        result[i] = mask[i] & (int)a[i];
    }
}

// Additional test with mixed types for comprehensive coverage
void test_mixed_comparisons(int *restrict a, int *restrict b, 
                           float *restrict fa, float *restrict fb,
                           int *restrict results, int n) {
    // Test multiple comparison types in separate loops
    for (int i = 0; i < n; ++i) {
        results[i] = (a[i] > b[i]) ? 1 : 0;  // GT_EXPR
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
    int a_int[N] ALIGN;
    int b_int[N] ALIGN;
    int mask_gt[N] ALIGN;
    int mask_le[N] ALIGN;
    int results[N] ALIGN;
    
    float a_float[N] ALIGN;
    float b_float[N] ALIGN;
    float x_float[N] ALIGN;
    float y_float[N] ALIGN;
    float dst_ge[N] ALIGN;
    
    unsigned int a_uint[N] ALIGN;
    unsigned int b_uint[N] ALIGN;
    
    double a_double[N] ALIGN;
    double b_double[N] ALIGN;
    
    // Initialize arrays with pattern data
    for (int i = 0; i < N; ++i) {
        // Integer arrays - alternating patterns
        a_int[i] = i;
        b_int[i] = (i % 3 == 0) ? i + 1 : i - 1;
        
        // Float arrays - sine/cosine pattern
        a_float[i] = sinf(i * 0.1f);
        b_float[i] = cosf(i * 0.1f);
        x_float[i] = i * 0.5f;
        y_float[i] = i * 0.3f;
        
        // Unsigned arrays - different pattern
        a_uint[i] = i * 2;
        b_uint[i] = i * 3;
        
        // Double arrays - exponential pattern
        a_double[i] = exp(i * 0.01);
        b_double[i] = exp(i * 0.02);
    }
    
    int total_sum = 0;
    
    // Test GT_EXPR (>)
    test_gt(a_int, b_int, mask_gt, N);
    for (int i = 0; i < N; ++i) total_sum += mask_gt[i];
    
    // Test GE_EXPR (>=) with float conditional blend
    test_ge(a_float, b_float, x_float, y_float, dst_ge, N);
    for (int i = 0; i < N; ++i) total_sum += (int)dst_ge[i];
    
    // Test LT_EXPR (<) with unsigned int and conditional accumulation
    total_sum += test_lt(a_uint, b_uint, N);
    
    // Test LE_EXPR (<=) with double and mask usage
    test_le(a_double, b_double, mask_le, results, N);
    for (int i = 0; i < N; ++i) total_sum += results[i];
    
    // Test mixed comparisons
    test_mixed_comparisons(a_int, b_int, a_float, b_float, results, N);
    for (int i = 0; i < N; ++i) total_sum += results[i];
    
    // Additional test: Nested comparisons to stress the transformation
    int temp_mask[N] ALIGN;
    for (int i = 0; i < N; ++i) {
        // Complex mask operation that might trigger BIT_AND/BIT_IOR
        temp_mask[i] = (a_int[i] > b_int[i]) & (a_float[i] >= b_float[i]);
        temp_mask[i] |= (a_uint[i] < b_uint[i]) & (a_double[i] <= b_double[i]);
        total_sum += temp_mask[i];
    }
    
    printf("Total sum: %d\n", total_sum);
    printf("(This output prevents dead code elimination)\n");
    
    return 0;
}

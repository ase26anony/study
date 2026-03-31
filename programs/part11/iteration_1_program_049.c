#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGN __attribute__((aligned(32)))

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

// LE_EXPR (<=) with mask usage and bitwise operation
void test_le(double *restrict a, double *restrict b, int *restrict mask, 
             int *restrict result, int n) {
    // First create the mask
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] <= b[i];
    }
    
    // Use the mask in a way that might trigger bitwise optimization
    for (int i = 0; i < n; ++i) {
        // This pattern might encourage mask to bitwise transformation
        result[i] = mask[i] & (int)a[i];
    }
}

// Additional test with mixed types to cover more cases
void test_mixed_comparisons(int *restrict a, int *restrict b, 
                           float *restrict fa, float *restrict fb,
                           int *restrict results, int n) {
    // Test > with integers
    for (int i = 0; i < n; ++i) {
        results[i] = a[i] > b[i];
    }
    
    // Test >= with floats
    for (int i = 0; i < n; ++i) {
        results[i + n] = fa[i] >= fb[i];
    }
}

int main() {
    // Aligned arrays for integer tests
    int a_int[N] ALIGN;
    int b_int[N] ALIGN;
    int mask_gt[N] ALIGN;
    int mask_le[N] ALIGN;
    int results_mixed[N * 2] ALIGN;
    
    // Aligned arrays for float tests
    float a_float[N] ALIGN;
    float b_float[N] ALIGN;
    float x_float[N] ALIGN;
    float y_float[N] ALIGN;
    float dst_ge[N] ALIGN;
    
    // Aligned arrays for unsigned tests
    unsigned int a_uint[N] ALIGN;
    unsigned int b_uint[N] ALIGN;
    
    // Aligned arrays for double tests
    double a_double[N] ALIGN;
    double b_double[N] ALIGN;
    int result_le[N] ALIGN;
    
    // Initialize arrays with patterned data
    for (int i = 0; i < N; ++i) {
        // Integer arrays: alternating pattern
        a_int[i] = i;
        b_int[i] = (i % 3 == 0) ? i + 1 : i - 1;
        
        // Float arrays: sine-like pattern
        a_float[i] = i * 0.1f;
        b_float[i] = (i % 2 == 0) ? i * 0.1f + 0.05f : i * 0.1f - 0.05f;
        x_float[i] = i * 0.2f;
        y_float[i] = i * 0.3f;
        
        // Unsigned arrays: different pattern
        a_uint[i] = i * 2;
        b_uint[i] = i * 2 + (i % 5);
        
        // Double arrays: similar to float but with different scale
        a_double[i] = i * 0.05;
        b_double[i] = (i % 4 == 0) ? i * 0.05 + 0.02 : i * 0.05 - 0.02;
    }
    
    int total_sum = 0;
    
    // Test 1: GT_EXPR (>)
    test_gt(a_int, b_int, mask_gt, N);
    for (int i = 0; i < N; ++i) {
        total_sum += mask_gt[i];
    }
    
    // Test 2: GE_EXPR (>=) with conditional blend
    test_ge(a_float, b_float, x_float, y_float, dst_ge, N);
    for (int i = 0; i < N; ++i) {
        total_sum += (int)dst_ge[i];
    }
    
    // Test 3: LT_EXPR (<) with conditional accumulation
    total_sum += test_lt(a_uint, b_uint, N);
    
    // Test 4: LE_EXPR (<=) with mask usage
    test_le(a_double, b_double, mask_le, result_le, N);
    for (int i = 0; i < N; ++i) {
        total_sum += result_le[i];
    }
    
    // Test 5: Mixed comparisons
    test_mixed_comparisons(a_int, b_int, a_float, b_float, results_mixed, N);
    for (int i = 0; i < N * 2; ++i) {
        total_sum += results_mixed[i];
    }
    
    // Print result to prevent dead code elimination
    printf("Total sum: %d\n", total_sum);
    
    return 0;
}

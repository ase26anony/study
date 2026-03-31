#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
        // This should generate a vector mask and conditional blend
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
    // First generate mask
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] <= b[i];
    }
    
    // Use mask in bitwise operation to ensure it's materialized
    for (int i = 0; i < n; ++i) {
        // Convert mask to double for computation
        result[i] = (double)(mask[i] & 1) * a[i];
    }
}

// Additional test with mixed types for comprehensive coverage
void test_mixed_comparisons(int *restrict a, int *restrict b, 
                           float *restrict fa, float *restrict fb,
                           int *restrict int_mask, float *restrict float_result, int n) {
    // Test GT_EXPR with integers
    for (int i = 0; i < n; ++i) {
        int_mask[i] = a[i] > b[i];
    }
    
    // Test LE_EXPR with floats
    for (int i = 0; i < n; ++i) {
        float_result[i] = (fa[i] <= fb[i]) ? fa[i] : 0.0f;
    }
}

int main() {
    // Aligned arrays for different data types
    int a_int[N] ALIGN;
    int b_int[N] ALIGN;
    int mask_gt[N] ALIGN;
    int mask_le[N] ALIGN;
    
    float a_float[N] ALIGN;
    float b_float[N] ALIGN;
    float x_float[N] ALIGN;
    float y_float[N] ALIGN;
    float dst_ge[N] ALIGN;
    float float_result[N] ALIGN;
    
    unsigned int a_uint[N] ALIGN;
    unsigned int b_uint[N] ALIGN;
    
    double a_double[N] ALIGN;
    double b_double[N] ALIGN;
    double result_le[N] ALIGN;
    
    // Initialize arrays with pseudo-random data
    srand(time(NULL));
    for (int i = 0; i < N; ++i) {
        a_int[i] = rand() % 1000;
        b_int[i] = rand() % 1000;
        a_uint[i] = rand() % 1000;
        b_uint[i] = rand() % 1000;
        a_float[i] = (float)(rand() % 1000) / 10.0f;
        b_float[i] = (float)(rand() % 1000) / 10.0f;
        x_float[i] = (float)(rand() % 1000) / 10.0f;
        y_float[i] = (float)(rand() % 1000) / 10.0f;
        a_double[i] = (double)(rand() % 1000) / 10.0;
        b_double[i] = (double)(rand() % 1000) / 10.0;
    }
    
    int total_sum = 0;
    
    // Test GT_EXPR (>)
    test_gt(a_int, b_int, mask_gt, N);
    
    // Test GE_EXPR (>=) with conditional blend
    test_ge(a_float, b_float, x_float, y_float, dst_ge, N);
    
    // Test LT_EXPR (<) with conditional accumulation
    total_sum += test_lt(a_uint, b_uint, N);
    
    // Test LE_EXPR (<=) with mask usage
    test_le(a_double, b_double, mask_le, result_le, N);
    
    // Test mixed comparisons
    test_mixed_comparisons(a_int, b_int, a_float, b_float, mask_gt, float_result, N);
    
    // Final reduction to prevent dead code elimination
    for (int i = 0; i < N; ++i) {
        total_sum += mask_gt[i];
        total_sum += (int)dst_ge[i];
        total_sum += (int)result_le[i];
        total_sum += (int)float_result[i];
    }
    
    printf("Total sum: %d\n", total_sum);
    printf("(This value varies based on random initialization)\n");
    
    return 0;
}

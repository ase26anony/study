#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGN __attribute__((aligned(32)))

// Test functions for each comparison operator
static void test_gt(int *restrict a, int *restrict b, int *restrict mask, int n) {
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] > b[i];  // GT_EXPR
    }
}

static void test_ge(float *restrict a, float *restrict b, 
                    float *restrict x, float *restrict y, 
                    float *restrict dst, int n) {
    for (int i = 0; i < n; ++i) {
        // GE_EXPR with conditional blend
        dst[i] = (a[i] >= b[i]) ? x[i] : y[i];
    }
}

static void test_lt(unsigned int *restrict a, unsigned int *restrict b, 
                    int *restrict mask, int n, unsigned int *restrict sum) {
    unsigned int local_sum = 0;
    for (int i = 0; i < n; ++i) {
        // LT_EXPR with conditional accumulation
        if (a[i] < b[i]) {  // LT_EXPR
            local_sum += a[i];
            mask[i] = 1;
        } else {
            mask[i] = 0;
        }
    }
    *sum = local_sum;
}

static void test_le(double *restrict a, double *restrict b, 
                    int *restrict mask, int n) {
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] <= b[i];  // LE_EXPR
    }
    
    // Use mask in bitwise operations to encourage transformation
    for (int i = 0; i < n; ++i) {
        // This should encourage BIT_NOT_EXPR + BIT_IOR_EXPR transformation
        mask[i] = ~mask[i] | (i & 0xFF);
    }
}

// Additional test with mixed types to stress different code paths
static void test_all_operators(int *restrict a, int *restrict b, 
                               int *restrict results, int n) {
    // Separate loops for each operator
    for (int i = 0; i < n; ++i) {
        results[i] = a[i] > b[i];  // GT_EXPR
    }
    
    for (int i = 0; i < n; ++i) {
        results[i + n] = a[i] >= b[i];  // GE_EXPR
    }
    
    for (int i = 0; i < n; ++i) {
        results[i + 2*n] = a[i] < b[i];  // LT_EXPR
    }
    
    for (int i = 0; i < n; ++i) {
        results[i + 3*n] = a[i] <= b[i];  // LE_EXPR
    }
}

int main() {
    // Aligned arrays for different data types
    int a_int[N] ALIGN;
    int b_int[N] ALIGN;
    int mask_gt[N] ALIGN;
    int mask_lt[N] ALIGN;
    int mask_le[N] ALIGN;
    int results[4*N] ALIGN;
    
    float a_float[N] ALIGN;
    float b_float[N] ALIGN;
    float x_float[N] ALIGN;
    float y_float[N] ALIGN;
    float dst_ge[N] ALIGN;
    
    unsigned int a_uint[N] ALIGN;
    unsigned int b_uint[N] ALIGN;
    unsigned int sum_lt = 0;
    
    double a_double[N] ALIGN;
    double b_double[N] ALIGN;
    
    // Initialize arrays with pattern data
    for (int i = 0; i < N; ++i) {
        a_int[i] = i;
        b_int[i] = N - i;
        
        a_float[i] = i * 0.5f;
        b_float[i] = (N - i) * 0.3f;
        x_float[i] = i * 1.0f;
        y_float[i] = i * 2.0f;
        
        a_uint[i] = i * 2;
        b_uint[i] = i * 3;
        
        a_double[i] = i * 0.25;
        b_double[i] = (N - i) * 0.15;
    }
    
    // Call test functions to exercise each comparison operator
    test_gt(a_int, b_int, mask_gt, N);
    
    test_ge(a_float, b_float, x_float, y_float, dst_ge, N);
    
    test_lt(a_uint, b_uint, mask_lt, N, &sum_lt);
    
    test_le(a_double, b_double, mask_le, N);
    
    test_all_operators(a_int, b_int, results, N);
    
    // Final reduction to prevent dead code elimination
    int final_sum = 0;
    for (int i = 0; i < N; ++i) {
        final_sum += mask_gt[i] + mask_lt[i] + mask_le[i] + (int)dst_ge[i];
    }
    final_sum += sum_lt;
    
    for (int i = 0; i < 4*N; ++i) {
        final_sum += results[i];
    }
    
    printf("Final sum: %d\n", final_sum);
    
    return 0;
}

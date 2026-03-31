#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

// Function prototypes
void test_gt(int *restrict a, int *restrict b, int *restrict mask, int n);
void test_ge(float *restrict a, float *restrict b, float *restrict dst, 
             float *restrict x, float *restrict y, int n);
long test_lt(unsigned int *restrict a, unsigned int *restrict b, int n);
void test_le(double *restrict a, double *restrict b, int *restrict mask, int n);

// Test GT_EXPR (>)
void test_gt(int *restrict a, int *restrict b, int *restrict mask, int n) {
    // Simple element-wise comparison: a[i] > b[i]
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] > b[i];
    }
}

// Test GE_EXPR (>=) with conditional blend
void test_ge(float *restrict a, float *restrict b, float *restrict dst,
             float *restrict x, float *restrict y, int n) {
    // Conditional blend: dst[i] = (a[i] >= b[i]) ? x[i] : y[i]
    for (int i = 0; i < n; ++i) {
        dst[i] = (a[i] >= b[i]) ? x[i] : y[i];
    }
}

// Test LT_EXPR (<) with conditional accumulation
long test_lt(unsigned int *restrict a, unsigned int *restrict b, int n) {
    long sum = 0;
    // Conditional accumulation: if (a[i] < b[i]) sum += a[i]
    for (int i = 0; i < n; ++i) {
        if (a[i] < b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

// Test LE_EXPR (<=) with mask usage
void test_le(double *restrict a, double *restrict b, int *restrict mask, int n) {
    // Store comparison results
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] <= b[i];
    }
    
    // Use mask in bitwise operation to ensure it's materialized
    for (int i = 0; i < n; ++i) {
        // This encourages the transformation to bitwise logic
        mask[i] = mask[i] & 0x1;
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
    
    // Initialize arrays with pattern data
    for (int i = 0; i < N; ++i) {
        // Integer arrays: alternating patterns
        a_int[i] = i;
        b_int[i] = (i % 3) * 100;
        
        // Float arrays: sine/cosine patterns
        a_float[i] = sinf(i * 0.1f);
        b_float[i] = cosf(i * 0.1f);
        x_float[i] = i * 0.5f;
        y_float[i] = i * 0.3f;
        
        // Unsigned arrays: different ranges
        a_uint[i] = i * 2;
        b_uint[i] = 1500 - i;
        
        // Double arrays: exponential patterns
        a_double[i] = exp(i * 0.01);
        b_double[i] = exp((N - i) * 0.01);
    }
    
    long total_sum = 0;
    
    // Test GT_EXPR (>)
    test_gt(a_int, b_int, mask_gt, N);
    for (int i = 0; i < N; ++i) {
        total_sum += mask_gt[i];
    }
    
    // Test GE_EXPR (>=)
    test_ge(a_float, b_float, dst_ge, x_float, y_float, N);
    for (int i = 0; i < N; ++i) {
        total_sum += (int)dst_ge[i];
    }
    
    // Test LT_EXPR (<)
    total_sum += test_lt(a_uint, b_uint, N);
    
    // Test LE_EXPR (<=)
    test_le(a_double, b_double, mask_le, N);
    for (int i = 0; i < N; ++i) {
        total_sum += mask_le[i];
    }
    
    // Print result to prevent dead code elimination
    printf("Total sum: %ld\n", total_sum);
    
    return 0;
}

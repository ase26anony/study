#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

// Test functions for each comparison operator
// Each function uses a different pattern to encourage mask optimization

// GT_EXPR: Simple comparison storing to mask array
void test_gt(int *restrict a, int *restrict b, int *restrict mask, int n) {
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] > b[i];  // GT_EXPR
    }
}

// GE_EXPR: Conditional blend using mask
void test_ge(float *restrict a, float *restrict b, 
             float *restrict x, float *restrict y, 
             float *restrict dst, int n) {
    for (int i = 0; i < n; ++i) {
        // GE_EXPR with conditional blend
        dst[i] = (a[i] >= b[i]) ? x[i] : y[i];
    }
}

// LT_EXPR: Masked accumulation
int test_lt(unsigned int *restrict a, unsigned int *restrict b, int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        if (a[i] < b[i]) {  // LT_EXPR
            sum += a[i];
        }
    }
    return sum;
}

// LE_EXPR: Comparison with subsequent bitwise operation
void test_le(double *restrict a, double *restrict b, 
             int *restrict mask, int *restrict result, int n) {
    // First create mask
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] <= b[i];  // LE_EXPR
    }
    
    // Then use mask in bitwise operation to encourage transformation
    for (int i = 0; i < n; ++i) {
        result[i] = mask[i] & 0x1;  // Ensure mask is used
    }
}

// Additional test with mixed types to cover different code paths
void test_mixed_comparisons(int *restrict a, int *restrict b,
                            int *restrict results, int n) {
    // Separate loops for each operator
    for (int i = 0; i < n; ++i) {
        results[i] = a[i] > b[i];  // GT_EXPR
    }
    
    for (int i = 0; i < n; ++i) {
        results[i] |= (a[i] >= b[i]) << 1;  // GE_EXPR
    }
    
    for (int i = 0; i < n; ++i) {
        results[i] |= (a[i] < b[i]) << 2;  // LT_EXPR
    }
    
    for (int i = 0; i < n; ++i) {
        results[i] |= (a[i] <= b[i]) << 3;  // LE_EXPR
    }
}

int main() {
    // Aligned arrays for different data types
    int a_int[N] ALIGNED;
    int b_int[N] ALIGNED;
    int mask_gt[N] ALIGNED;
    int results[N] ALIGNED;
    
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
    
    // Initialize arrays with pattern data
    for (int i = 0; i < N; ++i) {
        a_int[i] = i;
        b_int[i] = N - i;
        a_float[i] = i * 0.5f;
        b_float[i] = (N - i) * 0.5f;
        x_float[i] = i * 1.0f;
        y_float[i] = i * 2.0f;
        a_uint[i] = i * 2;
        b_uint[i] = i * 3;
        a_double[i] = i * 0.25;
        b_double[i] = (N - i) * 0.25;
    }
    
    int total_sum = 0;
    
    // Test GT_EXPR - integer comparison
    test_gt(a_int, b_int, mask_gt, N);
    for (int i = 0; i < N; ++i) total_sum += mask_gt[i];
    
    // Test GE_EXPR - float comparison with blend
    test_ge(a_float, b_float, x_float, y_float, dst_ge, N);
    for (int i = 0; i < N; ++i) total_sum += (int)dst_ge[i];
    
    // Test LT_EXPR - unsigned comparison with accumulation
    total_sum += test_lt(a_uint, b_uint, N);
    
    // Test LE_EXPR - double comparison with bitwise use
    test_le(a_double, b_double, mask_le, result_le, N);
    for (int i = 0; i < N; ++i) total_sum += result_le[i];
    
    // Test mixed comparisons
    test_mixed_comparisons(a_int, b_int, results, N);
    for (int i = 0; i < N; ++i) total_sum += results[i];
    
    printf("Total sum: %d\n", total_sum);
    
    // Prevent dead code elimination
    volatile int dummy = total_sum;
    
    return 0;
}

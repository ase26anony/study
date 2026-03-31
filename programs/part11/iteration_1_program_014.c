#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

// Test functions for each comparison operator
void test_gt(int *restrict a, int *restrict b, int *restrict mask, int n) {
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] > b[i];  // GT_EXPR
    }
}

void test_ge(float *restrict a, float *restrict b, float *restrict x, 
             float *restrict y, float *restrict dst, int n) {
    for (int i = 0; i < n; ++i) {
        // GE_EXPR with conditional blend
        dst[i] = (a[i] >= b[i]) ? x[i] : y[i];
    }
}

int test_lt(unsigned int *restrict a, unsigned int *restrict b, int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        // LT_EXPR with conditional accumulation
        if (a[i] < b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

void test_le(double *restrict a, double *restrict b, int *restrict mask, 
             int *restrict result, int n) {
    // LE_EXPR with mask generation
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] <= b[i];
    }
    
    // Use mask in bitwise operation to encourage transformation
    for (int i = 0; i < n; ++i) {
        result[i] = mask[i] & 0x1;
    }
}

// Additional test with mixed types to stress different pathways
void test_mixed_comparisons(int8_t *restrict a8, int8_t *restrict b8,
                           int16_t *restrict a16, int16_t *restrict b16,
                           int *restrict results, int n) {
    for (int i = 0; i < n; ++i) {
        // Mix of comparison operators
        int r1 = a8[i] > b8[i];    // GT_EXPR
        int r2 = a16[i] >= b16[i]; // GE_EXPR
        int r3 = a8[i] < b8[i];    // LT_EXPR
        int r4 = a16[i] <= b16[i]; // LE_EXPR
        
        // Combine results
        results[i] = r1 + r2 + r3 + r4;
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
    
    int8_t a_int8[N] ALIGNED;
    int8_t b_int8[N] ALIGNED;
    int16_t a_int16[N] ALIGNED;
    int16_t b_int16[N] ALIGNED;
    int mixed_results[N] ALIGNED;
    
    // Initialize arrays with pattern data
    for (int i = 0; i < N; ++i) {
        a_int[i] = i;
        b_int[i] = N - i;
        
        a_float[i] = i * 1.5f;
        b_float[i] = i * 0.7f;
        x_float[i] = i * 2.0f;
        y_float[i] = i * 3.0f;
        
        a_uint[i] = i * 2;
        b_uint[i] = i * 3;
        
        a_double[i] = i * 1.1;
        b_double[i] = i * 0.9;
        
        a_int8[i] = (i % 256) - 128;
        b_int8[i] = ((i + 37) % 256) - 128;
        a_int16[i] = (i % 65536) - 32768;
        b_int16[i] = ((i + 123) % 65536) - 32768;
    }
    
    int total_sum = 0;
    
    // Test GT_EXPR (line 12216-12218)
    test_gt(a_int, b_int, mask_gt, N);
    for (int i = 0; i < N; ++i) total_sum += mask_gt[i];
    
    // Test GE_EXPR (line 12219-12221)
    test_ge(a_float, b_float, x_float, y_float, dst_ge, N);
    for (int i = 0; i < N; ++i) total_sum += (int)dst_ge[i];
    
    // Test LT_EXPR (line 12222-12226)
    total_sum += test_lt(a_uint, b_uint, N);
    
    // Test LE_EXPR (line 12227-12233)
    test_le(a_double, b_double, mask_le, result_le, N);
    for (int i = 0; i < N; ++i) total_sum += result_le[i];
    
    // Test mixed comparisons
    test_mixed_comparisons(a_int8, b_int8, a_int16, b_int16, mixed_results, N);
    for (int i = 0; i < N; ++i) total_sum += mixed_results[i];
    
    printf("Total sum: %d\n", total_sum);
    
    return 0;
}

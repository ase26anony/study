#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define ALIGN __attribute__((aligned(32)))

// Test functions for each comparison operator
// Each function is defined separately to ensure they're analyzed independently

// GT_EXPR: greater than
void test_gt(int *restrict a, int *restrict b, int *restrict mask, int n) {
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] > b[i];  // GT_EXPR
    }
}

// GE_EXPR: greater than or equal
void test_ge(float *restrict a, float *restrict b, float *restrict x, 
             float *restrict y, float *restrict dst, int n) {
    for (int i = 0; i < n; ++i) {
        // Conditional blend using GE_EXPR
        dst[i] = (a[i] >= b[i]) ? x[i] : y[i];  // GE_EXPR
    }
}

// LT_EXPR: less than
long test_lt(unsigned int *restrict a, unsigned int *restrict b, int n) {
    long sum = 0;
    for (int i = 0; i < n; ++i) {
        if (a[i] < b[i]) {  // LT_EXPR
            sum += a[i];
        }
    }
    return sum;
}

// LE_EXPR: less than or equal
void test_le(double *restrict a, double *restrict b, int *restrict mask, 
             int *restrict result, int n) {
    // First compute mask
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] <= b[i];  // LE_EXPR
    }
    
    // Then use mask in bitwise operation to ensure transformation
    for (int i = 0; i < n; ++i) {
        result[i] = mask[i] & (int)a[i];  // Use mask in bitwise operation
    }
}

// Additional test with mixed types to cover more cases
void test_mixed_gt_float(float *restrict a, float *restrict b, 
                         int *restrict mask, int n) {
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] > b[i];  // GT_EXPR with floats
    }
}

void test_mixed_le_int(int *restrict a, int *restrict b, 
                       int *restrict dst, int n) {
    for (int i = 0; i < n; ++i) {
        dst[i] = (a[i] <= b[i]) ? a[i] : b[i];  // LE_EXPR with conditional
    }
}

int main() {
    // Aligned arrays for different data types
    int a_int[N] ALIGN;
    int b_int[N] ALIGN;
    int mask_gt[N] ALIGN;
    int mask_le[N] ALIGN;
    int result_int[N] ALIGN;
    
    float a_float[N] ALIGN;
    float b_float[N] ALIGN;
    float x_float[N] ALIGN;
    float y_float[N] ALIGN;
    float dst_float[N] ALIGN;
    
    unsigned int a_uint[N] ALIGN;
    unsigned int b_uint[N] ALIGN;
    
    double a_double[N] ALIGN;
    double b_double[N] ALIGN;
    
    // Initialize with pseudo-random data
    srand(time(NULL));
    for (int i = 0; i < N; ++i) {
        // Integer arrays
        a_int[i] = rand() % 1000;
        b_int[i] = rand() % 1000;
        
        // Float arrays
        a_float[i] = (float)(rand() % 1000) / 10.0f;
        b_float[i] = (float)(rand() % 1000) / 10.0f;
        x_float[i] = (float)(rand() % 1000) / 10.0f;
        y_float[i] = (float)(rand() % 1000) / 10.0f;
        
        // Unsigned arrays
        a_uint[i] = rand() % 1000;
        b_uint[i] = rand() % 1000;
        
        // Double arrays
        a_double[i] = (double)(rand() % 1000) / 10.0;
        b_double[i] = (double)(rand() % 1000) / 10.0;
    }
    
    long total_sum = 0;
    
    // Test 1: GT_EXPR with integers
    test_gt(a_int, b_int, mask_gt, N);
    for (int i = 0; i < N; ++i) total_sum += mask_gt[i];
    
    // Test 2: GE_EXPR with floats and conditional blend
    test_ge(a_float, b_float, x_float, y_float, dst_float, N);
    for (int i = 0; i < N; ++i) total_sum += (int)dst_float[i];
    
    // Test 3: LT_EXPR with unsigned integers and conditional accumulation
    total_sum += test_lt(a_uint, b_uint, N);
    
    // Test 4: LE_EXPR with doubles and mask usage
    test_le(a_double, b_double, mask_le, result_int, N);
    for (int i = 0; i < N; ++i) total_sum += result_int[i];
    
    // Additional tests to increase coverage
    test_mixed_gt_float(a_float, b_float, mask_gt, N);
    for (int i = 0; i < N; ++i) total_sum += mask_gt[i];
    
    test_mixed_le_int(a_int, b_int, result_int, N);
    for (int i = 0; i < N; ++i) total_sum += result_int[i];
    
    // Print result to prevent dead code elimination
    printf("Total sum: %ld\n", total_sum);
    
    return 0;
}

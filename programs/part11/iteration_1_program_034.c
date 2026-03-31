#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define ALIGN __attribute__((aligned(32)))

// Test functions for each comparison operator
void test_gt(int *restrict a, int *restrict b, int *restrict mask, int n) {
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] > b[i];  // GT_EXPR
    }
}

void test_ge(float *restrict a, float *restrict b, float *restrict dst, 
             float *restrict x, float *restrict y, int n) {
    for (int i = 0; i < n; ++i) {
        // GE_EXPR with conditional blend
        dst[i] = (a[i] >= b[i]) ? x[i] : y[i];
    }
}

void test_lt(unsigned int *restrict a, unsigned int *restrict b, 
             unsigned int *restrict sum_ptr, int n) {
    unsigned int sum = *sum_ptr;
    for (int i = 0; i < n; ++i) {
        // LT_EXPR with conditional accumulation
        if (a[i] < b[i]) {
            sum += a[i];
        }
    }
    *sum_ptr = sum;
}

void test_le(double *restrict a, double *restrict b, int *restrict mask, 
             double *restrict result, int n) {
    for (int i = 0; i < n; ++i) {
        // LE_EXPR
        mask[i] = a[i] <= b[i];
    }
    
    // Use mask in bitwise operations to encourage transformation
    for (int i = 0; i < n; ++i) {
        result[i] = mask[i] ? a[i] : b[i];
    }
}

// Additional test with mixed types to stress different code paths
void test_mixed_comparisons(int *restrict a, int *restrict b, 
                            int *restrict results, int n) {
    // Separate loops for each operator to ensure vectorization
    for (int i = 0; i < n; ++i) {
        results[i] = a[i] > b[i];  // GT_EXPR
    }
    
    for (int i = 0; i < n; ++i) {
        results[i] |= (a[i] >= b[i]) << 1;  // GE_EXPR
    }
    
    for (int i = 0; i < n; ++i) {
        results[i] |= (a[i] < b[i]) << 2;   // LT_EXPR
    }
    
    for (int i = 0; i < n; ++i) {
        results[i] |= (a[i] <= b[i]) << 3;  // LE_EXPR
    }
}

int main() {
    // Aligned arrays for different data types
    int a_int[N] ALIGN;
    int b_int[N] ALIGN;
    int mask_gt[N] ALIGN;
    int results_mixed[N] ALIGN;
    
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
    int mask_le[N] ALIGN;
    double result_le[N] ALIGN;
    
    // Initialize with random data
    srand(time(NULL));
    for (int i = 0; i < N; ++i) {
        a_int[i] = rand() % 1000;
        b_int[i] = rand() % 1000;
        
        a_float[i] = (float)rand() / RAND_MAX * 1000.0f;
        b_float[i] = (float)rand() / RAND_MAX * 1000.0f;
        x_float[i] = (float)rand() / RAND_MAX * 1000.0f;
        y_float[i] = (float)rand() / RAND_MAX * 1000.0f;
        
        a_uint[i] = rand() % 1000;
        b_uint[i] = rand() % 1000;
        
        a_double[i] = (double)rand() / RAND_MAX * 1000.0;
        b_double[i] = (double)rand() / RAND_MAX * 1000.0;
    }
    
    // Execute all test functions
    test_gt(a_int, b_int, mask_gt, N);
    test_ge(a_float, b_float, dst_ge, x_float, y_float, N);
    test_lt(a_uint, b_uint, &sum_lt, N);
    test_le(a_double, b_double, mask_le, result_le, N);
    test_mixed_comparisons(a_int, b_int, results_mixed, N);
    
    // Prevent dead code elimination by computing and printing a checksum
    int checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += mask_gt[i];
        checksum += (int)dst_ge[i];
        checksum += results_mixed[i];
        checksum += (int)result_le[i];
    }
    checksum += sum_lt;
    
    printf("Checksum: %d\n", checksum);
    printf("All comparison tests completed.\n");
    
    return 0;
}

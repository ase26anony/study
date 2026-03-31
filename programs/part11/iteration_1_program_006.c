#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

// Function declarations
void test_gt(int *restrict a, int *restrict b, int *restrict mask, int n);
void test_ge(float *restrict a, float *restrict b, float *restrict dst, 
             float *restrict x, float *restrict y, int n);
unsigned long test_lt(unsigned int *restrict a, unsigned int *restrict b, int n);
void test_le(double *restrict a, double *restrict b, int *restrict mask, int n);

// Non-inlined functions to ensure separate compilation context
__attribute__((noinline))
void test_gt(int *restrict a, int *restrict b, int *restrict mask, int n) {
    // GT_EXPR: a[i] > b[i]
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] > b[i];
    }
}

__attribute__((noinline))
void test_ge(float *restrict a, float *restrict b, float *restrict dst,
             float *restrict x, float *restrict y, int n) {
    // GE_EXPR: a[i] >= b[i] with conditional blend
    for (int i = 0; i < n; ++i) {
        dst[i] = (a[i] >= b[i]) ? x[i] : y[i];
    }
}

__attribute__((noinline))
unsigned long test_lt(unsigned int *restrict a, unsigned int *restrict b, int n) {
    // LT_EXPR: a[i] < b[i] with conditional accumulation
    unsigned long sum = 0;
    for (int i = 0; i < n; ++i) {
        if (a[i] < b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

__attribute__((noinline))
void test_le(double *restrict a, double *restrict b, int *restrict mask, int n) {
    // LE_EXPR: a[i] <= b[i]
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] <= b[i];
    }
    
    // Additional bitwise operation to encourage mask transformation
    for (int i = 0; i < n; ++i) {
        // Use the mask in a bitwise operation
        mask[i] = mask[i] & 0x1;
    }
}

// Additional test functions for different data types
__attribute__((noinline))
void test_gt_float(float *restrict a, float *restrict b, int *restrict mask, int n) {
    // GT_EXPR with floating point
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] > b[i];
    }
}

__attribute__((noinline))
void test_ge_int(int *restrict a, int *restrict b, int *restrict dst,
                 int *restrict x, int *restrict y, int n) {
    // GE_EXPR with integers
    for (int i = 0; i < n; ++i) {
        dst[i] = (a[i] >= b[i]) ? x[i] : y[i];
    }
}

int main() {
    // Aligned arrays for different data types
    int a_int[N] ALIGNED;
    int b_int[N] ALIGNED;
    int mask_gt[N] ALIGNED;
    int mask_le[N] ALIGNED;
    
    float a_float[N] ALIGNED;
    float b_float[N] ALIGNED;
    float x_float[N] ALIGNED;
    float y_float[N] ALIGNED;
    float dst_ge[N] ALIGNED;
    
    unsigned int a_uint[N] ALIGNED;
    unsigned int b_uint[N] ALIGNED;
    
    double a_double[N] ALIGNED;
    double b_double[N] ALIGNED;
    
    // Initialize arrays with pseudo-random data
    srand(time(NULL));
    for (int i = 0; i < N; ++i) {
        a_int[i] = rand() % 1000;
        b_int[i] = rand() % 1000;
        a_float[i] = (float)(rand() % 1000) / 10.0f;
        b_float[i] = (float)(rand() % 1000) / 10.0f;
        x_float[i] = (float)(rand() % 1000) / 10.0f;
        y_float[i] = (float)(rand() % 1000) / 10.0f;
        a_uint[i] = rand() % 1000;
        b_uint[i] = rand() % 1000;
        a_double[i] = (double)(rand() % 1000) / 10.0;
        b_double[i] = (double)(rand() % 1000) / 10.0;
    }
    
    // Test GT_EXPR with integers
    test_gt(a_int, b_int, mask_gt, N);
    
    // Test GE_EXPR with floats and conditional blend
    test_ge(a_float, b_float, dst_ge, x_float, y_float, N);
    
    // Test LT_EXPR with unsigned integers and conditional accumulation
    unsigned long sum_lt = test_lt(a_uint, b_uint, N);
    
    // Test LE_EXPR with doubles
    test_le(a_double, b_double, mask_le, N);
    
    // Additional tests for different type combinations
    test_gt_float(a_float, b_float, mask_gt, N);
    
    int dst_int[N] ALIGNED;
    int x_int[N] ALIGNED;
    int y_int[N] ALIGNED;
    for (int i = 0; i < N; ++i) {
        x_int[i] = rand() % 1000;
        y_int[i] = rand() % 1000;
    }
    test_ge_int(a_int, b_int, dst_int, x_int, y_int, N);
    
    // Prevent dead code elimination by computing and printing a checksum
    unsigned long checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += mask_gt[i];
        checksum += (unsigned long)dst_ge[i];
        checksum += mask_le[i];
        checksum += dst_int[i];
    }
    checksum += sum_lt;
    
    printf("Checksum: %lu\n", checksum);
    printf("All comparison tests completed.\n");
    
    return 0;
}

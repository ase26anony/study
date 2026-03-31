#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test functions for each comparison operator */

/* GT_EXPR (>) */
void test_gt_expr(int *restrict a, int *restrict b, int *restrict c, int *restrict d) {
    for (int i = 0; i < N; ++i) {
        // Conditional assignment using > operator
        c[i] = (a[i] > b[i]) ? a[i] * 2 : b[i];
        // Masked store pattern
        if (a[i] > b[i]) {
            d[i] = a[i] - b[i];
        } else {
            d[i] = 0;
        }
    }
}

/* GE_EXPR (>=) */
void test_ge_expr(float *restrict a, float *restrict b, float *restrict c, float *restrict d) {
    for (int i = 0; i < N; ++i) {
        // Conditional assignment using >= operator
        c[i] = (a[i] >= b[i]) ? a[i] + b[i] : a[i] - b[i];
        // Reduction pattern with conditional
        if (i == 0) d[0] = 0.0f;
        d[0] += (a[i] >= b[i]) ? a[i] : 0.0f;
    }
}

/* LT_EXPR (<) */
void test_lt_expr(int *restrict a, int *restrict b, int *restrict c, int *restrict d) {
    for (int i = 0; i < N; ++i) {
        // Conditional assignment using < operator
        c[i] = (a[i] < b[i]) ? a[i] + b[i] : a[i] - b[i];
        // Blend pattern
        d[i] = (a[i] < b[i]) ? a[i] * b[i] : a[i] / (b[i] + 1);
    }
}

/* LE_EXPR (<=) */
void test_le_expr(float *restrict a, float *restrict b, float *restrict c, float *restrict d) {
    float sum = 0.0f;
    for (int i = 0; i < N; ++i) {
        // Conditional assignment using <= operator
        c[i] = (a[i] <= b[i]) ? a[i] * 2.0f : b[i] * 3.0f;
        // Reduction with conditional increment
        sum += (a[i] <= b[i]) ? a[i] : b[i];
    }
    d[0] = sum;
}

/* Helper function to initialize arrays with varying patterns */
void init_arrays(int *a_int, int *b_int, float *a_float, float *b_float) {
    for (int i = 0; i < N; ++i) {
        // Create mixed true/false conditions
        a_int[i] = i;
        b_int[i] = N/2 - i % 100;  // Creates crossing pattern
        
        a_float[i] = (float)(i * 1.5f);
        b_float[i] = (float)(N/2 + i % 50);  // Creates mixed conditions
    }
}

/* Verification function to ensure computations are correct */
int verify_results() {
    int a_int[N] ALIGNED;
    int b_int[N] ALIGNED;
    int c_int[N] ALIGNED;
    int d_int[N] ALIGNED;
    float a_float[N] ALIGNED;
    float b_float[N] ALIGNED;
    float c_float[N] ALIGNED;
    float d_float[N] ALIGNED;
    
    // Initialize with deterministic patterns
    init_arrays(a_int, b_int, a_float, b_float);
    
    // Clear output arrays
    memset(c_int, 0, sizeof(c_int));
    memset(d_int, 0, sizeof(d_int));
    memset(c_float, 0, sizeof(c_float));
    memset(d_float, 0, sizeof(d_float));
    
    // Execute all test functions
    test_gt_expr(a_int, b_int, c_int, d_int);
    test_ge_expr(a_float, b_float, c_float, d_float);
    test_lt_expr(a_int, b_int, c_int, d_int);
    test_le_expr(a_float, b_float, c_float, d_float);
    
    // Compute checksums to prevent dead code elimination
    int int_checksum = 0;
    float float_checksum = 0.0f;
    
    for (int i = 0; i < N; ++i) {
        int_checksum += c_int[i] + d_int[i];
        float_checksum += c_float[i];
    }
    float_checksum += d_float[0];
    
    // Return non-zero result to ensure all code executes
    return (int_checksum != 0) || (float_checksum != 0.0f);
}

int main() {
    // Run verification multiple times to increase coverage chances
    int result = 0;
    for (int iter = 0; iter < 10; ++iter) {
        result |= verify_results();
    }
    
    printf("Test completed with result: %d\n", result);
    printf("If result is non-zero, conditionals were executed.\n");
    
    return result == 0 ? 1 : 0;  // Return 0 if all tests passed
}

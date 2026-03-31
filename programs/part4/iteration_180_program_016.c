#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test functions for each comparison operator */

/* GT_EXPR (>) */
void test_gt_expr(int *restrict a, int *restrict b, int *restrict c, int *restrict d) {
    for (int i = 0; i < N; i++) {
        // Conditional assignment using > operator
        // This should generate vector masks converted to bit operations
        c[i] = (a[i] > b[i]) ? a[i] * 2 : b[i] / 2;
        
        // Additional masked operation to ensure mask usage
        if (a[i] > b[i]) {
            d[i] = a[i] - b[i];
        } else {
            d[i] = b[i] - a[i];
        }
    }
}

/* GE_EXPR (>=) */
void test_ge_expr(float *restrict a, float *restrict b, float *restrict c, float *restrict d) {
    float sum = 0.0f;
    for (int i = 0; i < N; i++) {
        // Conditional reduction using >= operator
        sum += (a[i] >= b[i]) ? a[i] : b[i];
        
        // Masked store with >=
        if (a[i] >= b[i]) {
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] + b[i];
        }
        
        // Blend operation
        d[i] = (a[i] >= b[i]) ? a[i] : b[i];
    }
    // Use sum to prevent dead code elimination
    c[0] += sum;
}

/* LT_EXPR (<) */
void test_lt_expr(short *restrict a, short *restrict b, short *restrict c, short *restrict d) {
    for (int i = 0; i < N; i++) {
        // Conditional assignment using < operator
        // This may trigger operand swapping in the transformation
        c[i] = (a[i] < b[i]) ? a[i] + b[i] : a[i] - b[i];
        
        // Complex masked operation
        if (a[i] < b[i]) {
            d[i] = a[i] * 3;
        } else {
            d[i] = b[i] * 2;
        }
    }
}

/* LE_EXPR (<=) */
void test_le_expr(double *restrict a, double *restrict b, double *restrict c, double *restrict d) {
    double prod = 1.0;
    for (int i = 0; i < N; i++) {
        // Conditional operation using <= operator
        // This may trigger operand swapping
        c[i] = (a[i] <= b[i]) ? a[i] * 2.0 : b[i] / 2.0;
        
        // Reduction with conditional
        prod *= (a[i] <= b[i]) ? a[i] : b[i];
        
        // Another masked operation
        if (a[i] <= b[i]) {
            d[i] = a[i] + 1.0;
        } else {
            d[i] = b[i] - 1.0;
        }
    }
    // Use prod to prevent optimization
    d[0] *= prod;
}

/* Mixed test with all operators in one loop */
void test_mixed_operators(int *restrict a, int *restrict b, int *restrict c, int *restrict d) {
    for (int i = 0; i < N; i++) {
        // Use all four comparison operators in the same loop
        // This increases chances of hitting all cases
        int gt_mask = (a[i] > b[i]) ? 1 : 0;
        int ge_mask = (a[i] >= b[i]) ? 1 : 0;
        int lt_mask = (a[i] < b[i]) ? 1 : 0;
        int le_mask = (a[i] <= b[i]) ? 1 : 0;
        
        // Combine masks in ways that might trigger bit operations
        c[i] = (gt_mask & ge_mask) ? a[i] : b[i];
        d[i] = (lt_mask | le_mask) ? a[i] + b[i] : a[i] - b[i];
    }
}

/* Initialize arrays with varying patterns to ensure mix of true/false comparisons */
void init_arrays(int *a_int, int *b_int, 
                 float *a_float, float *b_float,
                 short *a_short, short *b_short,
                 double *a_double, double *b_double) {
    for (int i = 0; i < N; i++) {
        // Create varying patterns for different comparison outcomes
        a_int[i] = i;
        b_int[i] = N/2 - i % 100;  // Mix of > and < cases
        
        a_float[i] = (float)(i * 1.5f);
        b_float[i] = (float)(N - i);
        
        a_short[i] = (short)(i % 32767);
        b_short[i] = (short)((i + 50) % 32767);
        
        a_double[i] = (double)i * 0.75;
        b_double[i] = (double)(N/2) * sin(i * 0.01);
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
    
    short a_short[N] ALIGNED;
    short b_short[N] ALIGNED;
    short c_short[N] ALIGNED;
    short d_short[N] ALIGNED;
    
    double a_double[N] ALIGNED;
    double b_double[N] ALIGNED;
    double c_double[N] ALIGNED;
    double d_double[N] ALIGNED;
    
    // Initialize with varying patterns
    init_arrays(a_int, b_int, a_float, b_float, 
                a_short, b_short, a_double, b_double);
    
    // Clear output arrays
    memset(c_int, 0, sizeof(c_int));
    memset(d_int, 0, sizeof(d_int));
    memset(c_float, 0, sizeof(c_float));
    memset(d_float, 0, sizeof(d_float));
    memset(c_short, 0, sizeof(c_short));
    memset(d_short, 0, sizeof(d_short));
    memset(c_double, 0, sizeof(c_double));
    memset(d_double, 0, sizeof(d_double));
    
    // Run all test functions
    test_gt_expr(a_int, b_int, c_int, d_int);
    test_ge_expr(a_float, b_float, c_float, d_float);
    test_lt_expr(a_short, b_short, c_short, d_short);
    test_le_expr(a_double, b_double, c_double, d_double);
    test_mixed_operators(a_int, b_int, c_int, d_int);
    
    // Compute checksums to ensure computations happened
    int int_checksum = 0;
    float float_checksum = 0.0f;
    short short_checksum = 0;
    double double_checksum = 0.0;
    
    for (int i = 0; i < N; i++) {
        int_checksum += c_int[i] + d_int[i];
        float_checksum += c_float[i] + d_float[i];
        short_checksum += c_short[i] + d_short[i];
        double_checksum += c_double[i] + d_double[i];
    }
    
    // Print checksums (prevents dead code elimination)
    printf("Integer checksum: %d\n", int_checksum);
    printf("Float checksum: %f\n", float_checksum);
    printf("Short checksum: %d\n", (int)short_checksum);
    printf("Double checksum: %lf\n", double_checksum);
    
    return (int_checksum != 0) && (short_checksum != 0);
}

int main() {
    printf("Testing vectorizable loops with comparison operators...\n");
    
    int result = verify_results();
    
    if (result) {
        printf("All tests completed successfully.\n");
        return 0;
    } else {
        printf("Test verification failed.\n");
        return 1;
    }
}

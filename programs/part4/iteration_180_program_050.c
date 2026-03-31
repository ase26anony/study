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
        // This should generate GT_EXPR comparison
        c[i] = (a[i] > b[i]) ? a[i] * 2 : b[i];
        
        // Additional masked operation to ensure vectorization
        if (a[i] > b[i]) {
            d[i] = a[i] - b[i];
        } else {
            d[i] = b[i] - a[i];
        }
    }
}

/* GE_EXPR (>=) */
void test_ge_expr(float *restrict a, float *restrict b, float *restrict c, float *restrict d) {
    for (int i = 0; i < N; i++) {
        // Conditional assignment using >= operator
        // This should generate GE_EXPR comparison
        c[i] = (a[i] >= b[i]) ? a[i] + b[i] : a[i] - b[i];
        
        // Reduction-like pattern with conditional
        if (a[i] >= b[i]) {
            d[i] = a[i] * b[i];
        } else {
            d[i] = a[i] / (b[i] + 1.0f);
        }
    }
}

/* LT_EXPR (<) */
void test_lt_expr(short *restrict a, short *restrict b, int *restrict c, int *restrict d) {
    for (int i = 0; i < N; i++) {
        // Conditional assignment using < operator
        // This should generate LT_EXPR comparison
        c[i] = (a[i] < b[i]) ? a[i] + b[i] : a[i] - b[i];
        
        // Masked store with < comparison
        if (a[i] < b[i]) {
            d[i] = a[i] * b[i] * 2;
        } else {
            d[i] = a[i] + b[i];
        }
    }
}

/* LE_EXPR (<=) */
void test_le_expr(double *restrict a, double *restrict b, double *restrict c, double *restrict d) {
    for (int i = 0; i < N; i++) {
        // Conditional assignment using <= operator
        // This should generate LE_EXPR comparison
        c[i] = (a[i] <= b[i]) ? a[i] * 3.0 : b[i] * 2.0;
        
        // Complex conditional with <=
        if (a[i] <= b[i]) {
            d[i] = (a[i] + b[i]) * 0.5;
        } else {
            d[i] = a[i] - b[i];
        }
    }
}

/* Additional test with mixed comparisons in same loop */
void test_mixed_comparisons(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        // Use all four comparison operators in same loop
        if (a[i] > b[i]) {
            c[i] = 1;
        } else if (a[i] >= b[i] + 10) {
            c[i] = 2;
        } else if (a[i] < b[i] - 5) {
            c[i] = 3;
        } else if (a[i] <= b[i]) {
            c[i] = 4;
        } else {
            c[i] = 0;
        }
    }
}

/* Reduction pattern with conditional */
int test_reduction_with_ge(int *restrict a, int *restrict b) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        // Conditional reduction using >=
        sum += (a[i] >= b[i]) ? a[i] : 0;
    }
    return sum;
}

int main() {
    // Aligned arrays to help vectorization
    ALIGNED int a_int[N], b_int[N], c_int[N], d_int[N];
    ALIGNED float a_float[N], b_float[N], c_float[N], d_float[N];
    ALIGNED short a_short[N], b_short[N];
    ALIGNED double a_double[N], b_double[N], c_double[N], d_double[N];
    ALIGNED int mixed_c[N];
    
    // Initialize arrays with varying patterns to ensure mix of true/false comparisons
    for (int i = 0; i < N; i++) {
        // Integer arrays
        a_int[i] = i;
        b_int[i] = N/2;
        c_int[i] = 0;
        d_int[i] = 0;
        
        // Float arrays
        a_float[i] = (float)(i - N/2);
        b_float[i] = (float)(i % 100);
        c_float[i] = 0.0f;
        d_float[i] = 0.0f;
        
        // Short arrays
        a_short[i] = (short)(i * 3);
        b_short[i] = (short)(i * 2 + 100);
        
        // Double arrays
        a_double[i] = (double)(i * 1.5);
        b_double[i] = (double)(i * 1.2 + 50.0);
        c_double[i] = 0.0;
        d_double[i] = 0.0;
        
        // Mixed test array
        mixed_c[i] = 0;
    }
    
    printf("Testing GT_EXPR (>)...\n");
    test_gt_expr(a_int, b_int, c_int, d_int);
    
    printf("Testing GE_EXPR (>=)...\n");
    test_ge_expr(a_float, b_float, c_float, d_float);
    
    printf("Testing LT_EXPR (<)...\n");
    test_lt_expr(a_short, b_short, c_int, d_int);
    
    printf("Testing LE_EXPR (<=)...\n");
    test_le_expr(a_double, b_double, c_double, d_double);
    
    printf("Testing mixed comparisons...\n");
    test_mixed_comparisons(a_int, b_int, mixed_c);
    
    printf("Testing reduction with GE_EXPR...\n");
    int reduction_sum = test_reduction_with_ge(a_int, b_int);
    
    // Compute checksums to ensure computations aren't optimized away
    int int_checksum = 0;
    float float_checksum = 0.0f;
    double double_checksum = 0.0;
    
    for (int i = 0; i < N; i++) {
        int_checksum += c_int[i] + d_int[i] + mixed_c[i];
        float_checksum += c_float[i] + d_float[i];
        double_checksum += c_double[i] + d_double[i];
    }
    
    printf("Results:\n");
    printf("  Integer checksum: %d\n", int_checksum);
    printf("  Float checksum: %f\n", float_checksum);
    printf("  Double checksum: %f\n", double_checksum);
    printf("  Reduction sum: %d\n", reduction_sum);
    
    // Verify at least some conditionals were true/false
    int gt_count = 0, ge_count = 0, lt_count = 0, le_count = 0;
    for (int i = 0; i < N; i++) {
        if (a_int[i] > b_int[i]) gt_count++;
        if (a_float[i] >= b_float[i]) ge_count++;
        if (a_short[i] < b_short[i]) lt_count++;
        if (a_double[i] <= b_double[i]) le_count++;
    }
    
    printf("Conditional statistics:\n");
    printf("  GT true: %d/%d\n", gt_count, N);
    printf("  GE true: %d/%d\n", ge_count, N);
    printf("  LT true: %d/%d\n", lt_count, N);
    printf("  LE true: %d/%d\n", le_count, N);
    
    return 0;
}

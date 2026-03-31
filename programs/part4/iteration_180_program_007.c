#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test functions for each comparison type */

/* GT_EXPR (> operator) */
void test_gt_expr(int *restrict a, int *restrict b, int *restrict c, int *restrict d) {
    for (int i = 0; i < N; i++) {
        // Conditional assignment using > comparison
        // This should generate vector masks converted to bit operations
        c[i] = (a[i] > b[i]) ? a[i] * 2 : b[i] / 2;
        
        // Additional masked operation to ensure vectorization
        if (a[i] > b[i]) {
            d[i] = a[i] - b[i];
        } else {
            d[i] = b[i] - a[i];
        }
    }
}

/* GE_EXPR (>= operator) */
void test_ge_expr(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        // Conditional reduction-like pattern
        c[i] = (a[i] >= b[i]) ? a[i] + b[i] : a[i] - b[i];
        
        // Another GE comparison in the same loop
        if (a[i] >= 0.5f * b[i]) {
            c[i] *= 2.0f;
        }
    }
}

/* LT_EXPR (< operator) */
void test_lt_expr(short *restrict a, short *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        // Conditional with < operator - should trigger operand swap
        if (a[i] < b[i]) {
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] + b[i];
        }
        
        // Additional LT comparison with different types
        c[i] += (a[i] < (short)(i % 256)) ? 100 : -100;
    }
}

/* LE_EXPR (<= operator) */
void test_le_expr(double *restrict a, double *restrict b, double *restrict c) {
    for (int i = 0; i < N; i++) {
        // Multiple LE comparisons in same loop
        c[i] = (a[i] <= b[i]) ? a[i] : b[i];
        
        // Nested conditional with LE
        if (a[i] <= 0.0) {
            c[i] = -c[i];
        }
        
        // Another LE comparison to increase vectorization benefit
        if (b[i] <= a[i] * 2.0) {
            c[i] += 1.0;
        }
    }
}

/* Mixed comparisons in one loop to test multiple transformations */
void test_mixed_comparisons(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        // Use all four comparison types in same loop
        int val = 0;
        val += (a[i] > b[i]) ? 1 : 0;    // GT_EXPR
        val += (a[i] >= b[i]) ? 2 : 0;   // GE_EXPR
        val += (a[i] < b[i]) ? 4 : 0;    // LT_EXPR
        val += (a[i] <= b[i]) ? 8 : 0;   // LE_EXPR
        c[i] = val;
    }
}

/* Helper function to verify results */
int verify_results(int *c, int *expected) {
    for (int i = 0; i < N; i++) {
        if (c[i] != expected[i]) {
            return 0;
        }
    }
    return 1;
}

int main() {
    // Aligned arrays for better vectorization
    ALIGNED int a_int[N], b_int[N], c_int[N], d_int[N];
    ALIGNED float a_float[N], b_float[N], c_float[N];
    ALIGNED short a_short[N], b_short[N];
    ALIGNED double a_double[N], b_double[N], c_double[N];
    ALIGNED int mixed_results[N], expected_mixed[N];
    
    // Initialize with varying patterns to ensure mix of true/false comparisons
    for (int i = 0; i < N; i++) {
        // Integer arrays: create crossing pattern
        a_int[i] = i;
        b_int[i] = N/2;
        
        // Float arrays: sine-like pattern
        a_float[i] = (i % 100) * 0.1f;
        b_float[i] = 5.0f + (i % 50) * 0.05f;
        
        // Short arrays: alternating pattern
        a_short[i] = (i % 2 == 0) ? i % 1000 : 500 - (i % 500);
        b_short[i] = 250 + (i % 750);
        
        // Double arrays: increasing/decreasing pattern
        a_double[i] = i * 0.25;
        b_double[i] = 100.0 - i * 0.1;
        
        // Expected results for mixed test
        expected_mixed[i] = 0;
        expected_mixed[i] += (a_int[i] > b_int[i]) ? 1 : 0;
        expected_mixed[i] += (a_int[i] >= b_int[i]) ? 2 : 0;
        expected_mixed[i] += (a_int[i] < b_int[i]) ? 4 : 0;
        expected_mixed[i] += (a_int[i] <= b_int[i]) ? 8 : 0;
    }
    
    printf("Testing GT_EXPR (>) vectorization...\n");
    test_gt_expr(a_int, b_int, c_int, d_int);
    
    printf("Testing GE_EXPR (>=) vectorization...\n");
    test_ge_expr(a_float, b_float, c_float);
    
    printf("Testing LT_EXPR (<) vectorization...\n");
    test_lt_expr(a_short, b_short, c_int);
    
    printf("Testing LE_EXPR (<=) vectorization...\n");
    test_le_expr(a_double, b_double, c_double);
    
    printf("Testing mixed comparisons vectorization...\n");
    test_mixed_comparisons(a_int, b_int, mixed_results);
    
    // Verify mixed results
    if (verify_results(mixed_results, expected_mixed)) {
        printf("All tests completed successfully!\n");
        
        // Compute checksums to prevent dead code elimination
        int checksum = 0;
        for (int i = 0; i < N; i++) {
            checksum += c_int[i] + d_int[i] + (int)c_float[i] + 
                       (int)c_double[i] + mixed_results[i];
        }
        printf("Final checksum: %d\n", checksum);
        return 0;
    } else {
        printf("Verification failed!\n");
        return 1;
    }
}

/* test_vector_comparisons.c
 * Designed to trigger vectorization of comparison operations
 * that map to BIT_NOT_EXPR and BIT_AND_EXPR/BIT_IOR_EXPR patterns
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test functions for each comparison operator */

void test_gt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

void test_ge(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] * 2;
        }
    }
}

void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            c[i] = a[i] | b[i];
        } else {
            c[i] = a[i] & b[i];
        }
    }
}

void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            c[i] = a[i] ^ b[i];
        } else {
            c[i] = a[i] + b[i] * 3;
        }
    }
}

/* Additional tests with floating point to ensure different data types */
void test_gt_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] / (b[i] + 1.0f);
        }
    }
}

void test_ge_float(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

/* Complex pattern to force mask creation */
void test_complex_lt_gt(int *restrict a, int *restrict b, int *restrict c, 
                        int *restrict d, int *restrict e) {
    for (int i = 0; i < N; i++) {
        // Multiple comparisons in same loop
        int cond1 = a[i] < b[i];
        int cond2 = a[i] > c[i];
        d[i] = cond1 ? a[i] : b[i];
        e[i] = cond2 ? c[i] : d[i];
    }
}

/* Main test driver */
int main() {
    // Aligned arrays for better vectorization
    ALIGNED int a_int[N], b_int[N], c_int[N], d_int[N], e_int[N];
    ALIGNED float a_float[N], b_float[N], c_float[N];
    
    // Initialize with patterned data to create varied comparison results
    for (int i = 0; i < N; i++) {
        a_int[i] = i;
        b_int[i] = N - i;
        c_int[i] = 0;
        d_int[i] = i % 64;
        e_int[i] = 0;
        
        a_float[i] = (float)i * 1.5f;
        b_float[i] = (float)(N - i) * 0.7f;
        c_float[i] = 0.0f;
    }
    
    // Execute all test functions
    test_gt(a_int, b_int, c_int);
    test_ge(a_int, b_int, c_int);
    test_lt(a_int, b_int, c_int);
    test_le(a_int, b_int, c_int);
    
    test_gt_float(a_float, b_float, c_float);
    test_ge_float(a_float, b_float, c_float);
    
    test_complex_lt_gt(a_int, b_int, c_int, d_int, e_int);
    
    // Compute checksums to ensure code executes
    int int_sum = 0;
    float float_sum = 0.0f;
    
    for (int i = 0; i < N; i++) {
        int_sum += c_int[i] + d_int[i] + e_int[i];
        float_sum += c_float[i];
    }
    
    printf("Integer checksum: %d\n", int_sum);
    printf("Float checksum: %f\n", float_sum);
    
    // Simple validation
    if (int_sum != 0 || float_sum != 0.0f) {
        printf("Tests completed successfully (non-zero checksums expected)\n");
    }
    
    return 0;
}

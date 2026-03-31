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
        // This should trigger GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR transformation
        c[i] = (a[i] > b[i]) ? (a[i] * 2) : (b[i] + 1);
        
        // Additional masked operation to ensure vectorization
        if (a[i] > b[i]) {
            d[i] = a[i] - b[i];
        } else {
            d[i] = b[i] - a[i];
        }
    }
}

/* GE_EXPR (>=) */
void test_ge_expr(int *restrict a, int *restrict b, int *restrict c, int *restrict d) {
    for (int i = 0; i < N; i++) {
        // Conditional assignment using >= operator
        // This should trigger GE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR transformation
        c[i] = (a[i] >= b[i]) ? (a[i] * 3) : (b[i] * 2);
        
        // Reduction pattern with conditional
        if (a[i] >= b[i]) {
            d[i] = d[i] + a[i];
        }
    }
}

/* LT_EXPR (<) */
void test_lt_expr(int *restrict a, int *restrict b, int *restrict c, int *restrict d) {
    for (int i = 0; i < N; i++) {
        // Conditional assignment using < operator
        // This should trigger LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR + swap transformation
        c[i] = (a[i] < b[i]) ? (a[i] + b[i]) : (a[i] - b[i]);
        
        // Masked store pattern
        if (a[i] < b[i]) {
            d[i] = a[i] * b[i];
        }
    }
}

/* LE_EXPR (<=) */
void test_le_expr(int *restrict a, int *restrict b, int *restrict c, int *restrict d) {
    for (int i = 0; i < N; i++) {
        // Conditional assignment using <= operator
        // This should trigger LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR + swap transformation
        c[i] = (a[i] <= b[i]) ? (a[i] << 1) : (b[i] >> 1);
        
        // Complex conditional with arithmetic
        if (a[i] <= b[i]) {
            d[i] = (a[i] + b[i]) * 2;
        } else {
            d[i] = (a[i] - b[i]) / 2;
        }
    }
}

/* Additional test with floating point to ensure different data types are covered */
void test_float_comparisons(float *restrict fa, float *restrict fb, float *restrict fc) {
    for (int i = 0; i < N; i++) {
        // Mix of different comparison operators with floats
        if (fa[i] > fb[i]) {
            fc[i] = fa[i] * 2.0f;
        } else if (fa[i] >= fb[i]) {
            fc[i] = fa[i] + fb[i];
        } else if (fa[i] < fb[i]) {
            fc[i] = fb[i] - fa[i];
        } else if (fa[i] <= fb[i]) {
            fc[i] = fa[i] / 2.0f;
        }
    }
}

/* Helper function to initialize arrays with varying patterns */
void init_arrays(int *a, int *b, int *c, int *d, 
                 float *fa, float *fb, float *fc) {
    for (int i = 0; i < N; i++) {
        // Create varying patterns to ensure mix of true/false comparisons
        a[i] = i;
        b[i] = N/2 - i % 100;  // Creates crossing pattern
        c[i] = 0;
        d[i] = i % 10;
        
        fa[i] = (float)i * 1.5f;
        fb[i] = (float)(N/2 - i % 50) * 1.2f;
        fc[i] = 0.0f;
    }
}

/* Verification function to ensure computations are correct */
int verify_results(int *c1, int *c2, int *c3, int *c4, 
                   int *d1, int *d2, int *d3, int *d4) {
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += c1[i] + c2[i] + c3[i] + c4[i];
        checksum += d1[i] + d2[i] + d3[i] + d4[i];
    }
    return checksum;
}

int main() {
    // Use aligned allocation for better vectorization
    int *a = (int*)aligned_alloc(32, N * sizeof(int));
    int *b = (int*)aligned_alloc(32, N * sizeof(int));
    
    // Output arrays for each test
    int *c_gt = (int*)aligned_alloc(32, N * sizeof(int));
    int *d_gt = (int*)aligned_alloc(32, N * sizeof(int));
    
    int *c_ge = (int*)aligned_alloc(32, N * sizeof(int));
    int *d_ge = (int*)aligned_alloc(32, N * sizeof(int));
    
    int *c_lt = (int*)aligned_alloc(32, N * sizeof(int));
    int *d_lt = (int*)aligned_alloc(32, N * sizeof(int));
    
    int *c_le = (int*)aligned_alloc(32, N * sizeof(int));
    int *d_le = (int*)aligned_alloc(32, N * sizeof(int));
    
    // Float arrays
    float *fa = (float*)aligned_alloc(32, N * sizeof(float));
    float *fb = (float*)aligned_alloc(32, N * sizeof(float));
    float *fc = (float*)aligned_alloc(32, N * sizeof(float));
    
    // Initialize all arrays
    init_arrays(a, b, c_gt, d_gt, fa, fb, fc);
    memcpy(c_ge, c_gt, N * sizeof(int));
    memcpy(d_ge, d_gt, N * sizeof(int));
    memcpy(c_lt, c_gt, N * sizeof(int));
    memcpy(d_lt, d_gt, N * sizeof(int));
    memcpy(c_le, c_gt, N * sizeof(int));
    memcpy(d_le, d_gt, N * sizeof(int));
    
    // Execute all test functions
    test_gt_expr(a, b, c_gt, d_gt);
    test_ge_expr(a, b, c_ge, d_ge);
    test_lt_expr(a, b, c_lt, d_lt);
    test_le_expr(a, b, c_le, d_le);
    test_float_comparisons(fa, fb, fc);
    
    // Verify and print results to prevent dead code elimination
    int checksum = verify_results(c_gt, c_ge, c_lt, c_le, d_gt, d_ge, d_lt, d_le);
    
    // Add float checksum
    float fchecksum = 0.0f;
    for (int i = 0; i < N; i++) {
        fchecksum += fc[i];
    }
    
    printf("Integer checksum: %d\n", checksum);
    printf("Float checksum: %f\n", fchecksum);
    
    // Cleanup
    free(a); free(b);
    free(c_gt); free(d_gt);
    free(c_ge); free(d_ge);
    free(c_lt); free(d_lt);
    free(c_le); free(d_le);
    free(fa); free(fb); free(fc);
    
    return 0;
}

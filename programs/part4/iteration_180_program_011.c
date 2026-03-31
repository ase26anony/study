#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test functions for each comparison operator */

/* GT_EXPR (>) */
void test_gt_expr(int *a, int *b, int *c, int *d) {
    for (int i = 0; i < N; ++i) {
        // Conditional assignment using > operator
        c[i] = (a[i] > b[i]) ? a[i] * 2 : b[i];
        // Additional operation to prevent optimization
        d[i] = (a[i] > b[i]) ? d[i] + 1 : d[i] - 1;
    }
}

/* GE_EXPR (>=) */
void test_ge_expr(int *a, int *b, int *c, int *d) {
    for (int i = 0; i < N; ++i) {
        // Conditional assignment using >= operator
        c[i] = (a[i] >= b[i]) ? a[i] + b[i] : a[i] - b[i];
        // Masked store pattern
        if (a[i] >= b[i]) {
            d[i] = a[i] * b[i];
        }
    }
}

/* LT_EXPR (<) */
void test_lt_expr(int *a, int *b, int *c, int *d) {
    for (int i = 0; i < N; ++i) {
        // Conditional assignment using < operator
        c[i] = (a[i] < b[i]) ? a[i] * 3 : b[i] * 2;
        // Reduction-like pattern
        d[i] += (a[i] < b[i]) ? 5 : -2;
    }
}

/* LE_EXPR (<=) */
void test_le_expr(int *a, int *b, int *c, int *d) {
    for (int i = 0; i < N; ++i) {
        // Conditional assignment using <= operator
        c[i] = (a[i] <= b[i]) ? a[i] + 100 : b[i] - 50;
        // Complex conditional operation
        if (a[i] <= b[i]) {
            d[i] = (a[i] << 2) | (b[i] & 0xFF);
        }
    }
}

/* Additional test with floating point to ensure different data types */
void test_float_comparisons(float *fa, float *fb, float *fc) {
    for (int i = 0; i < N; ++i) {
        // Mix of comparison operators with floats
        if (fa[i] > fb[i]) {
            fc[i] = fa[i] * 2.0f;
        } else if (fa[i] >= fb[i]) {
            fc[i] = fa[i] + fb[i];
        } else if (fa[i] < fb[i]) {
            fc[i] = fa[i] - fb[i];
        } else if (fa[i] <= fb[i]) {
            fc[i] = fa[i] / (fb[i] + 1.0f);
        }
    }
}

/* Mixed comparison types in same loop */
void test_mixed_comparisons(int *a, int *b, int *c, int *d) {
    for (int i = 0; i < N; ++i) {
        // Use different comparison operators based on index
        if (i % 4 == 0) {
            c[i] = (a[i] > b[i]) ? 1 : 0;
        } else if (i % 4 == 1) {
            c[i] = (a[i] >= b[i]) ? 2 : 0;
        } else if (i % 4 == 2) {
            c[i] = (a[i] < b[i]) ? 3 : 0;
        } else {
            c[i] = (a[i] <= b[i]) ? 4 : 0;
        }
        d[i] = c[i] * a[i];
    }
}

/* Helper function to verify results */
int verify_results(int *c, int *c_ref) {
    for (int i = 0; i < N; ++i) {
        if (c[i] != c_ref[i]) {
            return 0;
        }
    }
    return 1;
}

int main() {
    // Aligned arrays for better vectorization
    ALIGNED int a[N], b[N], c1[N], c2[N], c3[N], c4[N];
    ALIGNED int d1[N], d2[N], d3[N], d4[N];
    ALIGNED float fa[N], fb[N], fc[N];
    
    // Initialize arrays with varying patterns
    for (int i = 0; i < N; ++i) {
        a[i] = i;
        b[i] = N/2 - i % 100;  // Creates mix of true/false comparisons
        d1[i] = d2[i] = d3[i] = d4[i] = i % 10;
        
        fa[i] = (float)i * 1.5f;
        fb[i] = (float)(N/2 - i % 50) * 1.2f;
    }
    
    printf("Testing vectorizable loops with comparison operators...\n");
    
    // Test each comparison operator separately
    test_gt_expr(a, b, c1, d1);
    test_ge_expr(a, b, c2, d2);
    test_lt_expr(a, b, c3, d3);
    test_le_expr(a, b, c4, d4);
    
    // Test floating point comparisons
    test_float_comparisons(fa, fb, fc);
    
    // Test mixed comparisons
    ALIGNED int cmixed[N], dmixed[N];
    test_mixed_comparisons(a, b, cmixed, dmixed);
    
    // Compute checksums to ensure computations aren't optimized away
    int checksum = 0;
    float fchecksum = 0.0f;
    
    for (int i = 0; i < N; ++i) {
        checksum += c1[i] + c2[i] + c3[i] + c4[i];
        checksum += d1[i] + d2[i] + d3[i] + d4[i];
        checksum += cmixed[i] + dmixed[i];
        fchecksum += fc[i];
    }
    
    printf("Integer checksum: %d\n", checksum);
    printf("Float checksum: %f\n", fchecksum);
    
    // Verify against sequential computation
    ALIGNED int c1_ref[N], c2_ref[N], c3_ref[N], c4_ref[N];
    ALIGNED int d1_ref[N] = {0}, d2_ref[N] = {0}, d3_ref[N] = {0}, d4_ref[N] = {0};
    
    // Initialize reference arrays
    for (int i = 0; i < N; ++i) {
        d1_ref[i] = d2_ref[i] = d3_ref[i] = d4_ref[i] = i % 10;
    }
    
    // Sequential computation for verification
    for (int i = 0; i < N; ++i) {
        c1_ref[i] = (a[i] > b[i]) ? a[i] * 2 : b[i];
        d1_ref[i] = (a[i] > b[i]) ? d1_ref[i] + 1 : d1_ref[i] - 1;
        
        c2_ref[i] = (a[i] >= b[i]) ? a[i] + b[i] : a[i] - b[i];
        if (a[i] >= b[i]) {
            d2_ref[i] = a[i] * b[i];
        }
        
        c3_ref[i] = (a[i] < b[i]) ? a[i] * 3 : b[i] * 2;
        d3_ref[i] += (a[i] < b[i]) ? 5 : -2;
        
        c4_ref[i] = (a[i] <= b[i]) ? a[i] + 100 : b[i] - 50;
        if (a[i] <= b[i]) {
            d4_ref[i] = (a[i] << 2) | (b[i] & 0xFF);
        }
    }
    
    // Verify each test
    int all_ok = 1;
    all_ok &= verify_results(c1, c1_ref);
    all_ok &= verify_results(c2, c2_ref);
    all_ok &= verify_results(c3, c3_ref);
    all_ok &= verify_results(c4, c4_ref);
    
    if (all_ok) {
        printf("All tests passed!\n");
    } else {
        printf("Some tests failed!\n");
        return 1;
    }
    
    return 0;
}

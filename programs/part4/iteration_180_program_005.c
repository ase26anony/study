/* test_vector_cond_bitops.c
 * 
 * This program is designed to trigger the transformation of comparison
 * operations (GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR) to bit operations
 * (BIT_NOT_EXPR, BIT_AND_EXPR, BIT_IOR_EXPR) during auto-vectorization
 * in GCC's tree-vect-stmts.cc.
 *
 * Compile with: gcc -O3 -ftree-vectorize -fno-vect-cost-model -march=native
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test function for GT_EXPR (>) */
void test_gt_expr(int *a, int *b, int *c, int *d) {
    for (int i = 0; i < N; ++i) {
        // Conditional assignment using > operator
        // This should trigger GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR transformation
        c[i] = (a[i] > b[i]) ? a[i] * 2 : b[i] / 2;
        
        // Additional masked operation to increase vectorization likelihood
        if (a[i] > b[i]) {
            d[i] = a[i] - b[i];
        } else {
            d[i] = b[i] - a[i];
        }
    }
}

/* Test function for GE_EXPR (>=) */
void test_ge_expr(int *a, int *b, int *c, int *d) {
    for (int i = 0; i < N; ++i) {
        // Conditional assignment using >= operator
        // This should trigger GE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR transformation
        c[i] = (a[i] >= b[i]) ? a[i] + b[i] : a[i] - b[i];
        
        // Reduction pattern with >=
        if (a[i] >= b[i]) {
            d[i] = d[i] * 2 + 1;
        } else {
            d[i] = d[i] / 2 - 1;
        }
    }
}

/* Test function for LT_EXPR (<) */
void test_lt_expr(int *a, int *b, int *c, int *d) {
    for (int i = 0; i < N; ++i) {
        // Conditional assignment using < operator
        // This should trigger LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR transformation
        // with operand swapping
        c[i] = (a[i] < b[i]) ? a[i] * 3 : b[i] * 2;
        
        // Masked store with <
        if (a[i] < b[i]) {
            d[i] = a[i] * b[i];
        }
    }
}

/* Test function for LE_EXPR (<=) */
void test_le_expr(int *a, int *b, int *c, int *d) {
    for (int i = 0; i < N; ++i) {
        // Conditional assignment using <= operator
        // This should trigger LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR transformation
        // with operand swapping
        c[i] = (a[i] <= b[i]) ? a[i] | b[i] : a[i] & b[i];
        
        // Complex conditional with <=
        if (a[i] <= b[i]) {
            d[i] = (d[i] << 1) | 1;
        } else {
            d[i] = (d[i] >> 1) & 0x7FFFFFFF;
        }
    }
}

/* Test function with floating point comparisons (also vectorizable) */
void test_float_comparisons(float *fa, float *fb, float *fc) {
    for (int i = 0; i < N; ++i) {
        // Mix of different comparison operators with floats
        if (fa[i] > fb[i]) {
            fc[i] = fa[i] * 2.0f;
        } else if (fa[i] >= fb[i]) {
            fc[i] = fa[i] + fb[i];
        } else if (fa[i] < fb[i]) {
            fc[i] = fb[i] - fa[i];
        } else if (fa[i] <= fb[i]) {
            fc[i] = fa[i] / (fb[i] + 1.0f);
        }
    }
}

/* Helper function to initialize arrays with pattern that creates
 * mix of true/false comparison results */
void init_arrays(int *a, int *b, int *c, int *d, 
                 float *fa, float *fb, float *fc) {
    for (int i = 0; i < N; ++i) {
        // Create varying patterns for integer arrays
        a[i] = i;
        b[i] = N/2 - i % 100;  // Creates mix of >, <, == cases
        c[i] = 0;
        d[i] = i * 2;
        
        // Create varying patterns for float arrays
        fa[i] = (float)i * 1.5f;
        fb[i] = (float)(N/2 - i % 50) * 2.0f;
        fc[i] = 0.0f;
    }
}

/* Verification function to ensure computations are correct
 * and prevent dead code elimination */
int verify_results(int *c1, int *c2, int *c3, int *c4, 
                   int *d1, int *d2, int *d3, int *d4,
                   float *fc) {
    int checksum = 0;
    
    for (int i = 0; i < N; ++i) {
        checksum += c1[i] + c2[i] + c3[i] + c4[i];
        checksum += d1[i] + d2[i] + d3[i] + d4[i];
        checksum += (int)fc[i];
    }
    
    return checksum;
}

int main() {
    // Aligned allocations for better vectorization
    int *a = aligned_alloc(32, N * sizeof(int));
    int *b = aligned_alloc(32, N * sizeof(int));
    int *c1 = aligned_alloc(32, N * sizeof(int));
    int *c2 = aligned_alloc(32, N * sizeof(int));
    int *c3 = aligned_alloc(32, N * sizeof(int));
    int *c4 = aligned_alloc(32, N * sizeof(int));
    int *d1 = aligned_alloc(32, N * sizeof(int));
    int *d2 = aligned_alloc(32, N * sizeof(int));
    int *d3 = aligned_alloc(32, N * sizeof(int));
    int *d4 = aligned_alloc(32, N * sizeof(int));
    
    float *fa = aligned_alloc(32, N * sizeof(float));
    float *fb = aligned_alloc(32, N * sizeof(float));
    float *fc = aligned_alloc(32, N * sizeof(float));
    
    if (!a || !b || !c1 || !c2 || !c3 || !c4 || 
        !d1 || !d2 || !d3 || !d4 || !fa || !fb || !fc) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize arrays with patterns that will exercise all comparison paths
    init_arrays(a, b, c1, d1, fa, fb, fc);
    
    // Copy initial values for each test
    memcpy(c2, c1, N * sizeof(int));
    memcpy(c3, c1, N * sizeof(int));
    memcpy(c4, c1, N * sizeof(int));
    memcpy(d2, d1, N * sizeof(int));
    memcpy(d3, d1, N * sizeof(int));
    memcpy(d4, d1, N * sizeof(int));
    
    // Execute all test functions to trigger different comparison transformations
    test_gt_expr(a, b, c1, d1);   // GT_EXPR: > operator
    test_ge_expr(a, b, c2, d2);   // GE_EXPR: >= operator  
    test_lt_expr(a, b, c3, d3);   // LT_EXPR: < operator
    test_le_expr(a, b, c4, d4);   // LE_EXPR: <= operator
    test_float_comparisons(fa, fb, fc);  // Float comparisons
    
    // Verify results to prevent optimization
    int checksum = verify_results(c1, c2, c3, c4, d1, d2, d3, d4, fc);
    
    printf("Test completed. Checksum: %d\n", checksum);
    printf("(If checksum varies, it's due to different initialization patterns)\n");
    
    // Cleanup
    free(a); free(b);
    free(c1); free(c2); free(c3); free(c4);
    free(d1); free(d2); free(d3); free(d4);
    free(fa); free(fb); free(fc);
    
    return 0;
}

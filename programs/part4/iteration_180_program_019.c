/* Test program to trigger vectorization of conditional expressions
   and hit the bit-operation transformation for comparison operators
   in tree-vect-stmts.cc lines 12216-12233 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGN 32

/* Aligned allocations for better vectorization */
#ifdef __GNUC__
#define ALIGNED __attribute__((aligned(ALIGN)))
#else
#define ALIGNED
#endif

/* Test function for GT_EXPR (>) */
void test_gt_expr(int *restrict a, int *restrict b, int *restrict c, int n) {
    for (int i = 0; i < n; i++) {
        /* Conditional assignment using > operator */
        c[i] = (a[i] > b[i]) ? a[i] * 2 : b[i] / 2;
    }
}

/* Test function for GE_EXPR (>=) */
void test_ge_expr(int *restrict a, int *restrict b, int *restrict c, int n) {
    for (int i = 0; i < n; i++) {
        /* Conditional assignment using >= operator */
        c[i] = (a[i] >= b[i]) ? a[i] + b[i] : a[i] - b[i];
    }
}

/* Test function for LT_EXPR (<) */
void test_lt_expr(int *restrict a, int *restrict b, int *restrict c, int n) {
    for (int i = 0; i < n; i++) {
        /* Conditional assignment using < operator */
        c[i] = (a[i] < b[i]) ? a[i] * b[i] : a[i] + 100;
    }
}

/* Test function for LE_EXPR (<=) */
void test_le_expr(int *restrict a, int *restrict b, int *restrict c, int n) {
    for (int i = 0; i < n; i++) {
        /* Conditional assignment using <= operator */
        c[i] = (a[i] <= b[i]) ? a[i] | b[i] : a[i] & b[i];
    }
}

/* Additional test with floating point to ensure different type handling */
void test_float_comparisons(float *restrict fa, float *restrict fb, 
                           float *restrict fc, int n) {
    for (int i = 0; i < n; i++) {
        /* Mix of different comparison operators with floats */
        if (fa[i] > fb[i]) {
            fc[i] = fa[i] * 2.0f;
        } else if (fa[i] >= fb[i] * 0.5f) {
            fc[i] = fa[i] + fb[i];
        } else if (fa[i] < fb[i]) {
            fc[i] = fa[i] - fb[i];
        } else if (fa[i] <= fb[i] * 2.0f) {
            fc[i] = fa[i] / fb[i];
        } else {
            fc[i] = fa[i];
        }
    }
}

/* Test with reduction pattern using comparisons */
int test_reduction_with_comparisons(int *restrict a, int *restrict b, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        /* Conditional increment using different comparison operators */
        sum += (a[i] > b[i]) ? a[i] : 0;
        sum += (a[i] >= b[i] + 10) ? b[i] : 0;
        sum += (a[i] < b[i] - 5) ? a[i] * 2 : 0;
        sum += (a[i] <= b[i] + 20) ? b[i] * 3 : 0;
    }
    return sum;
}

/* Initialize arrays with pattern that creates mix of true/false comparisons */
void init_arrays(int *a, int *b, float *fa, float *fb) {
    for (int i = 0; i < N; i++) {
        /* Create varying patterns to ensure all comparison paths are taken */
        a[i] = i;
        b[i] = N/2 - i % 100;  /* Creates mix of >, <, == cases */
        fa[i] = (float)(i % 50) * 1.5f;
        fb[i] = (float)(i % 30) * 2.0f;
    }
}

/* Verify results by comparing with sequential computation */
int verify_results(int *c1, int *c2, int *c3, int *c4, float *fc, int n) {
    int errors = 0;
    
    /* Recompute expected values sequentially */
    for (int i = 0; i < n; i++) {
        int a = i;
        int b = N/2 - i % 100;
        float fa = (float)(i % 50) * 1.5f;
        float fb = (float)(i % 30) * 2.0f;
        
        /* Check GT */
        int expected_gt = (a > b) ? a * 2 : b / 2;
        if (c1[i] != expected_gt) errors++;
        
        /* Check GE */
        int expected_ge = (a >= b) ? a + b : a - b;
        if (c2[i] != expected_ge) errors++;
        
        /* Check LT */
        int expected_lt = (a < b) ? a * b : a + 100;
        if (c3[i] != expected_lt) errors++;
        
        /* Check LE */
        int expected_le = (a <= b) ? a | b : a & b;
        if (c4[i] != expected_le) errors++;
        
        /* Check float comparisons */
        float expected_fc;
        if (fa > fb) {
            expected_fc = fa * 2.0f;
        } else if (fa >= fb * 0.5f) {
            expected_fc = fa + fb;
        } else if (fa < fb) {
            expected_fc = fa - fb;
        } else if (fa <= fb * 2.0f) {
            expected_fc = fa / fb;
        } else {
            expected_fc = fa;
        }
        
        if (fc[i] != expected_fc) errors++;
    }
    
    return errors;
}

int main() {
    /* Declare aligned arrays */
    int ALIGNED a[N], b[N];
    int ALIGNED c_gt[N], c_ge[N], c_lt[N], c_le[N];
    float ALIGNED fa[N], fb[N], fc[N];
    
    /* Initialize test data */
    init_arrays(a, b, fa, fb);
    
    /* Clear output arrays */
    memset(c_gt, 0, sizeof(c_gt));
    memset(c_ge, 0, sizeof(c_ge));
    memset(c_lt, 0, sizeof(c_lt));
    memset(c_le, 0, sizeof(c_le));
    memset(fc, 0, sizeof(fc));
    
    /* Execute all test functions */
    test_gt_expr(a, b, c_gt, N);
    test_ge_expr(a, b, c_ge, N);
    test_lt_expr(a, b, c_lt, N);
    test_le_expr(a, b, c_le, N);
    test_float_comparisons(fa, fb, fc, N);
    
    /* Test reduction pattern */
    int reduction_sum = test_reduction_with_comparisons(a, b, N);
    
    /* Verify results */
    int errors = verify_results(c_gt, c_ge, c_lt, c_le, fc, N);
    
    /* Print results to prevent dead code elimination */
    printf("Test results:\n");
    printf("  GT_EXPR test: c_gt[0]=%d, c_gt[%d]=%d\n", c_gt[0], N-1, c_gt[N-1]);
    printf("  GE_EXPR test: c_ge[0]=%d, c_ge[%d]=%d\n", c_ge[0], N-1, c_ge[N-1]);
    printf("  LT_EXPR test: c_lt[0]=%d, c_lt[%d]=%d\n", c_lt[0], N-1, c_lt[N-1]);
    printf("  LE_EXPR test: c_le[0]=%d, c_le[%d]=%d\n", c_le[0], N-1, c_le[N-1]);
    printf("  Float comparisons: fc[0]=%.2f, fc[%d]=%.2f\n", fc[0], N-1, fc[N-1]);
    printf("  Reduction sum: %d\n", reduction_sum);
    printf("  Verification errors: %d\n", errors);
    
    /* Use results to affect return value */
    return errors > 0 ? 1 : 0;
}

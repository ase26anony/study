/* test_vectorized_comparisons.c
 * 
 * This program contains vectorizable loops with conditional statements
 * using all four comparison operators (>, >=, <, <=) to trigger the
 * transformation of comparisons to bit operations in GCC's tree vectorizer.
 * 
 * Compile with: gcc -O3 -ftree-vectorize -fno-vect-cost-model -march=native -fno-tree-slp-vectorize test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test function for GT_EXPR (>) */
void test_gt_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict d) {
    for (int i = 0; i < N; i++) {
        /* Conditional assignment using > operator */
        if (a[i] > b[i]) {
            c[i] = a[i] * 2 + b[i];
        } else {
            c[i] = a[i] + b[i] / 2;
        }
        
        /* Additional masked operation to encourage bitmask usage */
        d[i] = (a[i] > b[i]) ? (a[i] - b[i]) : (b[i] - a[i]);
    }
}

/* Test function for GE_EXPR (>=) */
void test_ge_expr(float* restrict a, float* restrict b, float* restrict c, float* restrict d) {
    for (int i = 0; i < N; i++) {
        /* Conditional increment pattern */
        if (a[i] >= b[i]) {
            c[i] = a[i] * 3.0f;
        } else {
            c[i] = b[i] * 2.0f;
        }
        
        /* Reduction-like pattern with >= */
        d[i] = (a[i] >= b[i]) ? (a[i] + b[i]) : (a[i] - b[i]);
    }
}

/* Test function for LT_EXPR (<) */
void test_lt_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict d) {
    for (int i = 0; i < N; i++) {
        /* Masked store with < operator */
        if (a[i] < b[i]) {
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] + b[i];
        }
        
        /* Conditional blend using ternary operator with < */
        d[i] = (a[i] < b[i]) ? (a[i] | b[i]) : (a[i] & b[i]);
    }
}

/* Test function for LE_EXPR (<=) */
void test_le_expr(float* restrict a, float* restrict b, float* restrict c, float* restrict d) {
    for (int i = 0; i < N; i++) {
        /* Complex conditional with <= */
        if (a[i] <= b[i]) {
            c[i] = a[i] * a[i] - b[i];
        } else {
            c[i] = b[i] * b[i] - a[i];
        }
        
        /* Pattern that might trigger operand swapping */
        d[i] = (a[i] <= b[i]) ? (a[i] * 2.0f) : (b[i] * 3.0f);
    }
}

/* Additional test with mixed comparisons in same loop */
void test_mixed_comparisons(int* restrict a, int* restrict b, int* restrict c) {
    for (int i = 0; i < N; i++) {
        /* Use all four operators in different expressions */
        int temp = 0;
        if (a[i] > b[i]) temp += 1;
        if (a[i] >= b[i]) temp += 2;
        if (a[i] < b[i]) temp += 4;
        if (a[i] <= b[i]) temp += 8;
        c[i] = temp;
    }
}

/* Reduction pattern that uses comparisons */
int test_reduction_with_comparisons(int* restrict a, int* restrict b) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        /* Reduction with > comparison */
        sum += (a[i] > b[i]) ? a[i] : b[i];
        
        /* Reduction with < comparison */
        sum -= (a[i] < b[i]) ? a[i] : b[i];
    }
    return sum;
}

/* Initialize arrays with pattern to ensure mix of true/false comparisons */
void init_arrays(int* a, int* b, float* fa, float* fb) {
    for (int i = 0; i < N; i++) {
        /* Create data pattern that yields mix of comparison results */
        a[i] = i - N/2;  /* Values from -512 to 511 */
        b[i] = i % 100;  /* Values from 0 to 99 */
        
        /* For floating point, similar pattern */
        fa[i] = (float)(i - N/2) * 0.5f;
        fb[i] = (float)(i % 100) * 0.3f;
    }
}

/* Verify results by comparing with sequential computation */
int verify_results(int* a, int* b, float* fa, float* fb, 
                   int* gt_c, int* gt_d, float* ge_c, float* ge_d,
                   int* lt_c, int* lt_d, float* le_c, float* le_d,
                   int* mixed_c) {
    int errors = 0;
    
    /* Verify GT_EXPR results */
    for (int i = 0; i < N; i++) {
        int expected_c = (a[i] > b[i]) ? (a[i] * 2 + b[i]) : (a[i] + b[i] / 2);
        int expected_d = (a[i] > b[i]) ? (a[i] - b[i]) : (b[i] - a[i]);
        
        if (gt_c[i] != expected_c) errors++;
        if (gt_d[i] != expected_d) errors++;
    }
    
    /* Verify GE_EXPR results */
    for (int i = 0; i < N; i++) {
        float expected_c = (fa[i] >= fb[i]) ? (fa[i] * 3.0f) : (fb[i] * 2.0f);
        float expected_d = (fa[i] >= fb[i]) ? (fa[i] + fb[i]) : (fa[i] - fb[i]);
        
        if (ge_c[i] != expected_c) errors++;
        if (ge_d[i] != expected_d) errors++;
    }
    
    /* Verify LT_EXPR results */
    for (int i = 0; i < N; i++) {
        int expected_c = (a[i] < b[i]) ? (a[i] * b[i]) : (a[i] + b[i]);
        int expected_d = (a[i] < b[i]) ? (a[i] | b[i]) : (a[i] & b[i]);
        
        if (lt_c[i] != expected_c) errors++;
        if (lt_d[i] != expected_d) errors++;
    }
    
    /* Verify LE_EXPR results */
    for (int i = 0; i < N; i++) {
        float expected_c = (fa[i] <= fb[i]) ? (fa[i] * fa[i] - fb[i]) : (fb[i] * fb[i] - fa[i]);
        float expected_d = (fa[i] <= fb[i]) ? (fa[i] * 2.0f) : (fb[i] * 3.0f);
        
        if (le_c[i] != expected_c) errors++;
        if (le_d[i] != expected_d) errors++;
    }
    
    /* Verify mixed comparisons */
    for (int i = 0; i < N; i++) {
        int expected = 0;
        if (a[i] > b[i]) expected += 1;
        if (a[i] >= b[i]) expected += 2;
        if (a[i] < b[i]) expected += 4;
        if (a[i] <= b[i]) expected += 8;
        
        if (mixed_c[i] != expected) errors++;
    }
    
    return errors;
}

int main() {
    /* Aligned allocations for better vectorization */
    ALIGNED int a[N], b[N];
    ALIGNED float fa[N], fb[N];
    
    /* Output arrays for each test */
    ALIGNED int gt_c[N], gt_d[N];
    ALIGNED float ge_c[N], ge_d[N];
    ALIGNED int lt_c[N], lt_d[N];
    ALIGNED float le_c[N], le_d[N];
    ALIGNED int mixed_c[N];
    
    /* Initialize input data */
    init_arrays(a, b, fa, fb);
    
    /* Run all comparison tests */
    test_gt_expr(a, b, gt_c, gt_d);
    test_ge_expr(fa, fb, ge_c, ge_d);
    test_lt_expr(a, b, lt_c, lt_d);
    test_le_expr(fa, fb, le_c, le_d);
    test_mixed_comparisons(a, b, mixed_c);
    
    /* Test reduction pattern */
    int reduction_result = test_reduction_with_comparisons(a, b);
    
    /* Verify all results */
    int errors = verify_results(a, b, fa, fb, gt_c, gt_d, ge_c, ge_d, 
                               lt_c, lt_d, le_c, le_d, mixed_c);
    
    if (errors == 0) {
        printf("All tests passed! Reduction result: %d\n", reduction_result);
        return 0;
    } else {
        printf("Found %d errors\n", errors);
        return 1;
    }
}

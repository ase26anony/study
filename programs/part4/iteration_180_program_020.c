/* test_vectorized_comparisons.c
 * 
 * This program creates vectorizable loops with conditional statements
 * using all four comparison operators (>, >=, <, <=) to trigger
 * the transformation of comparisons to bit operations in GCC's
 * tree vectorizer (tree-vect-stmts.cc lines 12216-12233).
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Function prototypes for each comparison type */
void test_gt_expr(int *a, int *b, int *c, int n);
void test_ge_expr(int *a, int *b, int *c, int n);
void test_lt_expr(int *a, int *b, int *c, int n);
void test_le_expr(int *a, int *b, int *c, int n);
void test_mixed_conditional_assign(int *a, int *b, int *c, int *d, int n);
void test_conditional_reduction(int *a, int *b, int n);

/* Helper function to verify results */
int verify_results(int *ref, int *test, int n, const char *test_name);

int main(void) {
    int i;
    
    /* Allocate and align arrays for better vectorization */
    int *a ALIGNED = (int*)aligned_alloc(32, N * sizeof(int));
    int *b ALIGNED = (int*)aligned_alloc(32, N * sizeof(int));
    int *c ALIGNED = (int*)aligned_alloc(32, N * sizeof(int));
    int *d ALIGNED = (int*)aligned_alloc(32, N * sizeof(int));
    int *ref ALIGNED = (int*)aligned_alloc(32, N * sizeof(int));
    
    if (!a || !b || !c || !d || !ref) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with varying data to create mix of true/false comparisons */
    for (i = 0; i < N; i++) {
        a[i] = i;                     /* 0, 1, 2, ..., N-1 */
        b[i] = N/2 - i%100;           /* Creates alternating pattern around N/2 */
        c[i] = 0;                     /* Output array */
        d[i] = i * 3;                 /* Another source array */
    }
    
    printf("Testing vectorized comparisons to trigger bit-op transformations...\n");
    
    /* Test 1: GT_EXPR (>) - Conditional assignment pattern */
    memset(c, 0, N * sizeof(int));
    test_gt_expr(a, b, c, N);
    
    /* Compute reference for GT test */
    for (i = 0; i < N; i++) {
        ref[i] = (a[i] > b[i]) ? a[i] * 2 : b[i];
    }
    if (!verify_results(ref, c, N, "GT_EXPR")) {
        printf("GT_EXPR test passed\n");
    }
    
    /* Test 2: GE_EXPR (>=) - Conditional increment pattern */
    memset(c, 0, N * sizeof(int));
    test_ge_expr(a, b, c, N);
    
    /* Compute reference for GE test */
    for (i = 0; i < N; i++) {
        ref[i] = (a[i] >= b[i]) ? a[i] + b[i] : a[i] - b[i];
    }
    if (!verify_results(ref, c, N, "GE_EXPR")) {
        printf("GE_EXPR test passed\n");
    }
    
    /* Test 3: LT_EXPR (<) - Masked store pattern */
    memset(c, 0, N * sizeof(int));
    test_lt_expr(a, b, c, N);
    
    /* Compute reference for LT test */
    for (i = 0; i < N; i++) {
        ref[i] = (a[i] < b[i]) ? a[i] * b[i] : 0;
    }
    if (!verify_results(ref, c, N, "LT_EXPR")) {
        printf("LT_EXPR test passed\n");
    }
    
    /* Test 4: LE_EXPR (<=) - Conditional blend pattern */
    memset(c, 0, N * sizeof(int));
    test_le_expr(a, b, c, N);
    
    /* Compute reference for LE test */
    for (i = 0; i < N; i++) {
        ref[i] = (a[i] <= b[i]) ? a[i] | b[i] : a[i] & b[i];
    }
    if (!verify_results(ref, c, N, "LE_EXPR")) {
        printf("LE_EXPR test passed\n");
    }
    
    /* Test 5: Mixed conditional assignment (uses all operators) */
    memset(c, 0, N * sizeof(int));
    test_mixed_conditional_assign(a, b, c, d, N);
    
    /* Compute reference for mixed test */
    for (i = 0; i < N; i++) {
        int temp = 0;
        if (a[i] > b[i]) temp += d[i];
        if (a[i] >= b[i] + 10) temp += a[i];
        if (a[i] < b[i] - 10) temp -= b[i];
        if (a[i] <= b[i] + 5) temp |= 0xFF;
        ref[i] = temp;
    }
    if (!verify_results(ref, c, N, "Mixed conditionals")) {
        printf("Mixed conditionals test passed\n");
    }
    
    /* Test 6: Conditional reduction (sum with condition) */
    int sum = test_conditional_reduction(a, b, N);
    
    /* Compute reference reduction */
    int ref_sum = 0;
    for (i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            ref_sum += a[i];
        }
        if (a[i] <= b[i]) {
            ref_sum -= b[i];
        }
    }
    
    if (sum == ref_sum) {
        printf("Conditional reduction test passed (sum = %d)\n", sum);
    } else {
        printf("Conditional reduction test FAILED: got %d, expected %d\n", sum, ref_sum);
    }
    
    /* Free allocated memory */
    free(a);
    free(b);
    free(c);
    free(d);
    free(ref);
    
    printf("\nAll tests completed. If compiled with -O3 -ftree-vectorize,\n");
    printf("the loops should trigger the comparison-to-bit-op transformation\n");
    printf("in tree-vect-stmts.cc (lines 12216-12233).\n");
    
    return 0;
}

/* Test function for GT_EXPR (>) */
void test_gt_expr(int *a, int *b, int *c, int n) {
    int i;
    /* Pattern: Conditional assignment that can be vectorized */
    for (i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            c[i] = a[i] * 2;  /* True path */
        } else {
            c[i] = b[i];      /* False path */
        }
    }
}

/* Test function for GE_EXPR (>=) */
void test_ge_expr(int *a, int *b, int *c, int n) {
    int i;
    /* Pattern: Conditional operation with arithmetic */
    for (i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

/* Test function for LT_EXPR (<) */
void test_lt_expr(int *a, int *b, int *c, int n) {
    int i;
    /* Pattern: Masked store - only write when condition is true */
    for (i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            c[i] = a[i] * b[i];
        }
        /* else leave as 0 (already initialized) */
    }
}

/* Test function for LE_EXPR (<=) */
void test_le_expr(int *a, int *b, int *c, int n) {
    int i;
    /* Pattern: Conditional bitwise operations */
    for (i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            c[i] = a[i] | b[i];
        } else {
            c[i] = a[i] & b[i];
        }
    }
}

/* Test function with mixed conditional assignments */
void test_mixed_conditional_assign(int *a, int *b, int *c, int *d, int n) {
    int i;
    /* Complex pattern with multiple conditionals in same loop */
    for (i = 0; i < n; i++) {
        int temp = 0;
        
        /* GT comparison */
        if (a[i] > b[i]) {
            temp += d[i];
        }
        
        /* GE comparison with offset */
        if (a[i] >= b[i] + 10) {
            temp += a[i];
        }
        
        /* LT comparison with offset */
        if (a[i] < b[i] - 10) {
            temp -= b[i];
        }
        
        /* LE comparison with offset */
        if (a[i] <= b[i] + 5) {
            temp |= 0xFF;
        }
        
        c[i] = temp;
    }
}

/* Test function for conditional reduction */
int test_conditional_reduction(int *a, int *b, int n) {
    int i;
    int sum = 0;
    
    /* Reduction with conditionals - may be vectorized as masked reduction */
    for (i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
        if (a[i] <= b[i]) {
            sum -= b[i];
        }
    }
    
    return sum;
}

/* Helper function to verify results */
int verify_results(int *ref, int *test, int n, const char *test_name) {
    int i;
    for (i = 0; i < n; i++) {
        if (ref[i] != test[i]) {
            printf("Test %s FAILED at index %d: ref[%d]=%d, test[%d]=%d\n",
                   test_name, i, i, ref[i], i, test[i]);
            return 1;
        }
    }
    return 0;
}

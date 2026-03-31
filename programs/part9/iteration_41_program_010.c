/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Function prototypes */
int test_ge_vectorize(void);
int test_gt_vectorize(void);
int test_lt_vectorize(void);
int test_le_vectorize(void);

/* Global aligned arrays to ensure vectorization */
ALIGNED int a[N];
ALIGNED int b[N];
ALIGNED int c[N];
ALIGNED int mask[N];

int main(void) {
    /* Initialize arrays with pattern that triggers comparisons */
    for (int i = 0; i < N; i++) {
        a[i] = i;               /* 0, 1, 2, ..., N-1 */
        b[i] = N/2;             /* All N/2 */
    }
    
    printf("Testing vector comparison expansions in tree-vect-stmts.cc\n");
    printf("Array size: %d, Threshold: %d\n\n", N, N/2);
    
    /* Test GE_EXPR case (lines 12219-12222) */
    int ge_result = test_ge_vectorize();
    printf("GE_EXPR test result: %d\n", ge_result);
    
    /* Test GT_EXPR case (lines 12216-12218) */
    int gt_result = test_gt_vectorize();
    printf("GT_EXPR test result: %d\n", gt_result);
    
    /* Test LT_EXPR case (lines 12223-12227) */
    int lt_result = test_lt_vectorize();
    printf("LT_EXPR test result: %d\n", lt_result);
    
    /* Test LE_EXPR case (lines 12228-12233) */
    int le_result = test_le_vectorize();
    printf("LE_EXPR test result: %d\n", le_result);
    
    /* Verify results match expected pattern */
    int expected_ge = 0;
    for (int i = 0; i < N; i++) {
        if (i >= N/2) expected_ge += a[i];
    }
    
    printf("\nExpected GE result: %d\n", expected_ge);
    printf("All tests completed.\n");
    
    return 0;
}

/* Test GE_EXPR (>=) - targets lines 12219-12222 */
int test_ge_vectorize(void) {
    int sum = 0;
    
    /* Pattern 1: Conditional sum with >= comparison */
    /* This creates a mask-like operation that should trigger the expansion */
    for (int i = 0; i < N; i++) {
        /* The comparison result is used in arithmetic context */
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    
    /* Pattern 2: Mask creation with >= (alternative path) */
    /* Create mask where true = -1 (all bits set), false = 0 */
    for (int i = 0; i < N; i++) {
        mask[i] = (a[i] >= b[i]) ? -1 : 0;
    }
    
    /* Use mask in computation to prevent optimization */
    for (int i = 0; i < N; i++) {
        c[i] = a[i] & mask[i];
    }
    
    volatile int prevent_opt = c[N-1]; /* Prevent dead code elimination */
    (void)prevent_opt;
    
    return sum;
}

/* Test GT_EXPR (>) - targets lines 12216-12218 */
int test_gt_vectorize(void) {
    int sum = 0;
    
    /* Conditional reduction with > comparison */
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    
    /* Mask-based operation */
    for (int i = 0; i < N; i++) {
        mask[i] = (a[i] > b[i]) ? -1 : 0;
        c[i] = b[i] & mask[i];
    }
    
    volatile int prevent_opt = c[N/4];
    (void)prevent_opt;
    
    return sum;
}

/* Test LT_EXPR (<) - targets lines 12223-12227 */
int test_lt_vectorize(void) {
    int sum = 0;
    
    /* Conditional reduction with < comparison */
    /* Note: This should trigger std::swap(cond_expr0, cond_expr1) */
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            sum += a[i];
        }
    }
    
    /* Mask creation with < */
    for (int i = 0; i < N; i++) {
        mask[i] = (a[i] < b[i]) ? -1 : 0;
        c[i] = a[i] | mask[i];
    }
    
    volatile int prevent_opt = c[N/3];
    (void)prevent_opt;
    
    return sum;
}

/* Test LE_EXPR (<=) - targets lines 12228-12233 */
int test_le_vectorize(void) {
    int sum = 0;
    
    /* Conditional reduction with <= comparison */
    /* Note: This should trigger std::swap(cond_expr0, cond_expr1) */
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            sum += a[i];
        }
    }
    
    /* Alternative: Conditional select */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? a[i] : b[i];
    }
    
    volatile int prevent_opt = c[0];
    (void)prevent_opt;
    
    return sum;
}

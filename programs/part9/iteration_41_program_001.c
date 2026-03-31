/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc
 * Specifically targets the switch cases for GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR
 * Lines 12216-12233 in tree-vect-stmts.cc
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Function prototypes for different comparison operators */
int test_ge_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c);
int test_gt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c);
int test_lt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c);
int test_le_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c);

int main(void) {
    /* Declare aligned arrays */
    ALIGNED int a[N];
    ALIGNED int b[N];
    ALIGNED int c[N];
    
    int sum_ge, sum_gt, sum_lt, sum_le;
    
    /* Initialize arrays with pattern that triggers various comparison results */
    for (int i = 0; i < N; i++) {
        a[i] = i;                     /* 0, 1, 2, ..., N-1 */
        b[i] = N/2;                   /* All elements = N/2 */
    }
    
    printf("Testing vector comparison expansions...\n");
    
    /* Test GE_EXPR (>=) - primary target */
    sum_ge = test_ge_vectorize(a, b, c);
    printf("GE_EXPR result: %d\n", sum_ge);
    
    /* Test GT_EXPR (>) */
    sum_gt = test_gt_vectorize(a, b, c);
    printf("GT_EXPR result: %d\n", sum_gt);
    
    /* Test LT_EXPR (<) */
    sum_lt = test_lt_vectorize(a, b, c);
    printf("LT_EXPR result: %d\n", sum_lt);
    
    /* Test LE_EXPR (<=) */
    sum_le = test_le_vectorize(a, b, c);
    printf("LE_EXPR result: %d\n", sum_le);
    
    /* Final checksum to ensure all computations are used */
    int total = sum_ge + sum_gt + sum_lt + sum_le;
    printf("Total checksum: %d\n", total);
    
    return 0;
}

/* GE_EXPR (>=) test - primary target for uncovered lines */
int test_ge_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    /* Pattern 1: Conditional sum using >= comparison
     * This creates a mask-like operation that should trigger
     * the BIT_NOT_EXPR + BIT_IOR_EXPR expansion for GE_EXPR
     */
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    
    /* Pattern 2: Mask creation using ternary operator
     * Creates -1/0 mask which is likely to use bitwise expansion
     */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? -1 : 0;
    }
    
    /* Use the mask in a computation to prevent optimization */
    for (int i = 0; i < N; i++) {
        sum += c[i];
    }
    
    return sum;
}

/* GT_EXPR (>) test */
int test_gt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    /* Similar pattern for > comparison */
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    
    /* Mask creation */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] > b[i]) ? -1 : 0;
    }
    
    for (int i = 0; i < N; i++) {
        sum += c[i];
    }
    
    return sum;
}

/* LT_EXPR (<) test */
int test_lt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    /* Pattern for < comparison */
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            sum += a[i];
        }
    }
    
    /* Mask creation */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? -1 : 0;
    }
    
    for (int i = 0; i < N; i++) {
        sum += c[i];
    }
    
    return sum;
}

/* LE_EXPR (<=) test */
int test_le_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    /* Pattern for <= comparison */
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            sum += a[i];
        }
    }
    
    /* Mask creation */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? -1 : 0;
    }
    
    for (int i = 0; i < N; i++) {
        sum += c[i];
    }
    
    return sum;
}

/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc
 * Specifically targets the GE_EXPR, GT_EXPR, LT_EXPR, and LE_EXPR cases
 * in the switch statement at lines 12216-12233
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Force results to be used to prevent optimization */
static volatile int g_result = 0;

/* Function to test GE_EXPR vectorization */
int test_ge_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    /* Pattern 1: Conditional sum with GE_EXPR - likely to trigger mask creation */
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    
    /* Pattern 2: Create mask from GE_EXPR comparison */
    for (int i = 0; i < N; i++) {
        /* This creates a mask: -1 for true, 0 for false */
        c[i] = (a[i] >= b[i]) ? -1 : 0;
    }
    
    /* Use the mask in a computation to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        sum += c[i];
    }
    
    return sum;
}

/* Function to test GT_EXPR vectorization */
int test_gt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    /* Conditional sum with GT_EXPR */
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    
    /* Create mask from GT_EXPR comparison */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] > b[i]) ? -1 : 0;
    }
    
    for (int i = 0; i < N; i++) {
        sum += c[i];
    }
    
    return sum;
}

/* Function to test LT_EXPR vectorization */
int test_lt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    /* Conditional sum with LT_EXPR */
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            sum += a[i];
        }
    }
    
    /* Create mask from LT_EXPR comparison */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? -1 : 0;
    }
    
    for (int i = 0; i < N; i++) {
        sum += c[i];
    }
    
    return sum;
}

/* Function to test LE_EXPR vectorization */
int test_le_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    /* Conditional sum with LE_EXPR */
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            sum += a[i];
        }
    }
    
    /* Create mask from LE_EXPR comparison */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? -1 : 0;
    }
    
    for (int i = 0; i < N; i++) {
        sum += c[i];
    }
    
    return sum;
}

/* Alternative test using short type - might trigger different code paths */
short test_ge_vectorize_short(ALIGNED short *a, ALIGNED short *b, ALIGNED short *c) {
    short sum = 0;
    
    /* Conditional sum with GE_EXPR on short */
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    
    /* Create mask from GE_EXPR comparison */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? -1 : 0;
    }
    
    for (int i = 0; i < N; i++) {
        sum += c[i];
    }
    
    return sum;
}

int main() {
    /* Aligned arrays to help vectorization */
    ALIGNED int a_int[N], b_int[N], c_int[N];
    ALIGNED short a_short[N], b_short[N], c_short[N];
    
    /* Initialize arrays with pattern that will trigger all comparison results */
    for (int i = 0; i < N; i++) {
        a_int[i] = i;
        b_int[i] = N/2;  /* Half will be >=, half < */
        
        a_short[i] = (short)(i % 1000);
        b_short[i] = 500;  /* Mix of true/false comparisons */
    }
    
    printf("Testing vector comparison expansions...\n");
    
    /* Test all comparison operators */
    int result_ge = test_ge_vectorize(a_int, b_int, c_int);
    int result_gt = test_gt_vectorize(a_int, b_int, c_int);
    int result_lt = test_lt_vectorize(a_int, b_int, c_int);
    int result_le = test_le_vectorize(a_int, b_int, c_int);
    short result_ge_short = test_ge_vectorize_short(a_short, b_short, c_short);
    
    /* Store results in volatile to prevent optimization */
    g_result = result_ge + result_gt + result_lt + result_le + result_ge_short;
    
    printf("Results: GE=%d, GT=%d, LT=%d, LE=%d, GE_short=%d\n",
           result_ge, result_gt, result_lt, result_le, result_ge_short);
    printf("Total (volatile): %d\n", g_result);
    
    /* Simple checksum to verify computation */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += c_int[i];
    }
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}

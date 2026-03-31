/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc
 * Specifically targets the GE_EXPR, GT_EXPR, LT_EXPR, and LE_EXPR cases
 * in the switch statement at lines 12216-12233
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Function to test GE_EXPR vectorization */
int test_ge_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    /* This loop should vectorize with GE_EXPR comparison */
    for (int i = 0; i < N; i++) {
        /* Create mask from comparison: -1 if a[i] >= b[i], 0 otherwise */
        int mask = (a[i] >= b[i]) ? -1 : 0;
        /* Use mask to conditionally accumulate values */
        c[i] = mask & a[i];
        sum += c[i];
    }
    
    return sum;
}

/* Function to test GT_EXPR vectorization */
int test_gt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        int mask = (a[i] > b[i]) ? -1 : 0;
        c[i] = mask & b[i];
        sum += c[i];
    }
    
    return sum;
}

/* Function to test LT_EXPR vectorization */
int test_lt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        int mask = (a[i] < b[i]) ? -1 : 0;
        c[i] = mask & a[i];
        sum += c[i];
    }
    
    return sum;
}

/* Function to test LE_EXPR vectorization */
int test_le_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        int mask = (a[i] <= b[i]) ? -1 : 0;
        c[i] = mask & b[i];
        sum += c[i];
    }
    
    return sum;
}

/* Alternative test using conditional reduction pattern */
int test_ge_reduction(ALIGNED short *a, ALIGNED short *b) {
    int count = 0;
    
    /* Count elements where a[i] >= b[i] */
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            count++;
        }
    }
    
    return count;
}

int main() {
    /* Aligned arrays to help vectorization */
    ALIGNED int a_int[N], b_int[N], c_int[N];
    ALIGNED short a_short[N], b_short[N];
    
    /* Initialize with pattern that ensures mixed comparison results */
    for (int i = 0; i < N; i++) {
        a_int[i] = i;
        b_int[i] = N/2;  /* Half will be >=, half < */
        a_short[i] = (short)(i % 256);
        b_short[i] = 128;
    }
    
    /* Test all comparison operators */
    int sum_ge = test_ge_vectorize(a_int, b_int, c_int);
    int sum_gt = test_gt_vectorize(a_int, b_int, c_int);
    int sum_lt = test_lt_vectorize(a_int, b_int, c_int);
    int sum_le = test_le_vectorize(a_int, b_int, c_int);
    int count_ge = test_ge_reduction(a_short, b_short);
    
    /* Use volatile to prevent optimization */
    volatile int result = sum_ge + sum_gt + sum_lt + sum_le + count_ge;
    
    printf("Results: GE=%d, GT=%d, LT=%d, LE=%d, Count=%d, Total=%d\n",
           sum_ge, sum_gt, sum_lt, sum_le, count_ge, result);
    
    /* Simple checksum verification */
    if (result != 0) {
        printf("Test completed successfully\n");
    }
    
    return 0;
}

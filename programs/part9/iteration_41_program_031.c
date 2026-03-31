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
    
    /* Loop with GE_EXPR comparison creating mask pattern */
    for (int i = 0; i < N; i++) {
        /* Create mask: -1 if a[i] >= b[i], 0 otherwise */
        int mask = (a[i] >= b[i]) ? -1 : 0;
        /* Use mask to conditionally accumulate */
        c[i] = mask & a[i];
        sum += c[i];
    }
    
    return sum;
}

/* Function to test GT_EXPR vectorization */
int test_gt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    /* Loop with GT_EXPR comparison */
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
    
    /* Loop with LT_EXPR comparison */
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
    
    /* Loop with LE_EXPR comparison */
    for (int i = 0; i < N; i++) {
        int mask = (a[i] <= b[i]) ? -1 : 0;
        c[i] = mask & b[i];
        sum += c[i];
    }
    
    return sum;
}

/* Alternative test using short type for different vectorization */
short test_ge_short_vectorize(ALIGNED short *a, ALIGNED short *b, ALIGNED short *c) {
    short sum = 0;
    
    for (int i = 0; i < N; i++) {
        short mask = (a[i] >= b[i]) ? -1 : 0;
        c[i] = mask & a[i];
        sum += c[i];
    }
    
    return sum;
}

int main() {
    /* Declare aligned arrays */
    ALIGNED int a_int[N], b_int[N], c_int[N];
    ALIGNED short a_short[N], b_short[N], c_short[N];
    
    /* Initialize with pattern that creates mix of true/false comparisons */
    for (int i = 0; i < N; i++) {
        a_int[i] = i;
        b_int[i] = N/2;  /* Half will be >=, half < */
        
        a_short[i] = (short)(i % 1000);
        b_short[i] = 500;  /* Mix of conditions */
    }
    
    /* Test all comparison operators */
    int sum_ge = test_ge_vectorize(a_int, b_int, c_int);
    int sum_gt = test_gt_vectorize(a_int, b_int, c_int);
    int sum_lt = test_lt_vectorize(a_int, b_int, c_int);
    int sum_le = test_le_vectorize(a_int, b_int, c_int);
    short sum_ge_short = test_ge_short_vectorize(a_short, b_short, c_short);
    
    /* Use results to prevent optimization */
    volatile int total = sum_ge + sum_gt + sum_lt + sum_le + sum_ge_short;
    
    printf("Results: GE=%d, GT=%d, LT=%d, LE=%d, GE_short=%d, Total=%d\n",
           sum_ge, sum_gt, sum_lt, sum_le, sum_ge_short, total);
    
    /* Simple checksum verification */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += c_int[i];
    }
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

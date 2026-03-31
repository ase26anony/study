/* Test for vector comparison expansion in tree-vect-stmts.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Function to test GE_EXPR vectorization */
int test_ge_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    /* This loop should vectorize with GE_EXPR comparison */
    for (int i = 0; i < N; i++) {
        /* Create mask from comparison: -1 if true, 0 if false */
        int mask = (a[i] >= b[i]) ? -1 : 0;
        /* Use mask to selectively accumulate values */
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

/* Alternative test using conditional reduction */
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
    ALIGNED int a[N], b[N], c[N];
    ALIGNED short as[N], bs[N];
    
    /* Initialize arrays with pattern that triggers comparisons */
    for (int i = 0; i < N; i++) {
        a[i] = i - N/2;          /* Range: -512 to 511 */
        b[i] = i % 100;          /* Range: 0 to 99 */
        as[i] = (short)(i - N/2);
        bs[i] = (short)(i % 100);
    }
    
    /* Test all comparison operators */
    int sum_ge = test_ge_vectorize(a, b, c);
    int sum_gt = test_gt_vectorize(a, b, c);
    int sum_lt = test_lt_vectorize(a, b, c);
    int sum_le = test_le_vectorize(a, b, c);
    int count_ge = test_ge_reduction(as, bs);
    
    /* Use volatile to prevent optimization */
    volatile int result = 0;
    result = sum_ge + sum_gt + sum_lt + sum_le + count_ge;
    
    printf("Results: GE=%d, GT=%d, LT=%d, LE=%d, CountGE=%d\n", 
           sum_ge, sum_gt, sum_lt, sum_le, count_ge);
    printf("Checksum: %d\n", result);
    
    /* Verify a few values */
    printf("Sample check: c[0]=%d, c[%d]=%d, c[%d]=%d\n", 
           c[0], N/4, c[N/4], N-1, c[N-1]);
    
    return 0;
}

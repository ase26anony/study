/* Test for vector comparison expansion in tree-vect-stmts.cc */
/* Specifically targeting GE_EXPR, GT_EXPR, LT_EXPR, LE_EXPR cases */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Function to test GE_EXPR vectorization */
int test_ge_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    /* This should trigger GE_EXPR expansion */
    for (int i = 0; i < N; i++) {
        /* Create mask from comparison - likely to use bitwise expansion */
        c[i] = (a[i] >= b[i]) ? -1 : 0;
        /* Use mask in computation to prevent optimization */
        sum += c[i] & a[i];
    }
    
    return sum;
}

/* Function to test GT_EXPR vectorization */
int test_gt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] > b[i]) ? -1 : 0;
        sum += c[i] & b[i];
    }
    
    return sum;
}

/* Function to test LT_EXPR vectorization */
int test_lt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? -1 : 0;
        sum += c[i] & a[i];
    }
    
    return sum;
}

/* Function to test LE_EXPR vectorization */
int test_le_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? -1 : 0;
        sum += c[i] & b[i];
    }
    
    return sum;
}

/* Alternative pattern: conditional reduction */
int test_ge_reduction(ALIGNED int *a, ALIGNED int *b) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    
    return sum;
}

/* Test with different data types */
short test_ge_short(ALIGNED short *a, ALIGNED short *b, ALIGNED short *c) {
    short sum = 0;
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? -1 : 0;
        sum += c[i] & a[i];
    }
    
    return sum;
}

int main() {
    /* Aligned arrays to help vectorization */
    ALIGNED int a[N], b[N], c[N];
    ALIGNED short as[N], bs[N], cs[N];
    
    /* Initialize with pattern that creates mixed comparison results */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N/2;  /* Half will be >=, half < */
        as[i] = (short)i;
        bs[i] = (short)(N/2);
    }
    
    /* Test all comparison types */
    int sum_ge = test_ge_vectorize(a, b, c);
    int sum_gt = test_gt_vectorize(a, b, c);
    int sum_lt = test_lt_vectorize(a, b, c);
    int sum_le = test_le_vectorize(a, b, c);
    int sum_ge_red = test_ge_reduction(a, b);
    short sum_ge_short = test_ge_short(as, bs, cs);
    
    /* Use results to prevent optimization */
    volatile int total = sum_ge + sum_gt + sum_lt + sum_le + sum_ge_red + sum_ge_short;
    
    printf("Results: %d %d %d %d %d %d\n", 
           sum_ge, sum_gt, sum_lt, sum_le, sum_ge_red, sum_ge_short);
    
    /* Simple checksum verification */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += c[i];
    }
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

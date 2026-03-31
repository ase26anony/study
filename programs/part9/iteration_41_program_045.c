/* Test program to trigger specific vector comparison expansion in tree-vect-stmts.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Force side effects to prevent optimization */
static volatile int result = 0;

/* Test GE_EXPR (>=) comparison */
int test_ge_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    /* Pattern 1: Conditional sum with >= */
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    
    /* Pattern 2: Create mask from >= comparison */
    for (int i = 0; i < N; i++) {
        /* This creates a mask: -1 if true, 0 if false */
        c[i] = (a[i] >= b[i]) ? -1 : 0;
    }
    
    /* Use the mask in computation */
    for (int i = 0; i < N; i++) {
        sum += c[i] & a[i];
    }
    
    return sum;
}

/* Test GT_EXPR (>) comparison */
int test_gt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] > b[i]) ? -1 : 0;
    }
    
    for (int i = 0; i < N; i++) {
        sum += c[i] & b[i];
    }
    
    return sum;
}

/* Test LT_EXPR (<) comparison */
int test_lt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            sum += b[i];
        }
    }
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? -1 : 0;
    }
    
    for (int i = 0; i < N; i++) {
        sum += c[i] & a[i];
    }
    
    return sum;
}

/* Test LE_EXPR (<=) comparison */
int test_le_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            sum += b[i];
        }
    }
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? -1 : 0;
    }
    
    for (int i = 0; i < N; i++) {
        sum += c[i] & b[i];
    }
    
    return sum;
}

/* Test with short type to potentially trigger different code paths */
short test_short_ge_vectorize(ALIGNED short *a, ALIGNED short *b, ALIGNED short *c) {
    short sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? -1 : 0;
    }
    
    for (int i = 0; i < N; i++) {
        sum += c[i] & a[i];
    }
    
    return sum;
}

int main() {
    /* Aligned arrays to help vectorization */
    ALIGNED int a_int[N], b_int[N], c_int[N];
    ALIGNED short a_short[N], b_short[N], c_short[N];
    
    /* Initialize with pattern that creates mixed true/false comparisons */
    for (int i = 0; i < N; i++) {
        a_int[i] = i;
        b_int[i] = N/2;  /* Half will be >=, half < */
        a_short[i] = i % 100;
        b_short[i] = 50;  /* Mixed comparisons */
    }
    
    printf("Testing vector comparison expansions...\n");
    
    /* Test all comparison types */
    int sum_ge = test_ge_vectorize(a_int, b_int, c_int);
    int sum_gt = test_gt_vectorize(a_int, b_int, c_int);
    int sum_lt = test_lt_vectorize(a_int, b_int, c_int);
    int sum_le = test_le_vectorize(a_int, b_int, c_int);
    short sum_short_ge = test_short_ge_vectorize(a_short, b_short, c_short);
    
    /* Use results to prevent optimization */
    result = sum_ge + sum_gt + sum_lt + sum_le + sum_short_ge;
    
    printf("Results: GE=%d, GT=%d, LT=%d, LE=%d, short_GE=%d\n",
           sum_ge, sum_gt, sum_lt, sum_le, sum_short_ge);
    printf("Total (volatile): %d\n", result);
    
    /* Quick sanity check */
    if (sum_ge > 0 && sum_gt > 0 && sum_lt > 0 && sum_le > 0) {
        printf("All comparisons produced non-zero results - test likely executed.\n");
    }
    
    return 0;
}

/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Function to test GE_EXPR vectorization */
int test_ge_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    /* Loop with GE_EXPR comparison creating a mask */
    for (int i = 0; i < N; i++) {
        /* Create mask: -1 if a[i] >= b[i], 0 otherwise */
        int mask = (a[i] >= b[i]) ? -1 : 0;
        /* Use mask to conditionally add to sum */
        sum += a[i] & mask;
        /* Also store mask for side effect */
        c[i] = mask;
    }
    
    return sum;
}

/* Function to test GT_EXPR vectorization */
int test_gt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        int mask = (a[i] > b[i]) ? -1 : 0;
        sum += b[i] & mask;
        c[i] = mask;
    }
    
    return sum;
}

/* Function to test LT_EXPR vectorization */
int test_lt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        int mask = (a[i] < b[i]) ? -1 : 0;
        sum += a[i] & mask;
        c[i] = mask;
    }
    
    return sum;
}

/* Function to test LE_EXPR vectorization */
int test_le_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        int mask = (a[i] <= b[i]) ? -1 : 0;
        sum += b[i] & mask;
        c[i] = mask;
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
    /* Aligned arrays to help vectorization */
    ALIGNED int a_int[N], b_int[N], c_int[N];
    ALIGNED short a_short[N], b_short[N];
    
    /* Initialize with pattern that triggers comparisons */
    for (int i = 0; i < N; i++) {
        a_int[i] = i;
        b_int[i] = N/2;  /* Half will be >=, half < */
        a_short[i] = i % 256;
        b_short[i] = 128;
    }
    
    printf("Testing vector comparison expansions...\n");
    
    /* Test all comparison operators */
    int sum_ge = test_ge_vectorize(a_int, b_int, c_int);
    int sum_gt = test_gt_vectorize(a_int, b_int, c_int);
    int sum_lt = test_lt_vectorize(a_int, b_int, c_int);
    int sum_le = test_le_vectorize(a_int, b_int, c_int);
    int count_ge = test_ge_reduction(a_short, b_short);
    
    /* Use volatile to prevent optimization */
    volatile int result = sum_ge + sum_gt + sum_lt + sum_le + count_ge;
    
    printf("Results: GE=%d, GT=%d, LT=%d, LE=%d, CountGE=%d\n", 
           sum_ge, sum_gt, sum_lt, sum_le, count_ge);
    printf("Checksum: %d\n", result);
    
    /* Verify some mask values */
    printf("Sample masks (indices 0, %d, %d): %d, %d, %d\n", 
           N/4, N/2, c_int[0], c_int[N/4], c_int[N/2]);
    
    return 0;
}

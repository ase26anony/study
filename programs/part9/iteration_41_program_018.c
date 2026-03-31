/* Test program to trigger specific vector comparison expansion in tree-vect-stmts.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Function targeting GE_EXPR case */
int test_ge_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    /* This should trigger the GE_EXPR expansion path */
    for (int i = 0; i < N; i++) {
        /* Create mask from comparison: -1 for true, 0 for false */
        int mask = (a[i] >= b[i]) ? -1 : 0;
        /* Use mask in computation to prevent optimization */
        c[i] = mask & a[i];
        sum += c[i];
    }
    
    return sum;
}

/* Function targeting GT_EXPR case */
int test_gt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        int mask = (a[i] > b[i]) ? -1 : 0;
        c[i] = mask & b[i];
        sum += c[i];
    }
    
    return sum;
}

/* Function targeting LT_EXPR case */
int test_lt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        int mask = (a[i] < b[i]) ? -1 : 0;
        c[i] = mask & a[i];
        sum += c[i];
    }
    
    return sum;
}

/* Function targeting LE_EXPR case */
int test_le_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        int mask = (a[i] <= b[i]) ? -1 : 0;
        c[i] = mask & b[i];
        sum += c[i];
    }
    
    return sum;
}

/* Alternative pattern using conditional reduction */
int test_ge_reduction(ALIGNED int *a, ALIGNED int *b) {
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
    ALIGNED int a[N];
    ALIGNED int b[N];
    ALIGNED int c[N];
    
    /* Initialize arrays with pattern that triggers comparisons */
    for (int i = 0; i < N; i++) {
        a[i] = i;               /* 0, 1, 2, ..., N-1 */
        b[i] = N/2;             /* All N/2 */
    }
    
    /* Test all comparison types */
    int result_ge = test_ge_vectorize(a, b, c);
    int result_gt = test_gt_vectorize(a, b, c);
    int result_lt = test_lt_vectorize(a, b, c);
    int result_le = test_le_vectorize(a, b, c);
    int count_ge = test_ge_reduction(a, b);
    
    /* Use volatile to prevent optimization */
    volatile int checksum = result_ge + result_gt + result_lt + result_le + count_ge;
    
    printf("Results: GE=%d, GT=%d, LT=%d, LE=%d, CountGE=%d\n", 
           result_ge, result_gt, result_lt, result_le, count_ge);
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

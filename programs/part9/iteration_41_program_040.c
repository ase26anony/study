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
        /* Use mask to conditionally add to sum */
        sum += mask & a[i];
        /* Also store mask to array to prevent optimization */
        c[i] = mask;
    }
    
    return sum;
}

/* Function to test GT_EXPR vectorization */
int test_gt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        int mask = (a[i] > b[i]) ? -1 : 0;
        sum += mask & b[i];
        c[i] = mask;
    }
    
    return sum;
}

/* Function to test LT_EXPR vectorization */
int test_lt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        int mask = (a[i] < b[i]) ? -1 : 0;
        sum += mask & a[i];
        c[i] = mask;
    }
    
    return sum;
}

/* Function to test LE_EXPR vectorization */
int test_le_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        int mask = (a[i] <= b[i]) ? -1 : 0;
        sum += mask & b[i];
        c[i] = mask;
    }
    
    return sum;
}

/* Alternative test using conditional reduction pattern */
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

/* Test with short type to potentially trigger different vectorization */
short test_ge_short(ALIGNED short *a, ALIGNED short *b, ALIGNED short *c) {
    short sum = 0;
    
    for (int i = 0; i < N; i++) {
        short mask = (a[i] >= b[i]) ? -1 : 0;
        sum += mask & a[i];
        c[i] = mask;
    }
    
    return sum;
}

int main() {
    /* Declare aligned arrays */
    ALIGNED int a[N], b[N], c[N];
    ALIGNED short as[N], bs[N], cs[N];
    
    /* Initialize arrays with pattern that ensures some comparisons are true and some false */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N/2;  /* Half will be >= N/2, half < */
        c[i] = 0;
        
        as[i] = (short)i;
        bs[i] = (short)(N/2);
        cs[i] = 0;
    }
    
    /* Test all comparison types */
    int sum_ge = test_ge_vectorize(a, b, c);
    int sum_gt = test_gt_vectorize(a, b, c);
    int sum_lt = test_lt_vectorize(a, b, c);
    int sum_le = test_le_vectorize(a, b, c);
    int count_ge = test_ge_reduction(a, b);
    short sum_ge_short = test_ge_short(as, bs, cs);
    
    /* Print results to prevent optimization and verify correctness */
    printf("GE sum: %d\n", sum_ge);
    printf("GT sum: %d\n", sum_gt);
    printf("LT sum: %d\n", sum_lt);
    printf("LE sum: %d\n", sum_le);
    printf("GE count: %d\n", count_ge);
    printf("GE short sum: %d\n", (int)sum_ge_short);
    
    /* Simple checksum to ensure all computations were performed */
    int checksum = sum_ge + sum_gt + sum_lt + sum_le + count_ge + sum_ge_short;
    printf("Checksum: %d\n", checksum);
    
    /* Verify a few mask values */
    printf("Sample masks (c[0], c[N/4], c[N/2], c[3*N/4]): %d, %d, %d, %d\n",
           c[0], c[N/4], c[N/2], c[3*N/4]);
    
    return 0;
}

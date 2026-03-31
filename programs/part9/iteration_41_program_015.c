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
    
    /* This should trigger GE_EXPR expansion when vectorized */
    for (int i = 0; i < N; i++) {
        /* Create mask from comparison: -1 for true, 0 for false */
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
        /* Create mask from > comparison */
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
        /* Create mask from < comparison */
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
        /* Create mask from <= comparison */
        int mask = (a[i] <= b[i]) ? -1 : 0;
        c[i] = mask & b[i];
        sum += c[i];
    }
    return sum;
}

/* Alternative test using short type to potentially trigger different paths */
short test_ge_short_vectorize(ALIGNED short *a, ALIGNED short *b, ALIGNED short *c) {
    short sum = 0;
    
    for (int i = 0; i < N; i++) {
        /* Different pattern: conditional assignment using comparison */
        c[i] = (a[i] >= b[i]) ? a[i] : b[i];
        sum += c[i];
    }
    return sum;
}

/* Test with reduction pattern */
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
    /* Aligned arrays to help vectorization */
    ALIGNED int a[N], b[N], c[N];
    ALIGNED short as[N], bs[N], cs[N];
    
    /* Initialize with pattern that will trigger various comparison results */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N/2;  /* Half will be >=, half < */
        as[i] = (short)i;
        bs[i] = (short)(N/2);
    }
    
    /* Test all comparison operators */
    int sum_ge = test_ge_vectorize(a, b, c);
    int sum_gt = test_gt_vectorize(a, b, c);
    int sum_lt = test_lt_vectorize(a, b, c);
    int sum_le = test_le_vectorize(a, b, c);
    short sum_short = test_ge_short_vectorize(as, bs, cs);
    int count = test_ge_reduction(a, b);
    
    /* Use results to prevent optimization */
    printf("GE sum: %d\n", sum_ge);
    printf("GT sum: %d\n", sum_gt);
    printf("LT sum: %d\n", sum_lt);
    printf("LE sum: %d\n", sum_le);
    printf("Short sum: %d\n", (int)sum_short);
    printf("Count >= : %d\n", count);
    
    /* Simple checksum to verify execution */
    int checksum = sum_ge + sum_gt + sum_lt + sum_le + sum_short + count;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

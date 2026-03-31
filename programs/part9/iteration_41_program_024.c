/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc
   Specifically targets the GE_EXPR, GT_EXPR, LT_EXPR, and LE_EXPR cases
   in the switch statement at lines 12216-12233 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Function to test GE_EXPR vectorization with mask creation */
int test_ge_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    /* This creates a mask pattern: -1 for true, 0 for false */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? -1 : 0;
        sum += c[i];  /* Use the mask to prevent optimization */
    }
    return sum;
}

/* Function to test GT_EXPR vectorization */
int test_gt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] > b[i]) ? -1 : 0;
        sum += c[i];
    }
    return sum;
}

/* Function to test LT_EXPR vectorization */
int test_lt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? -1 : 0;
        sum += c[i];
    }
    return sum;
}

/* Function to test LE_EXPR vectorization */
int test_le_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? -1 : 0;
        sum += c[i];
    }
    return sum;
}

/* Alternative test using conditional reduction (another common pattern) */
int test_ge_reduction(ALIGNED int *a, ALIGNED int *b) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

/* Test with different data types to explore different paths */
short test_ge_short(ALIGNED short *a, ALIGNED short *b, ALIGNED short *c) {
    short sum = 0;
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? -1 : 0;
        sum += c[i];
    }
    return sum;
}

int main() {
    /* Aligned arrays to help vectorization */
    ALIGNED int a[N], b[N], c[N];
    ALIGNED short as[N], bs[N], cs[N];
    
    /* Initialize with pattern that will trigger comparisons */
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
    int sum_red = test_ge_reduction(a, b);
    short sum_short = test_ge_short(as, bs, cs);
    
    /* Print results to prevent optimization and verify correctness */
    printf("GE mask sum: %d\n", sum_ge);
    printf("GT mask sum: %d\n", sum_gt);
    printf("LT mask sum: %d\n", sum_lt);
    printf("LE mask sum: %d\n", sum_le);
    printf("GE reduction sum: %d\n", sum_red);
    printf("GE short mask sum: %d\n", (int)sum_short);
    
    /* Simple checksum to ensure all computations happened */
    int checksum = sum_ge + sum_gt + sum_lt + sum_le + sum_red + sum_short;
    printf("Total checksum: %d\n", checksum);
    
    return 0;
}

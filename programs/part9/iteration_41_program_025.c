/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc
   Specifically targets the GE_EXPR, GT_EXPR, LT_EXPR, and LE_EXPR cases
   in the switch statement at lines 12216-12233 */

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
        /* Use mask to conditionally add to sum */
        sum += mask & a[i];
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
        sum += mask & a[i];
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
        sum += mask & a[i];
        c[i] = mask;
    }
    
    return sum;
}

/* Alternative pattern: conditional reduction */
int test_ge_reduction(ALIGNED int *a, ALIGNED int *b) {
    int count = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            count++;
        }
    }
    
    return count;
}

/* Another pattern: conditional select */
void test_ge_select(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? a[i] : b[i];
    }
}

int main() {
    /* Use volatile to prevent complete optimization */
    ALIGNED int a[N];
    ALIGNED int b[N];
    ALIGNED int c[N];
    
    /* Initialize arrays with pattern that will trigger comparisons */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N/2;  /* Half will be >=, half < */
    }
    
    printf("Testing vector comparison expansions...\n");
    
    /* Test GE_EXPR */
    int ge_result = test_ge_vectorize(a, b, c);
    printf("GE_EXPR test result: %d\n", ge_result);
    
    /* Test GT_EXPR */
    int gt_result = test_gt_vectorize(a, b, c);
    printf("GT_EXPR test result: %d\n", gt_result);
    
    /* Test LT_EXPR */
    int lt_result = test_lt_vectorize(a, b, c);
    printf("LT_EXPR test result: %d\n", lt_result);
    
    /* Test LE_EXPR */
    int le_result = test_le_vectorize(a, b, c);
    printf("LE_EXPR test result: %d\n", le_result);
    
    /* Test conditional reduction */
    int count = test_ge_reduction(a, b);
    printf("GE_EXPR reduction count: %d\n", count);
    
    /* Test conditional select */
    test_ge_select(a, b, c);
    printf("GE_EXPR select test completed\n");
    
    /* Simple checksum to ensure computations happened */
    int checksum = ge_result + gt_result + lt_result + le_result + count;
    printf("Total checksum: %d\n", checksum);
    
    return 0;
}

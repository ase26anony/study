/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc
 * Specifically targets the GE_EXPR case (lines 12216-12233)
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Function to test GE_EXPR vectorization */
int test_ge_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    /* Pattern 1: Conditional sum with GE_EXPR - likely to trigger mask creation */
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    
    /* Pattern 2: Mask creation with GE_EXPR - direct mask generation */
    for (int i = 0; i < N; i++) {
        /* Create mask: -1 for true, 0 for false */
        c[i] = (a[i] >= b[i]) ? -1 : 0;
    }
    
    /* Pattern 3: Conditional assignment using mask */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? a[i] : b[i];
    }
    
    return sum;
}

/* Function to test GT_EXPR vectorization */
int test_gt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    
    /* Create mask with GT_EXPR */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] > b[i]) ? -1 : 0;
    }
    
    return sum;
}

/* Function to test LT_EXPR vectorization */
int test_lt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            sum += a[i];
        }
    }
    
    /* Create mask with LT_EXPR */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? -1 : 0;
    }
    
    return sum;
}

/* Function to test LE_EXPR vectorization */
int test_le_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            sum += a[i];
        }
    }
    
    /* Create mask with LE_EXPR */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? -1 : 0;
    }
    
    return sum;
}

/* Test with short type to potentially trigger different vectorization */
short test_ge_short_vectorize(ALIGNED short *a, ALIGNED short *b, ALIGNED short *c) {
    short sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    
    /* Mask creation with short type */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? -1 : 0;
    }
    
    return sum;
}

int main() {
    /* Aligned arrays to help vectorization */
    ALIGNED int a_int[N], b_int[N], c_int[N];
    ALIGNED short a_short[N], b_short[N], c_short[N];
    
    /* Initialize arrays with pattern that creates mix of true/false comparisons */
    for (int i = 0; i < N; i++) {
        a_int[i] = i;
        b_int[i] = N/2;  /* Half will be >=, half < */
        
        a_short[i] = (short)(i % 1000);
        b_short[i] = 500;  /* Mix of true/false */
    }
    
    printf("Testing vector comparison expansions...\n");
    
    /* Test all comparison operators to cover the switch block */
    int sum_ge = test_ge_vectorize(a_int, b_int, c_int);
    int sum_gt = test_gt_vectorize(a_int, b_int, c_int);
    int sum_lt = test_lt_vectorize(a_int, b_int, c_int);
    int sum_le = test_le_vectorize(a_int, b_int, c_int);
    short sum_ge_short = test_ge_short_vectorize(a_short, b_short, c_short);
    
    /* Use volatile to prevent optimization */
    volatile int result = 0;
    result = sum_ge + sum_gt + sum_lt + sum_le + sum_ge_short;
    
    /* Simple checksum to verify computation */
    printf("Result checksum: %d\n", result);
    
    /* Verify some mask values */
    printf("Sample mask values (should be -1 or 0):\n");
    printf("c_int[0] = %d, c_int[N/4] = %d, c_int[N/2] = %d, c_int[3*N/4] = %d\n",
           c_int[0], c_int[N/4], c_int[N/2], c_int[3*N/4]);
    
    return 0;
}

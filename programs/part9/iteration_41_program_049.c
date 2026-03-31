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
    
    /* Pattern 1: Conditional sum with GE comparison */
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    
    /* Pattern 2: Create mask from GE comparison */
    for (int i = 0; i < N; i++) {
        // Creates mask: -1 (all bits set) if true, 0 if false
        c[i] = (a[i] >= b[i]) ? -1 : 0;
    }
    
    /* Pattern 3: Conditional assignment using mask */
    for (int i = 0; i < N; i++) {
        // Use the mask created above for conditional operation
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
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? -1 : 0;
    }
    
    return sum;
}

int main() {
    /* Use volatile to prevent complete optimization */
    ALIGNED int a[N];
    ALIGNED int b[N];
    ALIGNED int c[N];
    
    /* Initialize arrays with pattern that triggers comparisons */
    for (int i = 0; i < N; i++) {
        a[i] = i;                    /* 0, 1, 2, ..., N-1 */
        b[i] = N/2;                  /* Constant N/2 */
    }
    
    printf("Testing vector comparison expansions...\n");
    
    /* Test all comparison operators to cover the switch cases */
    int sum_ge = test_ge_vectorize(a, b, c);
    int sum_gt = test_gt_vectorize(a, b, c);
    int sum_lt = test_lt_vectorize(a, b, c);
    int sum_le = test_le_vectorize(a, b, c);
    
    /* Use results to prevent optimization */
    volatile int total = sum_ge + sum_gt + sum_lt + sum_le;
    
    /* Also use array c to prevent dead code elimination */
    volatile int check = c[N/4] + c[N/2] + c[3*N/4];
    
    printf("Results: GE=%d, GT=%d, LT=%d, LE=%d\n", 
           sum_ge, sum_gt, sum_lt, sum_le);
    printf("Check value: %d\n", check);
    
    return 0;
}

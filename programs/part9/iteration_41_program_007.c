/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc
 * Specifically targets the GE_EXPR case (lines 12216-12233) and related cases
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
    
    /* Pattern 3: Conditional assignment using mask logic */
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
        /* Also create mask */
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
        /* Also create mask */
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
        /* Also create mask */
        c[i] = (a[i] <= b[i]) ? -1 : 0;
    }
    
    return sum;
}

/* Helper to initialize arrays with interesting patterns */
void init_arrays(ALIGNED int *a, ALIGNED int *b) {
    for (int i = 0; i < N; i++) {
        /* Create pattern where some elements satisfy >=, others don't */
        a[i] = i;                     /* 0, 1, 2, 3, ... */
        b[i] = N/2;                   /* 512, 512, 512, ... */
    }
}

int main() {
    /* Use aligned arrays to help vectorization */
    ALIGNED int a[N];
    ALIGNED int b[N];
    ALIGNED int c[N];
    
    /* Initialize with pattern that will trigger comparisons */
    init_arrays(a, b);
    
    /* Test all comparison operators to cover the entire switch block */
    int sum_ge = test_ge_vectorize(a, b, c);
    int sum_gt = test_gt_vectorize(a, b, c);
    int sum_lt = test_lt_vectorize(a, b, c);
    int sum_le = test_le_vectorize(a, b, c);
    
    /* Use results to prevent optimization */
    volatile int total = sum_ge + sum_gt + sum_lt + sum_le;
    
    /* Also use array c to prevent dead code elimination */
    volatile int check = c[N/4] + c[N/2] + c[3*N/4];
    
    printf("Results: %d, %d\n", total, check);
    printf("Test completed - check compilation with -fopt-info-vec\n");
    
    return 0;
}

/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc
 * Specifically targets the GE_EXPR, GT_EXPR, LT_EXPR, and LE_EXPR cases
 * in the switch statement at lines 12216-12233
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Function to test GE_EXPR (greater than or equal) vectorization */
int test_ge_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    /* This loop should vectorize with GE_EXPR comparison
     * The pattern (a[i] >= b[i]) ? -1 : 0 creates a mask
     * which may trigger the specific expansion path
     */
    for (int i = 0; i < N; i++) {
        // Create mask: -1 for true, 0 for false
        int mask = (a[i] >= b[i]) ? -1 : 0;
        // Use mask to conditionally accumulate
        c[i] = mask & a[i];
        sum += c[i];
    }
    return sum;
}

/* Function to test GT_EXPR (greater than) vectorization */
int test_gt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        int mask = (a[i] > b[i]) ? -1 : 0;
        c[i] = mask & b[i];
        sum += c[i];
    }
    return sum;
}

/* Function to test LT_EXPR (less than) vectorization */
int test_lt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        int mask = (a[i] < b[i]) ? -1 : 0;
        c[i] = mask & a[i];
        sum += c[i];
    }
    return sum;
}

/* Function to test LE_EXPR (less than or equal) vectorization */
int test_le_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        int mask = (a[i] <= b[i]) ? -1 : 0;
        c[i] = mask & b[i];
        sum += c[i];
    }
    return sum;
}

/* Alternative test using short type to potentially trigger different paths */
short test_ge_vectorize_short(ALIGNED short *a, ALIGNED short *b, ALIGNED short *c) {
    short sum = 0;
    
    for (int i = 0; i < N; i++) {
        short mask = (a[i] >= b[i]) ? -1 : 0;
        c[i] = mask & a[i];
        sum += c[i];
    }
    return sum;
}

/* Test with conditional reduction pattern */
int test_ge_reduction(ALIGNED int *a, ALIGNED int *b) {
    int count = 0;
    
    /* Count elements where a[i] >= b[i]
     * This creates a vector comparison and reduction
     */
    for (int i = 0; i < N; i++) {
        count += (a[i] >= b[i]) ? 1 : 0;
    }
    return count;
}

int main() {
    /* Aligned arrays to help vectorization */
    ALIGNED int a_int[N], b_int[N], c_int[N];
    ALIGNED short a_short[N], b_short[N], c_short[N];
    
    /* Initialize with pattern that will trigger comparisons */
    for (int i = 0; i < N; i++) {
        a_int[i] = i;
        b_int[i] = N/2;  // Half will be >=, half <
        
        a_short[i] = (short)(i % 100);
        b_short[i] = 50;  // Mixed comparison results
    }
    
    printf("Testing vector comparison expansions in tree-vect-stmts.cc\n");
    
    /* Test all comparison operators to cover the switch cases */
    int result_ge = test_ge_vectorize(a_int, b_int, c_int);
    printf("GE_EXPR test result: %d\n", result_ge);
    
    int result_gt = test_gt_vectorize(a_int, b_int, c_int);
    printf("GT_EXPR test result: %d\n", result_gt);
    
    int result_lt = test_lt_vectorize(a_int, b_int, c_int);
    printf("LT_EXPR test result: %d\n", result_lt);
    
    int result_le = test_le_vectorize(a_int, b_int, c_int);
    printf("LE_EXPR test result: %d\n", result_le);
    
    /* Test with short type */
    short result_short = test_ge_vectorize_short(a_short, b_short, c_short);
    printf("GE_EXPR (short) test result: %d\n", (int)result_short);
    
    /* Test conditional reduction */
    int count = test_ge_reduction(a_int, b_int);
    printf("GE_EXPR reduction count: %d (expected ~%d)\n", count, N/2 + 1);
    
    /* Use volatile to prevent optimization */
    volatile int check = result_ge + result_gt + result_lt + result_le + result_short + count;
    printf("Final checksum: %d\n", (int)check);
    
    return 0;
}

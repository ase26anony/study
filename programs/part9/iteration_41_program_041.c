/* Test program to trigger vector comparison expansion in GCC's tree-vect-stmts.cc
 * Specifically targets the GE_EXPR, GT_EXPR, LT_EXPR, and LE_EXPR cases
 * in the switch statement at lines 12216-12233
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Function prototypes */
int test_ge_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c);
int test_gt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c);
int test_lt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c);
int test_le_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c);

int main(void) {
    /* Create aligned arrays to help vectorization */
    ALIGNED int a[N];
    ALIGNED int b[N];
    ALIGNED int c[N];
    
    /* Initialize arrays with pattern that will trigger comparisons */
    for (int i = 0; i < N; i++) {
        a[i] = i;                    /* 0, 1, 2, ..., N-1 */
        b[i] = N/2;                  /* All N/2 (512) */
    }
    
    printf("Testing vector comparison expansions in GCC...\n");
    
    /* Test GE_EXPR case (>=) - PRIMARY TARGET */
    int ge_result = test_ge_vectorize(a, b, c);
    printf("GE_EXPR test result: %d (expected positive value)\n", ge_result);
    
    /* Test GT_EXPR case (>) */
    int gt_result = test_gt_vectorize(a, b, c);
    printf("GT_EXPR test result: %d (expected positive value)\n", gt_result);
    
    /* Test LT_EXPR case (<) */
    int lt_result = test_lt_vectorize(a, b, c);
    printf("LT_EXPR test result: %d (expected positive value)\n", lt_result);
    
    /* Test LE_EXPR case (<=) */
    int le_result = test_le_vectorize(a, b, c);
    printf("LE_EXPR test result: %d (expected positive value)\n", le_result);
    
    /* Verify some results to prevent dead code elimination */
    volatile int checksum = ge_result + gt_result + lt_result + le_result;
    printf("Total checksum: %d\n", checksum);
    
    return 0;
}

/* Test function for GE_EXPR (>=) comparison
 * This pattern creates a mask from the comparison result
 * which should trigger the specific expansion path:
 *   bitop1 = BIT_NOT_EXPR;
 *   bitop2 = BIT_IOR_EXPR;
 */
int test_ge_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    /* Loop with GE_EXPR comparison creating a mask pattern
     * The ternary operator with -1 and 0 creates a mask that
     * should trigger the bitwise expansion */
    for (int i = 0; i < N; i++) {
        /* Create mask: -1 if a[i] >= b[i], 0 otherwise */
        int mask = (a[i] >= b[i]) ? -1 : 0;
        
        /* Use mask to conditionally accumulate values */
        c[i] = mask & a[i];
        sum += c[i];
    }
    
    return sum;
}

/* Test function for GT_EXPR (>) comparison */
int test_gt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        /* Create mask: -1 if a[i] > b[i], 0 otherwise */
        int mask = (a[i] > b[i]) ? -1 : 0;
        
        /* Conditional accumulation using mask */
        c[i] = mask & a[i];
        sum += c[i];
    }
    
    return sum;
}

/* Test function for LT_EXPR (<) comparison
 * Note: This should trigger std::swap(cond_expr0, cond_expr1)
 */
int test_lt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        /* Create mask: -1 if a[i] < b[i], 0 otherwise */
        int mask = (a[i] < b[i]) ? -1 : 0;
        
        /* Alternative pattern: use mask in bitwise operation */
        c[i] = (mask & a[i]) | (~mask & b[i]);  /* Select a[i] or b[i] based on mask */
        sum += c[i];
    }
    
    return sum;
}

/* Test function for LE_EXPR (<=) comparison
 * Note: This should trigger std::swap(cond_expr0, cond_expr1)
 */
int test_le_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        /* Create mask: -1 if a[i] <= b[i], 0 otherwise */
        int mask = (a[i] <= b[i]) ? -1 : 0;
        
        /* Use mask in conditional reduction pattern */
        if (a[i] <= b[i]) {
            sum += a[i];
        }
        c[i] = mask;
    }
    
    return sum;
}

/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc
 * Specifically targets the GE_EXPR, GT_EXPR, LT_EXPR, and LE_EXPR cases
 * in the switch statement at lines 12216-12233
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Function prototypes */
int test_ge_vectorize(void);
int test_gt_vectorize(void);
int test_lt_vectorize(void);
int test_le_vectorize(void);

/* Global aligned arrays to ensure vectorization */
ALIGNED int a[N];
ALIGNED int b[N];
ALIGNED int mask_result[N];
ALIGNED int select_result[N];

/* Initialize arrays with pattern that creates mixed comparison results */
void init_arrays(void) {
    for (int i = 0; i < N; i++) {
        a[i] = i;                     /* 0, 1, 2, ..., N-1 */
        b[i] = N/2;                   /* All N/2 (512) */
        mask_result[i] = 0;
        select_result[i] = 0;
    }
}

/* Test GE_EXPR (>=) - primary target for uncovered lines */
int test_ge_vectorize(void) {
    int sum = 0;
    
    /* Pattern 1: Conditional sum using >= comparison
     * This creates a reduction with comparison that should vectorize */
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    
    /* Pattern 2: Create mask from comparison result
     * This directly maps to the bitwise expansion in the uncovered code */
    for (int i = 0; i < N; i++) {
        /* Create mask: -1 for true, 0 for false
         * This pattern often triggers the specific expansion path */
        mask_result[i] = (a[i] >= b[i]) ? -1 : 0;
    }
    
    /* Pattern 3: Select operation using comparison
     * Another pattern that uses comparison result */
    for (int i = 0; i < N; i++) {
        select_result[i] = (a[i] >= b[i]) ? a[i] : b[i];
    }
    
    /* Use results to prevent optimization */
    volatile int check = sum + mask_result[N/4] + select_result[N/2];
    return check;
}

/* Test GT_EXPR (>) - covers the GT_EXPR case */
int test_gt_vectorize(void) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    
    for (int i = 0; i < N; i++) {
        mask_result[i] = (a[i] > b[i]) ? -1 : 0;
    }
    
    volatile int check = sum + mask_result[N/4];
    return check;
}

/* Test LT_EXPR (<) - covers the LT_EXPR case with swap */
int test_lt_vectorize(void) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            sum += a[i];
        }
    }
    
    for (int i = 0; i < N; i++) {
        mask_result[i] = (a[i] < b[i]) ? -1 : 0;
    }
    
    volatile int check = sum + mask_result[N/4];
    return check;
}

/* Test LE_EXPR (<=) - covers the LE_EXPR case with swap */
int test_le_vectorize(void) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            sum += a[i];
        }
    }
    
    for (int i = 0; i < N; i++) {
        mask_result[i] = (a[i] <= b[i]) ? -1 : 0;
    }
    
    volatile int check = sum + mask_result[N/4];
    return check;
}

/* Test with short type to explore different vectorization paths */
short test_short_ge_vectorize(void) {
    ALIGNED short sa[N];
    ALIGNED short sb[N];
    short sum = 0;
    
    for (int i = 0; i < N; i++) {
        sa[i] = (short)i;
        sb[i] = (short)(N/2);
    }
    
    for (int i = 0; i < N; i++) {
        if (sa[i] >= sb[i]) {
            sum += sa[i];
        }
    }
    
    volatile short check = sum;
    return check;
}

int main(void) {
    int result = 0;
    
    /* Initialize test data */
    init_arrays();
    
    printf("Testing vector comparison expansions in GCC...\n");
    
    /* Test all comparison operators to cover the switch cases */
    result += test_ge_vectorize();  /* Primary target: GE_EXPR */
    printf("GE test completed\n");
    
    result += test_gt_vectorize();  /* Covers GT_EXPR */
    printf("GT test completed\n");
    
    result += test_lt_vectorize();  /* Covers LT_EXPR with swap */
    printf("LT test completed\n");
    
    result += test_le_vectorize();  /* Covers LE_EXPR with swap */
    printf("LE test completed\n");
    
    /* Test with different data type */
    result += test_short_ge_vectorize();
    printf("Short GE test completed\n");
    
    /* Use result to prevent dead code elimination */
    printf("Final checksum: %d\n", result);
    
    /* Quick verification */
    printf("Sample values: a[%d]=%d, b[%d]=%d, mask[%d]=%d\n", 
           N/4, a[N/4], N/4, b[N/4], N/4, mask_result[N/4]);
    
    return 0;
}

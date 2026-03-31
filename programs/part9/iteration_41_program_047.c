/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc
 * Specifically targets the GE_EXPR, GT_EXPR, LT_EXPR, and LE_EXPR cases
 * in the switch statement at lines 12216-12233
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Force the compiler to keep computations */
static volatile int sink;

/* Test GE_EXPR (greater than or equal) */
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
        c[i] = (a[i] >= b[i]) ? -1 : 0;  /* Creates all-ones mask for true */
    }
    
    /* Use the mask in computation */
    int mask_sum = 0;
    for (int i = 0; i < N; i++) {
        mask_sum += c[i] & a[i];  /* Bitwise AND with mask */
    }
    
    return sum + mask_sum;
}

/* Test GT_EXPR (greater than) */
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
    
    int mask_sum = 0;
    for (int i = 0; i < N; i++) {
        mask_sum += c[i] & b[i];
    }
    
    return sum + mask_sum;
}

/* Test LT_EXPR (less than) */
int test_lt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            sum += b[i];
        }
    }
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? -1 : 0;
    }
    
    int mask_sum = 0;
    for (int i = 0; i < N; i++) {
        mask_sum += c[i] & a[i];
    }
    
    return sum + mask_sum;
}

/* Test LE_EXPR (less than or equal) */
int test_le_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            sum += b[i];
        }
    }
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? -1 : 0;
    }
    
    int mask_sum = 0;
    for (int i = 0; i < N; i++) {
        mask_sum += c[i] & b[i];
    }
    
    return sum + mask_sum;
}

/* Also test with short type to potentially trigger different paths */
short test_short_ge_vectorize(ALIGNED short *a, ALIGNED short *b, ALIGNED short *c) {
    short sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? -1 : 0;
    }
    
    short mask_sum = 0;
    for (int i = 0; i < N; i++) {
        mask_sum += c[i] & a[i];
    }
    
    return sum + mask_sum;
}

int main() {
    /* Allocate aligned arrays */
    ALIGNED int *a_int = (ALIGNED int*)malloc(N * sizeof(int));
    ALIGNED int *b_int = (ALIGNED int*)malloc(N * sizeof(int));
    ALIGNED int *c_int = (ALIGNED int*)malloc(N * sizeof(int));
    
    ALIGNED short *a_short = (ALIGNED short*)malloc(N * sizeof(short));
    ALIGNED short *b_short = (ALIGNED short*)malloc(N * sizeof(short));
    ALIGNED short *c_short = (ALIGNED short*)malloc(N * sizeof(short));
    
    /* Initialize with pattern that creates mix of true/false comparisons */
    for (int i = 0; i < N; i++) {
        a_int[i] = i;
        b_int[i] = N/2;  /* Half will be >=, half < */
        
        a_short[i] = (short)(i % 1000);
        b_short[i] = 500;  /* Mix of conditions */
    }
    
    /* Run all tests */
    int result_ge = test_ge_vectorize(a_int, b_int, c_int);
    int result_gt = test_gt_vectorize(a_int, b_int, c_int);
    int result_lt = test_lt_vectorize(a_int, b_int, c_int);
    int result_le = test_le_vectorize(a_int, b_int, c_int);
    short result_short = test_short_ge_vectorize(a_short, b_short, c_short);
    
    /* Force results to be used */
    sink = result_ge + result_gt + result_lt + result_le + result_short;
    
    printf("Results: GE=%d, GT=%d, LT=%d, LE=%d, short_GE=%d\n",
           result_ge, result_gt, result_lt, result_le, (int)result_short);
    printf("Sink (volatile use): %d\n", sink);
    
    /* Simple checksum to verify computation */
    int checksum = result_ge ^ result_gt ^ result_lt ^ result_le ^ result_short;
    printf("Checksum: %d\n", checksum);
    
    free(a_int);
    free(b_int);
    free(c_int);
    free(a_short);
    free(b_short);
    free(c_short);
    
    return 0;
}

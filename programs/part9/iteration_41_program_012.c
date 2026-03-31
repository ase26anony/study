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
    
    /* Pattern 1: Conditional sum with GE_EXPR - likely to trigger mask creation */
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    
    /* Pattern 2: Mask creation with GE_EXPR - direct mask generation */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? -1 : 0;  /* Creates all-ones or all-zeros mask */
    }
    
    return sum;
}

/* Function to test GT_EXPR vectorization */
int test_gt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            sum += b[i];
        }
        c[i] = (a[i] > b[i]) ? a[i] : b[i];  /* Max operation using GT_EXPR */
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
        c[i] = (a[i] < b[i]) ? -1 : 0;  /* Mask creation with LT_EXPR */
    }
    
    return sum;
}

/* Function to test LE_EXPR vectorization */
int test_le_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            sum += b[i];
        }
        c[i] = (a[i] <= b[i]) ? a[i] : b[i];  /* Min operation using LE_EXPR */
    }
    
    return sum;
}

/* Test with short type to potentially trigger different vectorization */
short test_short_ge_vectorize(ALIGNED short *a, ALIGNED short *b, ALIGNED short *c) {
    short sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
        c[i] = (a[i] >= b[i]) ? (short)-1 : (short)0;
    }
    
    return sum;
}

int main() {
    /* Aligned arrays to help vectorization */
    ALIGNED int a[N], b[N], c[N];
    ALIGNED short as[N], bs[N], cs[N];
    
    /* Initialize with pattern that creates mixed true/false conditions */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N/2;  /* Half will be >=, half < */
        c[i] = 0;
        
        as[i] = (short)i;
        bs[i] = (short)(N/4);
        cs[i] = 0;
    }
    
    /* Force volatile storage to prevent optimization */
    volatile int result_ge, result_gt, result_lt, result_le;
    volatile short result_short;
    
    /* Test all comparison operators */
    result_ge = test_ge_vectorize(a, b, c);
    result_gt = test_gt_vectorize(a, b, c);
    result_lt = test_lt_vectorize(a, b, c);
    result_le = test_le_vectorize(a, b, c);
    result_short = test_short_ge_vectorize(as, bs, cs);
    
    /* Use results to prevent dead code elimination */
    printf("Results:\n");
    printf("GE_EXPR sum: %d\n", result_ge);
    printf("GT_EXPR sum: %d\n", result_gt);
    printf("LT_EXPR sum: %d\n", result_lt);
    printf("LE_EXPR sum: %d\n", result_le);
    printf("Short GE_EXPR sum: %d\n", (int)result_short);
    
    /* Simple checksum to verify computation */
    int checksum = c[N/4] + c[N/2] + c[3*N/4];
    printf("Checksum from mask array: %d\n", checksum);
    
    return 0;
}

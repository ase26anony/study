/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc
 * Specifically targets GE_EXPR, GT_EXPR, LT_EXPR, and LE_EXPR cases
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Function to test GE_EXPR vectorization */
int test_ge_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    /* Pattern 1: Conditional sum using >= comparison */
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    
    /* Pattern 2: Create mask from >= comparison */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? -1 : 0;  /* Creates all-ones mask for true */
    }
    
    /* Use the mask in computation to prevent optimization */
    int mask_sum = 0;
    for (int i = 0; i < N; i++) {
        mask_sum += c[i] & a[i];
    }
    
    return sum + mask_sum;
}

/* Function to test GT_EXPR vectorization */
int test_gt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    /* Conditional sum using > comparison */
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    
    /* Create mask from > comparison */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] > b[i]) ? -1 : 0;
    }
    
    int mask_sum = 0;
    for (int i = 0; i < N; i++) {
        mask_sum += c[i] & b[i];
    }
    
    return sum + mask_sum;
}

/* Function to test LT_EXPR vectorization */
int test_lt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    /* Conditional sum using < comparison */
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            sum += b[i];
        }
    }
    
    /* Create mask from < comparison */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? -1 : 0;
    }
    
    int mask_sum = 0;
    for (int i = 0; i < N; i++) {
        mask_sum += c[i] & a[i];
    }
    
    return sum + mask_sum;
}

/* Function to test LE_EXPR vectorization */
int test_le_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    /* Conditional sum using <= comparison */
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            sum += a[i];
        }
    }
    
    /* Create mask from <= comparison */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? -1 : 0;
    }
    
    int mask_sum = 0;
    for (int i = 0; i < N; i++) {
        mask_sum += c[i] & b[i];
    }
    
    return sum + mask_sum;
}

/* Alternative test using short types */
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
    /* Aligned arrays to help vectorization */
    ALIGNED int a[N], b[N], c[N];
    ALIGNED short as[N], bs[N], cs[N];
    
    /* Initialize with pattern that triggers various comparison results */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N/2;  /* Half will be >=, half < */
        c[i] = 0;
        
        as[i] = (short)i;
        bs[i] = (short)(N/2);
        cs[i] = 0;
    }
    
    /* Volatile results to prevent optimization */
    volatile int result_ge = test_ge_vectorize(a, b, c);
    volatile int result_gt = test_gt_vectorize(a, b, c);
    volatile int result_lt = test_lt_vectorize(a, b, c);
    volatile int result_le = test_le_vectorize(a, b, c);
    volatile short result_short = test_short_ge_vectorize(as, bs, cs);
    
    /* Print results to ensure code isn't optimized away */
    printf("GE result: %d\n", result_ge);
    printf("GT result: %d\n", result_gt);
    printf("LT result: %d\n", result_lt);
    printf("LE result: %d\n", result_le);
    printf("Short GE result: %d\n", (int)result_short);
    
    /* Simple checksum */
    int checksum = result_ge + result_gt + result_lt + result_le + result_short;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc
 * Specifically targets the GE_EXPR case in expand_vec_cmp_expr_p */
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
    
    /* Pattern 2: Mask creation - creates all-ones or all-zeros mask */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? -1 : 0;  /* Creates mask pattern */
    }
    
    /* Use the mask in computation to prevent optimization */
    int mask_sum = 0;
    for (int i = 0; i < N; i++) {
        mask_sum += c[i] & a[i];  /* Use the mask */
    }
    
    return sum + mask_sum;
}

/* Function to test GT_EXPR vectorization */
int test_gt_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            sum += b[i];
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
    
    int mask_sum = 0;
    for (int i = 0; i < N; i++) {
        mask_sum += c[i] & a[i];
    }
    
    return sum + mask_sum;
}

/* Function to test LE_EXPR vectorization */
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

/* Alternative test using short types (different vector width) */
short test_ge_vectorize_short(ALIGNED short *a, ALIGNED short *b, ALIGNED short *c) {
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
    
    /* Initialize with pattern that creates mix of true/false comparisons */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N/2;  /* Half will be >=, half < */
        c[i] = 0;
        
        as[i] = (short)i;
        bs[i] = (short)(N/4);
        cs[i] = 0;
    }
    
    /* Force volatile storage to prevent optimization */
    volatile int result_ge = test_ge_vectorize(a, b, c);
    volatile int result_gt = test_gt_vectorize(a, b, c);
    volatile int result_lt = test_lt_vectorize(a, b, c);
    volatile int result_le = test_le_vectorize(a, b, c);
    volatile short result_ge_short = test_ge_vectorize_short(as, bs, cs);
    
    /* Print results to prevent dead code elimination */
    printf("Results: %d %d %d %d %d\n", 
           result_ge, result_gt, result_lt, result_le, result_ge_short);
    
    return 0;
}

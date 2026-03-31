/* Test program to cover vector comparison expansion in tree-vect-stmts.cc */
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
ALIGNED int c[N];
ALIGNED short sa[N];
ALIGNED short sb[N];
ALIGNED short sc[N];

/* Initialize arrays with pattern that creates mixed comparison results */
void init_arrays(void) {
    for (int i = 0; i < N; i++) {
        a[i] = i - N/2;          /* Range: [-512, 511] */
        b[i] = i % 100;          /* Range: [0, 99] */
        sa[i] = (short)(i - 200);
        sb[i] = (short)(i % 50);
    }
}

/* Test GE_EXPR (>=) - creates mask pattern likely to use BIT_NOT_EXPR + BIT_IOR_EXPR */
int test_ge_vectorize(void) {
    int sum = 0;
    
    /* Pattern 1: Conditional sum with >= comparison */
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    
    /* Pattern 2: Mask creation - this is key for triggering the specific expansion */
    for (int i = 0; i < N; i++) {
        /* Create mask: -1 if a[i] >= b[i], 0 otherwise */
        c[i] = (a[i] >= b[i]) ? -1 : 0;
    }
    
    /* Use mask in computation to prevent optimization */
    int mask_sum = 0;
    for (int i = 0; i < N; i++) {
        mask_sum += c[i] & a[i];
    }
    
    return sum + mask_sum;
}

/* Test GT_EXPR (>) - should use BIT_NOT_EXPR + BIT_AND_EXPR */
int test_gt_vectorize(void) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    
    /* Mask creation for > */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] > b[i]) ? -1 : 0;
    }
    
    int mask_sum = 0;
    for (int i = 0; i < N; i++) {
        mask_sum += c[i] & b[i];
    }
    
    return sum + mask_sum;
}

/* Test LT_EXPR (<) - should swap operands and use BIT_NOT_EXPR + BIT_AND_EXPR */
int test_lt_vectorize(void) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            sum += b[i];
        }
    }
    
    /* Mask creation for < */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] < b[i]) ? -1 : 0;
    }
    
    int mask_sum = 0;
    for (int i = 0; i < N; i++) {
        mask_sum += c[i] & a[i];
    }
    
    return sum + mask_sum;
}

/* Test LE_EXPR (<=) - should swap operands and use BIT_NOT_EXPR + BIT_IOR_EXPR */
int test_le_vectorize(void) {
    int sum = 0;
    
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            sum += a[i] + b[i];
        }
    }
    
    /* Mask creation for <= */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] <= b[i]) ? -1 : 0;
    }
    
    int mask_sum = 0;
    for (int i = 0; i < N; i++) {
        mask_sum += c[i] & (a[i] | b[i]);
    }
    
    return sum + mask_sum;
}

/* Additional test with short type to explore different vectorization paths */
int test_short_comparisons(void) {
    int sum = 0;
    
    /* Test all comparison operators with short type */
    for (int i = 0; i < N; i++) {
        if (sa[i] >= sb[i]) sum += sa[i];
        if (sa[i] > sb[i]) sum += sb[i];
        if (sa[i] < sb[i]) sum += sa[i];
        if (sa[i] <= sb[i]) sum += sb[i];
    }
    
    /* Create masks with short comparisons */
    for (int i = 0; i < N; i++) {
        sc[i] = (sa[i] >= sb[i]) ? -1 : 0;
    }
    
    short mask_sum = 0;
    for (int i = 0; i < N; i++) {
        mask_sum += sc[i] & sa[i];
    }
    
    return sum + mask_sum;
}

int main(void) {
    init_arrays();
    
    printf("Testing vector comparison expansions...\n");
    
    /* Force computation and use results to prevent optimization */
    volatile int result_ge = test_ge_vectorize();
    volatile int result_gt = test_gt_vectorize();
    volatile int result_lt = test_lt_vectorize();
    volatile int result_le = test_le_vectorize();
    volatile int result_short = test_short_comparisons();
    
    printf("GE result: %d\n", result_ge);
    printf("GT result: %d\n", result_gt);
    printf("LT result: %d\n", result_lt);
    printf("LE result: %d\n", result_le);
    printf("Short comparisons result: %d\n", result_short);
    
    /* Simple checksum to verify computation */
    int checksum = result_ge + result_gt + result_lt + result_le + result_short;
    printf("Total checksum: %d\n", checksum);
    
    return 0;
}

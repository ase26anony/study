/* Test program to trigger vector comparison expansion in tree-vect-stmts.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Function targeting GE_EXPR case (lines 12219-12222) */
int test_ge_vectorize(ALIGNED int *a, ALIGNED int *b, ALIGNED int *c) {
    int sum = 0;
    
    /* Pattern 1: Conditional sum with >= comparison */
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    
    /* Pattern 2: Mask creation with >= comparison - good candidate for bitwise expansion */
    for (int i = 0; i < N; i++) {
        /* Creates mask: -1 (all bits 1) if true, 0 if false */
        c[i] = (a[i] >= b[i]) ? -1 : 0;
    }
    
    /* Use mask in computation to prevent optimization */
    int mask_sum = 0;
    for (int i = 0; i < N; i++) {
        mask_sum += c[i] & a[i];
    }
    
    return sum + mask_sum;
}

/* Function targeting GT_EXPR case (lines 12216-12218) */
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

/* Function targeting LT_EXPR case (lines 12223-12227) */
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

/* Function targeting LE_EXPR case (lines 12228-12233) */
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
    
    int mask_sum = 0;
    for (int i = 0; i < N; i++) {
        mask_sum += c[i] & b[i];
    }
    
    return sum + mask_sum;
}

/* Test with different data types to increase coverage */
short test_ge_short_vectorize(ALIGNED short *a, ALIGNED short *b, ALIGNED short *c) {
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
        b[i] = N/2;  /* Half will be >= N/2, half < */
        c[i] = 0;
        
        as[i] = (short)i;
        bs[i] = (short)(N/4);
        cs[i] = 0;
    }
    
    /* Volatile results to prevent optimization */
    volatile int result_ge = test_ge_vectorize(a, b, c);
    volatile int result_gt = test_gt_vectorize(a, b, c);
    volatile int result_lt = test_lt_vectorize(a, b, c);
    volatile int result_le = test_le_vectorize(a, b, c);
    volatile short result_ge_short = test_ge_short_vectorize(as, bs, cs);
    
    /* Print results to ensure code executes */
    printf("Results:\n");
    printf("GE (int): %d\n", result_ge);
    printf("GT (int): %d\n", result_gt);
    printf("LT (int): %d\n", result_lt);
    printf("LE (int): %d\n", result_le);
    printf("GE (short): %d\n", (int)result_ge_short);
    
    /* Simple checksum to verify computation */
    int checksum = result_ge + result_gt + result_lt + result_le + result_ge_short;
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024

/* Function prototypes for each comparison type */
void test_gt(int *src, int *dst, int threshold);
void test_ge(int *src, int *dst, int limit);
void test_lt(int *src, int *dst, int lower_bound);
void test_le(int *src, int *dst, int upper_bound);

int main(void) {
    /* Initialize random seed */
    srand(time(NULL));
    
    /* Allocate and initialize input arrays with varying data */
    int src1[N], src2[N];
    int dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    
    /* Fill input arrays with non-uniform, non-constant data */
    for (int i = 0; i < N; i++) {
        /* Mix of positive, negative, and zero values */
        src1[i] = (i % 100) - 50;          /* Range: -50 to 49 */
        src2[i] = (i % 73) - 36;           /* Range: -36 to 36 */
    }
    
    /* Define thresholds that will trigger different comparison outcomes */
    int threshold_gt = 20;   /* For > comparison */
    int limit_ge = -10;      /* For >= comparison */
    int lower_bound_lt = 15; /* For < comparison */
    int upper_bound_le = 30; /* For <= comparison */
    
    /* Execute all four comparison patterns */
    test_gt(src1, dst_gt, threshold_gt);
    test_ge(src1, dst_ge, limit_ge);
    test_lt(src2, dst_lt, lower_bound_lt);
    test_le(src2, dst_le, upper_bound_le);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += dst_gt[i] + dst_ge[i] + dst_lt[i] + dst_le[i];
    }
    
    /* Print result to create side effect */
    printf("Final checksum: %lld\n", checksum);
    
    return 0;
}

/* GT_EXPR (> comparison) pattern */
void test_gt(int *src, int *dst, int threshold) {
    for (int i = 0; i < N; i++) {
        /* Conditional assignment using > operator */
        if (src[i] > threshold) {
            dst[i] = src[i] * 2;  /* Transform if condition true */
        } else {
            dst[i] = src[i] / 2;  /* Different transform if false */
        }
    }
}

/* GE_EXPR (>= comparison) pattern */
void test_ge(int *src, int *dst, int limit) {
    for (int i = 0; i < N; i++) {
        /* Conditional assignment using >= operator */
        if (src[i] >= limit) {
            dst[i] = src[i] + 100;
        } else {
            dst[i] = src[i] - 100;
        }
    }
}

/* LT_EXPR (< comparison) pattern */
void test_lt(int *src, int *dst, int lower_bound) {
    for (int i = 0; i < N; i++) {
        /* Conditional assignment using < operator */
        if (src[i] < lower_bound) {
            dst[i] = 0;           /* Set to zero if condition true */
        } else {
            dst[i] = src[i];      /* Keep original if false */
        }
    }
}

/* LE_EXPR (<= comparison) pattern */
void test_le(int *src, int *dst, int upper_bound) {
    for (int i = 0; i < N; i++) {
        /* Conditional assignment using <= operator */
        if (src[i] <= upper_bound) {
            dst[i] = -1;          /* Set to -1 if condition true */
        } else {
            dst[i] = 1;           /* Set to 1 if false */
        }
    }
}

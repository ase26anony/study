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
    
    /* Initialize with mixed values to ensure comparisons have varying results */
    for (int i = 0; i < N; i++) {
        src1[i] = (i % 100) - 50;          /* Values from -50 to 49 */
        src2[i] = (rand() % 200) - 100;    /* Values from -100 to 99 */
    }
    
    /* Define thresholds for each comparison */
    int gt_threshold = 25;     /* For > comparison */
    int ge_limit = -10;        /* For >= comparison */
    int lt_lower_bound = 30;   /* For < comparison */
    int le_upper_bound = -20;  /* For <= comparison */
    
    /* Execute all four comparison patterns */
    test_gt(src1, dst_gt, gt_threshold);
    test_ge(src1, dst_ge, ge_limit);
    test_lt(src2, dst_lt, lt_lower_bound);
    test_le(src2, dst_le, le_upper_bound);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += dst_gt[i] + dst_ge[i] + dst_lt[i] + dst_le[i];
    }
    
    /* Print result to ensure side effects */
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}

/* GT_EXPR (> comparison) - should trigger BIT_NOT_EXPR + BIT_AND_EXPR pattern */
void test_gt(int *src, int *dst, int threshold) {
    for (int i = 0; i < N; i++) {
        if (src[i] > threshold) {
            dst[i] = src[i] * 2;  /* Conditional assignment */
        } else {
            dst[i] = 0;
        }
    }
}

/* GE_EXPR (>= comparison) - should trigger BIT_NOT_EXPR + BIT_IOR_EXPR pattern */
void test_ge(int *src, int *dst, int limit) {
    for (int i = 0; i < N; i++) {
        if (src[i] >= limit) {
            dst[i] = src[i] + 100;  /* Different operation to avoid CSE */
        } else {
            dst[i] = -1;
        }
    }
}

/* LT_EXPR (< comparison) - should trigger BIT_NOT_EXPR + BIT_AND_EXPR with swap */
void test_lt(int *src, int *dst, int lower_bound) {
    for (int i = 0; i < N; i++) {
        if (src[i] < lower_bound) {
            dst[i] = src[i] / 2;  /* Conditional assignment */
        } else {
            dst[i] = 1;
        }
    }
}

/* LE_EXPR (<= comparison) - should trigger BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
void test_le(int *src, int *dst, int upper_bound) {
    for (int i = 0; i < N; i++) {
        if (src[i] <= upper_bound) {
            dst[i] = src[i] - 50;  /* Conditional assignment */
        } else {
            dst[i] = 100;
        }
    }
}

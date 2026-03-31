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
    /* Allocate and initialize arrays */
    int src[N];
    int dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    
    /* Initialize with varying, non-uniform data */
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        /* Mix of positive, negative, and zero values */
        src[i] = (rand() % 200) - 100;
    }
    
    /* Define thresholds for each comparison */
    int threshold = 25;      /* For GT */
    int limit = -10;         /* For GE */
    int lower_bound = 50;    /* For LT */
    int upper_bound = -30;   /* For LE */
    
    /* Execute all four vectorizable loops */
    test_gt(src, dst_gt, threshold);
    test_ge(src, dst_ge, limit);
    test_lt(src, dst_lt, lower_bound);
    test_le(src, dst_le, upper_bound);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += dst_gt[i] + dst_ge[i] + dst_lt[i] + dst_le[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    return 0;
}

/* Vectorizable loop with GT_EXPR (> comparison) */
void test_gt(int *src, int *dst, int threshold) {
    for (int i = 0; i < N; i++) {
        /* Pattern: if (src[i] > threshold) dst[i] = value */
        if (src[i] > threshold) {
            dst[i] = src[i] * 2;  /* Non-trivial assignment */
        } else {
            dst[i] = 0;
        }
    }
}

/* Vectorizable loop with GE_EXPR (>= comparison) */
void test_ge(int *src, int *dst, int limit) {
    for (int i = 0; i < N; i++) {
        /* Pattern: if (src[i] >= limit) dst[i] = src[i] */
        if (src[i] >= limit) {
            dst[i] = src[i];
        } else {
            dst[i] = -1;
        }
    }
}

/* Vectorizable loop with LT_EXPR (< comparison) */
void test_lt(int *src, int *dst, int lower_bound) {
    for (int i = 0; i < N; i++) {
        /* Pattern: if (src[i] < lower_bound) dst[i] = 0 */
        if (src[i] < lower_bound) {
            dst[i] = 0;
        } else {
            dst[i] = src[i] + 100;
        }
    }
}

/* Vectorizable loop with LE_EXPR (<= comparison) */
void test_le(int *src, int *dst, int upper_bound) {
    for (int i = 0; i < N; i++) {
        /* Pattern: if (src[i] <= upper_bound) dst[i] = -1 */
        if (src[i] <= upper_bound) {
            dst[i] = -1;
        } else {
            dst[i] = src[i] / 2;
        }
    }
}

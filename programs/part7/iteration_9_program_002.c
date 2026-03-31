#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024

/* Function prototypes for each comparison type */
void vector_gt(int *src, int *dst, int threshold);
void vector_ge(int *src, int *dst, int limit);
void vector_lt(int *src, int *dst, int lower_bound);
void vector_le(int *src, int *dst, int upper_bound);

int main(void) {
    /* Initialize random seed */
    srand(time(NULL));
    
    /* Allocate and initialize input arrays with varying data */
    int src1[N], src2[N];
    for (int i = 0; i < N; i++) {
        /* Mix of positive, negative, and zero values */
        src1[i] = (i % 100) - 50;          /* Range: -50 to 49 */
        src2[i] = (rand() % 200) - 100;    /* Range: -100 to 99 */
    }
    
    /* Allocate output arrays for each comparison type */
    int dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    
    /* Initialize output arrays to known non-zero values */
    for (int i = 0; i < N; i++) {
        dst_gt[i] = 999;
        dst_ge[i] = 999;
        dst_lt[i] = 999;
        dst_le[i] = 999;
    }
    
    /* Define thresholds for each comparison */
    int gt_threshold = 25;     /* For GT_EXPR: a[i] > 25 */
    int ge_limit = -10;        /* For GE_EXPR: a[i] >= -10 */
    int lt_lower_bound = 30;   /* For LT_EXPR: a[i] < 30 */
    int le_upper_bound = -20;  /* For LE_EXPR: a[i] <= -20 */
    
    /* Execute each vectorizable comparison loop */
    vector_gt(src1, dst_gt, gt_threshold);
    vector_ge(src1, dst_ge, ge_limit);
    vector_lt(src2, dst_lt, lt_lower_bound);
    vector_le(src2, dst_le, le_upper_bound);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += dst_gt[i] + dst_ge[i] + dst_lt[i] + dst_le[i];
    }
    
    /* Print result to ensure side effects */
    printf("Final checksum: %lld\n", checksum);
    
    return 0;
}

/* GT_EXPR transformation target: if (src[i] > threshold) dst[i] = src[i] */
void vector_gt(int *src, int *dst, int threshold) {
    for (int i = 0; i < N; i++) {
        if (src[i] > threshold) {
            dst[i] = src[i];
        } else {
            dst[i] = 0;
        }
    }
}

/* GE_EXPR transformation target: if (src[i] >= limit) dst[i] = src[i] * 2 */
void vector_ge(int *src, int *dst, int limit) {
    for (int i = 0; i < N; i++) {
        if (src[i] >= limit) {
            dst[i] = src[i] * 2;
        } else {
            dst[i] = -1;
        }
    }
}

/* LT_EXPR transformation target: if (src[i] < lower_bound) dst[i] = 0 */
void vector_lt(int *src, int *dst, int lower_bound) {
    for (int i = 0; i < N; i++) {
        if (src[i] < lower_bound) {
            dst[i] = 0;
        } else {
            dst[i] = src[i];
        }
    }
}

/* LE_EXPR transformation target: if (src[i] <= upper_bound) dst[i] = -1 */
void vector_le(int *src, int *dst, int upper_bound) {
    for (int i = 0; i < N; i++) {
        if (src[i] <= upper_bound) {
            dst[i] = -1;
        } else {
            dst[i] = src[i] * 3;
        }
    }
}

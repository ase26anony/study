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
        src1[i] = (i % 200) - 100;        /* Range: -100 to 99 */
        src2[i] = (rand() % 256) - 128;   /* Range: -128 to 127 */
    }
    
    /* Allocate output arrays */
    int dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    
    /* Initialize thresholds with different values */
    int threshold_gt = 25;    /* For GT_EXPR */
    int limit_ge = -50;       /* For GE_EXPR */
    int lower_bound_lt = 75;  /* For LT_EXPR */
    int upper_bound_le = -25; /* For LE_EXPR */
    
    /* Execute all four vectorizable loops */
    vector_gt(src1, dst_gt, threshold_gt);
    vector_ge(src1, dst_ge, limit_ge);
    vector_lt(src2, dst_lt, lower_bound_lt);
    vector_le(src2, dst_le, upper_bound_le);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += dst_gt[i] + dst_ge[i] + dst_lt[i] + dst_le[i];
    }
    
    /* Print result to ensure side effects */
    printf("Final checksum: %lld\n", checksum);
    
    return 0;
}

/* GT_EXPR transformation target: if (src[i] > threshold) dst[i] = value */
void vector_gt(int *src, int *dst, int threshold) {
    for (int i = 0; i < N; i++) {
        /* This should trigger GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR */
        if (src[i] > threshold) {
            dst[i] = src[i] * 2;  /* Non-trivial assignment */
        } else {
            dst[i] = -1;          /* Different value for false case */
        }
    }
}

/* GE_EXPR transformation target: if (src[i] >= limit) dst[i] = value */
void vector_ge(int *src, int *dst, int limit) {
    for (int i = 0; i < N; i++) {
        /* This should trigger GE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR */
        if (src[i] >= limit) {
            dst[i] = src[i] + 100;
        } else {
            dst[i] = 0;
        }
    }
}

/* LT_EXPR transformation target: if (src[i] < lower_bound) dst[i] = value */
void vector_lt(int *src, int *dst, int lower_bound) {
    for (int i = 0; i < N; i++) {
        /* This should trigger LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR with swap */
        if (src[i] < lower_bound) {
            dst[i] = src[i] / 2;
        } else {
            dst[i] = src[i];
        }
    }
}

/* LE_EXPR transformation target: if (src[i] <= upper_bound) dst[i] = value */
void vector_le(int *src, int *dst, int upper_bound) {
    for (int i = 0; i < N; i++) {
        /* This should trigger LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
        if (src[i] <= upper_bound) {
            dst[i] = upper_bound - src[i];
        } else {
            dst[i] = src[i];
        }
    }
}

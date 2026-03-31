#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024

/* Function prototypes for each comparison type */
void vector_gt(int *src, int *dst, int threshold);
void vector_ge(int *src, int *dst, int limit);
void vector_lt(int *src, int *dst, int lower_bound);
void vector_le(int *src, int *dst, int upper_bound);

int main() {
    /* Allocate and initialize arrays */
    int src[N], dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    int i, sum = 0;
    
    /* Seed random number generator for predictable but non-constant data */
    srand(42);
    
    /* Initialize source array with values in range [-100, 100] */
    for (i = 0; i < N; i++) {
        src[i] = (rand() % 201) - 100;  /* Values from -100 to 100 */
    }
    
    /* Define thresholds for each comparison */
    int threshold_gt = 50;    /* For > comparison */
    int limit_ge = 25;        /* For >= comparison */
    int lower_bound_lt = -25; /* For < comparison */
    int upper_bound_le = 75;  /* For <= comparison */
    
    /* Execute each vectorizable comparison loop */
    vector_gt(src, dst_gt, threshold_gt);
    vector_ge(src, dst_ge, limit_ge);
    vector_lt(src, dst_lt, lower_bound_lt);
    vector_le(src, dst_le, upper_bound_le);
    
    /* Compute checksum to prevent dead code elimination */
    for (i = 0; i < N; i++) {
        sum += dst_gt[i] + dst_ge[i] + dst_lt[i] + dst_le[i];
    }
    
    /* Print result to ensure side effects */
    printf("Checksum: %d\n", sum);
    
    return 0;
}

/* GT_EXPR (> comparison) transformation target */
void vector_gt(int *src, int *dst, int threshold) {
    int i;
    for (i = 0; i < N; i++) {
        /* This should trigger GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR */
        if (src[i] > threshold) {
            dst[i] = src[i] * 2;  /* Non-trivial assignment */
        } else {
            dst[i] = 0;
        }
    }
}

/* GE_EXPR (>= comparison) transformation target */
void vector_ge(int *src, int *dst, int limit) {
    int i;
    for (i = 0; i < N; i++) {
        /* This should trigger GE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR */
        if (src[i] >= limit) {
            dst[i] = src[i] + 100;  /* Different operation to avoid pattern merging */
        } else {
            dst[i] = -1;
        }
    }
}

/* LT_EXPR (< comparison) transformation target */
void vector_lt(int *src, int *dst, int lower_bound) {
    int i;
    for (i = 0; i < N; i++) {
        /* This should trigger LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR with swap */
        if (src[i] < lower_bound) {
            dst[i] = src[i] / 2;  /* Different operation */
        } else {
            dst[i] = 1;
        }
    }
}

/* LE_EXPR (<= comparison) transformation target */
void vector_le(int *src, int *dst, int upper_bound) {
    int i;
    for (i = 0; i < N; i++) {
        /* This should trigger LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
        if (src[i] <= upper_bound) {
            dst[i] = src[i] - 50;  /* Different operation */
        } else {
            dst[i] = 1000;
        }
    }
}

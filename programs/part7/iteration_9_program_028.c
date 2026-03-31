#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024

/* Function prototypes for each comparison type */
void vector_gt(int *src, int *dst, int threshold);
void vector_ge(int *src, int *dst, int limit);
void vector_lt(int *src, int *dst, int lower_bound);
void vector_le(int *src, int *dst, int upper_bound);

/* Initialize array with non-uniform, non-constant pattern */
void init_array(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        /* Mix of positive, negative, and zero values */
        arr[i] = (i * 73) % 197 - 98;  /* Range: -98 to 98 */
    }
}

/* GT_EXPR transformation: if (src[i] > threshold) dst[i] = src[i] */
void vector_gt(int *src, int *dst, int threshold) {
    for (int i = 0; i < N; i++) {
        if (src[i] > threshold) {
            dst[i] = src[i];
        } else {
            dst[i] = 0;
        }
    }
}

/* GE_EXPR transformation: if (src[i] >= limit) dst[i] = src[i] * 2 */
void vector_ge(int *src, int *dst, int limit) {
    for (int i = 0; i < N; i++) {
        if (src[i] >= limit) {
            dst[i] = src[i] * 2;
        } else {
            dst[i] = src[i];
        }
    }
}

/* LT_EXPR transformation: if (src[i] < lower_bound) dst[i] = -src[i] */
void vector_lt(int *src, int *dst, int lower_bound) {
    for (int i = 0; i < N; i++) {
        if (src[i] < lower_bound) {
            dst[i] = -src[i];
        } else {
            dst[i] = src[i];
        }
    }
}

/* LE_EXPR transformation: if (src[i] <= upper_bound) dst[i] = src[i] + 1 */
void vector_le(int *src, int *dst, int upper_bound) {
    for (int i = 0; i < N; i++) {
        if (src[i] <= upper_bound) {
            dst[i] = src[i] + 1;
        } else {
            dst[i] = src[i] - 1;
        }
    }
}

int main() {
    /* Allocate arrays */
    int src[N];
    int dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    
    /* Initialize source array with non-uniform data */
    init_array(src, N);
    
    /* Set thresholds that will trigger different comparison outcomes */
    int threshold_gt = 25;      /* Some elements > 25, some not */
    int limit_ge = -50;         /* Some elements >= -50, some not */
    int lower_bound_lt = -75;   /* Some elements < -75, some not */
    int upper_bound_le = 50;    /* Some elements <= 50, some not */
    
    /* Execute all four vectorizable loops */
    vector_gt(src, dst_gt, threshold_gt);
    vector_ge(src, dst_ge, limit_ge);
    vector_lt(src, dst_lt, lower_bound_lt);
    vector_le(src, dst_le, upper_bound_le);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += dst_gt[i];
        checksum += dst_ge[i];
        checksum += dst_lt[i];
        checksum += dst_le[i];
    }
    
    /* Print result to ensure side effects */
    printf("Final checksum: %lld\n", checksum);
    
    return 0;
}

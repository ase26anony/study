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
        arr[i] = (i * 73) % 257 - 128;  /* Pattern ensures varied values */
    }
}

/* GT_EXPR transformation target: if (src[i] > threshold) dst[i] = src[i] * 2 */
void vector_gt(int *src, int *dst, int threshold) {
    for (int i = 0; i < N; i++) {
        if (src[i] > threshold) {
            dst[i] = src[i] * 2;
        } else {
            dst[i] = src[i];
        }
    }
}

/* GE_EXPR transformation target: if (src[i] >= limit) dst[i] = src[i] + limit */
void vector_ge(int *src, int *dst, int limit) {
    for (int i = 0; i < N; i++) {
        if (src[i] >= limit) {
            dst[i] = src[i] + limit;
        } else {
            dst[i] = src[i] - limit;
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
            dst[i] = 1;
        }
    }
}

int main() {
    /* Allocate arrays */
    int src[N];
    int dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    
    /* Initialize source array with non-uniform data */
    init_array(src, N);
    
    /* Set thresholds that will trigger varied comparison outcomes */
    int threshold = 50;      /* For GT: some values > 50, some <= 50 */
    int limit = -30;         /* For GE: some values >= -30, some < -30 */
    int lower_bound = 25;    /* For LT: some values < 25, some >= 25 */
    int upper_bound = -40;   /* For LE: some values <= -40, some > -40 */
    
    /* Execute all four vectorizable loops */
    vector_gt(src, dst_gt, threshold);
    vector_ge(src, dst_ge, limit);
    vector_lt(src, dst_lt, lower_bound);
    vector_le(src, dst_le, upper_bound);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += dst_gt[i] + dst_ge[i] + dst_lt[i] + dst_le[i];
    }
    
    /* Print result to ensure side effects */
    printf("Checksum: %lld\n", checksum);
    
    /* Additional verification: count how many elements satisfied each condition */
    int count_gt = 0, count_ge = 0, count_lt = 0, count_le = 0;
    for (int i = 0; i < N; i++) {
        count_gt += (src[i] > threshold);
        count_ge += (src[i] >= limit);
        count_lt += (src[i] < lower_bound);
        count_le += (src[i] <= upper_bound);
    }
    printf("GT: %d, GE: %d, LT: %d, LE: %d\n", count_gt, count_ge, count_lt, count_le);
    
    return 0;
}

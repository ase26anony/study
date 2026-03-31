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
    int src[N];
    int dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    
    /* Initialize with varying data to ensure comparisons have mixed results */
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        src[i] = (rand() % 200) - 100;  /* Values between -100 and 99 */
    }
    
    /* Define thresholds that will trigger different comparison outcomes */
    int threshold = 25;      /* For GT: some values > 25 */
    int limit = -10;         /* For GE: some values >= -10 */
    int lower_bound = 50;    /* For LT: some values < 50 */
    int upper_bound = -25;   /* For LE: some values <= -25 */
    
    /* Execute all four comparison patterns */
    vector_gt(src, dst_gt, threshold);
    vector_ge(src, dst_ge, limit);
    vector_lt(src, dst_lt, lower_bound);
    vector_le(src, dst_le, upper_bound);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += dst_gt[i] + dst_ge[i] + dst_lt[i] + dst_le[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    return 0;
}

/* Greater-than comparison transformation target */
void vector_gt(int *src, int *dst, int threshold) {
    for (int i = 0; i < N; i++) {
        /* This should trigger GT_EXPR -> BIT_NOT_EXPR, BIT_AND_EXPR transformation */
        if (src[i] > threshold) {
            dst[i] = src[i] * 2;  /* Non-trivial assignment to avoid optimization */
        } else {
            dst[i] = 0;
        }
    }
}

/* Greater-or-equal comparison transformation target */
void vector_ge(int *src, int *dst, int limit) {
    for (int i = 0; i < N; i++) {
        /* This should trigger GE_EXPR -> BIT_NOT_EXPR, BIT_IOR_EXPR transformation */
        if (src[i] >= limit) {
            dst[i] = src[i] + 100;
        } else {
            dst[i] = -1;
        }
    }
}

/* Less-than comparison transformation target */
void vector_lt(int *src, int *dst, int lower_bound) {
    for (int i = 0; i < N; i++) {
        /* This should trigger LT_EXPR -> BIT_NOT_EXPR, BIT_AND_EXPR with swap */
        if (src[i] < lower_bound) {
            dst[i] = src[i] / 2;  /* Different operation to avoid pattern merging */
        } else {
            dst[i] = 999;
        }
    }
}

/* Less-or-equal comparison transformation target */
void vector_le(int *src, int *dst, int upper_bound) {
    for (int i = 0; i < N; i++) {
        /* This should trigger LE_EXPR -> BIT_NOT_EXPR, BIT_IOR_EXPR with swap */
        if (src[i] <= upper_bound) {
            dst[i] = src[i] - 50;
        } else {
            dst[i] = 777;
        }
    }
}

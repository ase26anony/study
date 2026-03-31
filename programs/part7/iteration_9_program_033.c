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
    int i;
    int src[N];
    int dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    
    /* Initialize with varying data to ensure comparisons have mixed results */
    srand(time(NULL));
    for (i = 0; i < N; i++) {
        src[i] = (rand() % 200) - 100;  /* Values between -100 and 99 */
    }
    
    /* Initialize output arrays */
    for (i = 0; i < N; i++) {
        dst_gt[i] = 0;
        dst_ge[i] = 0;
        dst_lt[i] = 0;
        dst_le[i] = 0;
    }
    
    /* Test all four comparison operators with different thresholds */
    vector_gt(src, dst_gt, 25);      /* GT_EXPR: > 25 */
    vector_ge(src, dst_ge, -50);     /* GE_EXPR: >= -50 */
    vector_lt(src, dst_lt, 75);      /* LT_EXPR: < 75 */
    vector_le(src, dst_le, -25);     /* LE_EXPR: <= -25 */
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (i = 0; i < N; i++) {
        checksum += dst_gt[i] + dst_ge[i] + dst_lt[i] + dst_le[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    return 0;
}

/* GT_EXPR transformation target: if (src[i] > threshold) dst[i] = src[i] */
void vector_gt(int *src, int *dst, int threshold) {
    int i;
    for (i = 0; i < N; i++) {
        if (src[i] > threshold) {
            dst[i] = src[i];
        } else {
            dst[i] = 0;
        }
    }
}

/* GE_EXPR transformation target: if (src[i] >= limit) dst[i] = src[i] + 1 */
void vector_ge(int *src, int *dst, int limit) {
    int i;
    for (i = 0; i < N; i++) {
        if (src[i] >= limit) {
            dst[i] = src[i] + 1;
        } else {
            dst[i] = -1;
        }
    }
}

/* LT_EXPR transformation target: if (src[i] < lower_bound) dst[i] = src[i] * 2 */
void vector_lt(int *src, int *dst, int lower_bound) {
    int i;
    for (i = 0; i < N; i++) {
        if (src[i] < lower_bound) {
            dst[i] = src[i] * 2;
        } else {
            dst[i] = src[i];
        }
    }
}

/* LE_EXPR transformation target: if (src[i] <= upper_bound) dst[i] = -src[i] */
void vector_le(int *src, int *dst, int upper_bound) {
    int i;
    for (i = 0; i < N; i++) {
        if (src[i] <= upper_bound) {
            dst[i] = -src[i];
        } else {
            dst[i] = 0;
        }
    }
}

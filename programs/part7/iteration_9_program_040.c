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
    int threshold = 25;      /* For GT: src[i] > 25 */
    int limit = -10;         /* For GE: src[i] >= -10 */
    int lower_bound = 50;    /* For LT: src[i] < 50 */
    int upper_bound = -25;   /* For LE: src[i] <= -25 */
    
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
    
    printf("Checksum: %lld\n", checksum);
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

/* LT_EXPR transformation target: if (src[i] < lower_bound) dst[i] = src[i] + 100 */
void vector_lt(int *src, int *dst, int lower_bound) {
    for (int i = 0; i < N; i++) {
        if (src[i] < lower_bound) {
            dst[i] = src[i] + 100;
        } else {
            dst[i] = src[i];
        }
    }
}

/* LE_EXPR transformation target: if (src[i] <= upper_bound) dst[i] = -src[i] */
void vector_le(int *src, int *dst, int upper_bound) {
    for (int i = 0; i < N; i++) {
        if (src[i] <= upper_bound) {
            dst[i] = -src[i];
        } else {
            dst[i] = 0;
        }
    }
}

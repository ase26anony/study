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
    /* Allocate and initialize input arrays with non-uniform data */
    int src1[N], src2[N];
    int dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    
    /* Initialize with varying values to ensure different comparison outcomes */
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        src1[i] = (i % 100) - 50;          /* Range: -50 to 49 */
        src2[i] = rand() % 200 - 100;      /* Range: -100 to 99 */
    }
    
    /* Define thresholds for each comparison */
    int threshold = 25;     /* For GT_EXPR */
    int limit = -10;        /* For GE_EXPR */
    int lower_bound = 0;    /* For LT_EXPR */
    int upper_bound = 40;   /* For LE_EXPR */
    
    /* Execute all four comparison patterns */
    test_gt(src1, dst_gt, threshold);
    test_ge(src1, dst_ge, limit);
    test_lt(src2, dst_lt, lower_bound);
    test_le(src2, dst_le, upper_bound);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += dst_gt[i] + dst_ge[i] + dst_lt[i] + dst_le[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    return 0;
}

/* GT_EXPR pattern: if (src[i] > threshold) dst[i] = value */
void test_gt(int *src, int *dst, int threshold) {
    for (int i = 0; i < N; i++) {
        if (src[i] > threshold) {
            dst[i] = src[i] * 2;  /* Non-trivial assignment */
        } else {
            dst[i] = 0;
        }
    }
}

/* GE_EXPR pattern: if (src[i] >= limit) dst[i] = src[i] */
void test_ge(int *src, int *dst, int limit) {
    for (int i = 0; i < N; i++) {
        if (src[i] >= limit) {
            dst[i] = src[i];      /* Direct assignment */
        } else {
            dst[i] = -1;
        }
    }
}

/* LT_EXPR pattern: if (src[i] < lower_bound) dst[i] = 0 */
void test_lt(int *src, int *dst, int lower_bound) {
    for (int i = 0; i < N; i++) {
        if (src[i] < lower_bound) {
            dst[i] = 0;
        } else {
            dst[i] = src[i] + 100;  /* Different computation */
        }
    }
}

/* LE_EXPR pattern: if (src[i] <= upper_bound) dst[i] = -1 */
void test_le(int *src, int *dst, int upper_bound) {
    for (int i = 0; i < N; i++) {
        if (src[i] <= upper_bound) {
            dst[i] = -1;
        } else {
            dst[i] = src[i] - 50;  /* Another computation */
        }
    }
}

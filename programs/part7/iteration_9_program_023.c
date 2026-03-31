#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024

/* GT_EXPR transformation test */
void test_gt(int *src, int *dst, int threshold) {
    for (int i = 0; i < N; i++) {
        if (src[i] > threshold) {
            dst[i] = src[i] * 2;
        } else {
            dst[i] = src[i];
        }
    }
}

/* GE_EXPR transformation test */
void test_ge(int *src, int *dst, int limit) {
    for (int i = 0; i < N; i++) {
        if (src[i] >= limit) {
            dst[i] = src[i] + 100;
        } else {
            dst[i] = src[i] - 50;
        }
    }
}

/* LT_EXPR transformation test */
void test_lt(int *src, int *dst, int lower_bound) {
    for (int i = 0; i < N; i++) {
        if (src[i] < lower_bound) {
            dst[i] = 0;
        } else {
            dst[i] = src[i];
        }
    }
}

/* LE_EXPR transformation test */
void test_le(int *src, int *dst, int upper_bound) {
    for (int i = 0; i < N; i++) {
        if (src[i] <= upper_bound) {
            dst[i] = -1;
        } else {
            dst[i] = 1;
        }
    }
}

/* Additional floating-point tests to cover more cases */
void test_gt_float(float *src, float *dst, float threshold) {
    for (int i = 0; i < N; i++) {
        if (src[i] > threshold) {
            dst[i] = src[i] * 2.0f;
        } else {
            dst[i] = src[i];
        }
    }
}

void test_le_float(float *src, float *dst, float upper_bound) {
    for (int i = 0; i < N; i++) {
        if (src[i] <= upper_bound) {
            dst[i] = src[i] * 0.5f;
        } else {
            dst[i] = src[i];
        }
    }
}

int main() {
    /* Initialize with predictable but non-constant data */
    int src_int[N];
    float src_float[N];
    int dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    float dst_gt_float[N], dst_le_float[N];
    
    srand(time(NULL));
    
    /* Create varying data patterns */
    for (int i = 0; i < N; i++) {
        src_int[i] = (i % 200) - 100;  /* Range: -100 to 99 */
        src_float[i] = (float)((i % 150) - 75) * 0.7f;  /* Range: -52.5 to 52.5 */
    }
    
    /* Test all four comparison operators with integer data */
    test_gt(src_int, dst_gt, 25);      /* GT_EXPR: > */
    test_ge(src_int, dst_ge, -50);     /* GE_EXPR: >= */
    test_lt(src_int, dst_lt, 0);       /* LT_EXPR: < */
    test_le(src_int, dst_le, 50);      /* LE_EXPR: <= */
    
    /* Test with floating-point data */
    test_gt_float(src_float, dst_gt_float, 10.0f);    /* GT_EXPR with floats */
    test_le_float(src_float, dst_le_float, -20.0f);   /* LE_EXPR with floats */
    
    /* Prevent dead code elimination by computing checksums */
    long long sum_int = 0;
    float sum_float = 0.0f;
    
    for (int i = 0; i < N; i++) {
        sum_int += dst_gt[i] + dst_ge[i] + dst_lt[i] + dst_le[i];
        sum_float += dst_gt_float[i] + dst_le_float[i];
    }
    
    /* Additional loop with mixed comparisons to increase coverage */
    int mixed_dst[N];
    for (int i = 0; i < N; i++) {
        /* Use both GT and LE in same loop */
        if (src_int[i] > 30) {
            mixed_dst[i] = src_int[i] * 3;
        } else if (src_int[i] <= -30) {
            mixed_dst[i] = src_int[i] / 2;
        } else {
            mixed_dst[i] = src_int[i];
        }
        sum_int += mixed_dst[i];
    }
    
    printf("Integer checksum: %lld\n", sum_int);
    printf("Float checksum: %f\n", sum_float);
    
    return 0;
}

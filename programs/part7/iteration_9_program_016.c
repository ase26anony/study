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

/* Floating point versions to ensure coverage with FP comparisons */
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
            dst[i] = -src[i];
        } else {
            dst[i] = src[i];
        }
    }
}

int main() {
    /* Initialize with non-constant but predictable data */
    int src_int[N];
    float src_float[N];
    int dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    float dst_gt_f[N], dst_le_f[N];
    
    srand(time(NULL));
    
    /* Create varying data patterns */
    for (int i = 0; i < N; i++) {
        src_int[i] = (i % 200) - 100;  /* Range: -100 to 99 */
        src_float[i] = (float)(i % 150) / 10.0f - 7.5f;  /* Range: -7.5 to 7.4 */
    }
    
    /* Test all four integer comparison operators */
    test_gt(src_int, dst_gt, 25);      /* GT_EXPR: > */
    test_ge(src_int, dst_ge, -50);     /* GE_EXPR: >= */
    test_lt(src_int, dst_lt, 0);       /* LT_EXPR: < */
    test_le(src_int, dst_le, 75);      /* LE_EXPR: <= */
    
    /* Test floating point comparisons */
    test_gt_float(src_float, dst_gt_f, 0.0f);    /* GT_EXPR with float */
    test_le_float(src_float, dst_le_f, 3.0f);    /* LE_EXPR with float */
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += dst_gt[i] + dst_ge[i] + dst_lt[i] + dst_le[i];
        checksum += (long long)(dst_gt_f[i] * 100) + (long long)(dst_le_f[i] * 100);
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    return 0;
}

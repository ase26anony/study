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
    
    /* Create mixed data that will trigger different comparison outcomes */
    for (int i = 0; i < N; i++) {
        src_int[i] = (i % 200) - 100;  /* Values from -100 to 99 */
        src_float[i] = (float)((i % 150) - 75) * 0.5f;  /* Values from -37.5 to 37.0 */
    }
    
    /* Test all four integer comparison operators */
    test_gt(src_int, dst_gt, 25);      /* GT_EXPR: > */
    test_ge(src_int, dst_ge, -50);     /* GE_EXPR: >= */
    test_lt(src_int, dst_lt, 0);       /* LT_EXPR: < */
    test_le(src_int, dst_le, 50);      /* LE_EXPR: <= */
    
    /* Test floating point comparisons */
    test_gt_float(src_float, dst_gt_f, 10.0f);    /* GT_EXPR with floats */
    test_le_float(src_float, dst_le_f, -5.0f);    /* LE_EXPR with floats */
    
    /* Compute checksums to prevent dead code elimination */
    long long sum = 0;
    float fsum = 0.0f;
    
    for (int i = 0; i < N; i++) {
        sum += dst_gt[i] + dst_ge[i] + dst_lt[i] + dst_le[i];
        fsum += dst_gt_f[i] + dst_le_f[i];
    }
    
    /* Additional loops with different data types for broader coverage */
    short src_short[N];
    short dst_short[N];
    
    for (int i = 0; i < N; i++) {
        src_short[i] = (short)((i % 256) - 128);
    }
    
    /* LT_EXPR with short type */
    for (int i = 0; i < N; i++) {
        if (src_short[i] < -64) {
            dst_short[i] = src_short[i] * 3;
        } else {
            dst_short[i] = src_short[i];
        }
        sum += dst_short[i];
    }
    
    /* GE_EXPR with unsigned char */
    unsigned char src_uchar[N];
    unsigned char dst_uchar[N];
    
    for (int i = 0; i < N; i++) {
        src_uchar[i] = (unsigned char)(i % 256);
    }
    
    for (int i = 0; i < N; i++) {
        if (src_uchar[i] >= 128) {
            dst_uchar[i] = src_uchar[i] / 2;
        } else {
            dst_uchar[i] = src_uchar[i];
        }
        sum += dst_uchar[i];
    }
    
    printf("Integer checksum: %lld\n", sum);
    printf("Float checksum: %f\n", fsum);
    
    return 0;
}

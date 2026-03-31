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

/* Additional test with float comparisons to ensure coverage */
void test_float_comparisons(float *fsrc, float *fdst, float fthreshold) {
    /* GT_EXPR with floats */
    for (int i = 0; i < N; i++) {
        if (fsrc[i] > fthreshold) {
            fdst[i] = fsrc[i] * 2.0f;
        } else {
            fdst[i] = fsrc[i];
        }
    }
    
    /* LE_EXPR with floats */
    for (int i = 0; i < N; i++) {
        if (fsrc[i] <= fthreshold) {
            fdst[i + N] = -fsrc[i];
        } else {
            fdst[i + N] = fsrc[i];
        }
    }
}

int main() {
    int src[N], dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    float fsrc[N], fdst[2 * N];
    
    /* Initialize with non-uniform, non-constant data */
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        /* Mix of positive and negative values to ensure varied comparisons */
        src[i] = (i % 200) - 100;  /* Range: -100 to 99 */
        fsrc[i] = (float)((i % 150) - 75) * 1.5f;  /* Range: -112.5 to 112.5 */
    }
    
    /* Test all four comparison operators with different thresholds */
    test_gt(src, dst_gt, 25);      /* GT_EXPR: > 25 */
    test_ge(src, dst_ge, -50);     /* GE_EXPR: >= -50 */
    test_lt(src, dst_lt, 0);       /* LT_EXPR: < 0 */
    test_le(src, dst_le, 50);      /* LE_EXPR: <= 50 */
    
    /* Test floating point comparisons */
    test_float_comparisons(fsrc, fdst, 25.0f);
    
    /* Compute checksums to prevent dead code elimination */
    long long sum = 0;
    for (int i = 0; i < N; i++) {
        sum += dst_gt[i] + dst_ge[i] + dst_lt[i] + dst_le[i];
    }
    
    /* Also sum float results */
    float fsum = 0.0f;
    for (int i = 0; i < 2 * N; i++) {
        fsum += fdst[i];
    }
    
    /* Print results to ensure side effects */
    printf("Integer checksum: %lld\n", sum);
    printf("Float checksum: %f\n", fsum);
    
    return 0;
}

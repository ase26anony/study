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
            fdst[i + N] = -1.0f;
        } else {
            fdst[i + N] = 1.0f;
        }
    }
}

int main() {
    int src_int[N];
    int dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    float src_float[N];
    float dst_float[N * 2];
    
    /* Initialize with non-uniform, non-constant data */
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        /* Create varied data pattern that will trigger different comparison outcomes */
        src_int[i] = (i % 200) - 100;  /* Values from -100 to 99 */
        src_float[i] = (float)((i % 150) - 75) * 0.5f;  /* Values from -37.5 to 37.0 */
    }
    
    /* Test all four comparison operators with different thresholds */
    test_gt(src_int, dst_gt, 25);      /* GT_EXPR: > 25 */
    test_ge(src_int, dst_ge, -50);     /* GE_EXPR: >= -50 */
    test_lt(src_int, dst_lt, 0);       /* LT_EXPR: < 0 */
    test_le(src_int, dst_le, 50);      /* LE_EXPR: <= 50 */
    
    /* Test float comparisons */
    test_float_comparisons(src_float, dst_float, 10.0f);
    
    /* Compute checksums to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += dst_gt[i] + dst_ge[i] + dst_lt[i] + dst_le[i];
    }
    
    /* Add float results to checksum (converted to int) */
    for (int i = 0; i < N * 2; i++) {
        checksum += (long long)dst_float[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    /* Additional test with short types to trigger different vectorization paths */
    {
        short src_short[N];
        short dst_short[N];
        
        for (int i = 0; i < N; i++) {
            src_short[i] = (short)((i % 128) - 64);
        }
        
        /* LT_EXPR with short */
        for (int i = 0; i < N; i++) {
            if (src_short[i] < (short)-10) {
                dst_short[i] = src_short[i] * 2;
            } else {
                dst_short[i] = src_short[i];
            }
        }
        
        /* Add to checksum */
        for (int i = 0; i < N; i++) {
            checksum += dst_short[i];
        }
        
        printf("Final checksum with short: %lld\n", checksum);
    }
    
    return 0;
}

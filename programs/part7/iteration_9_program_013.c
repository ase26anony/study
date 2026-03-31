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

/* Mixed floating-point comparisons to ensure different data types are tested */
void test_float_gt(float *src, float *dst, float threshold) {
    for (int i = 0; i < N; i++) {
        if (src[i] > threshold) {
            dst[i] = src[i] * 1.5f;
        } else {
            dst[i] = src[i];
        }
    }
}

void test_float_le(float *src, float *dst, float upper_bound) {
    for (int i = 0; i < N; i++) {
        if (src[i] <= upper_bound) {
            dst[i] = src[i] * 0.5f;
        } else {
            dst[i] = src[i] * 2.0f;
        }
    }
}

int main() {
    /* Initialize with non-constant but predictable data */
    int src_int[N];
    float src_float[N];
    int dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    float dst_float_gt[N], dst_float_le[N];
    
    srand(time(NULL));
    
    /* Create varying data patterns to ensure comparisons have mixed results */
    for (int i = 0; i < N; i++) {
        src_int[i] = (i % 200) - 100;  /* Range: -100 to 99 */
        src_float[i] = (float)((i % 150) - 75) * 0.7f;  /* Range: -52.5 to 52.5 */
    }
    
    /* Test all four integer comparison operators */
    test_gt(src_int, dst_gt, 25);      /* GT_EXPR: > */
    test_ge(src_int, dst_ge, -50);     /* GE_EXPR: >= */
    test_lt(src_int, dst_lt, 0);       /* LT_EXPR: < */
    test_le(src_int, dst_le, 50);      /* LE_EXPR: <= */
    
    /* Test floating-point comparisons */
    test_float_gt(src_float, dst_float_gt, 10.0f);   /* GT_EXPR with floats */
    test_float_le(src_float, dst_float_le, -10.0f);  /* LE_EXPR with floats */
    
    /* Compute checksums to prevent dead code elimination */
    long long sum_int = 0;
    float sum_float = 0.0f;
    
    for (int i = 0; i < N; i++) {
        sum_int += dst_gt[i] + dst_ge[i] + dst_lt[i] + dst_le[i];
        sum_float += dst_float_gt[i] + dst_float_le[i];
    }
    
    /* Additional loop with mixed comparisons in same function */
    int mixed_results[N];
    for (int i = 0; i < N; i++) {
        /* Use all four comparison operators in sequence */
        if (src_int[i] > 30) {
            mixed_results[i] = 1;
        } else if (src_int[i] >= -30) {
            mixed_results[i] = 2;
        } else if (src_int[i] < -60) {
            mixed_results[i] = 3;
        } else if (src_int[i] <= 60) {
            mixed_results[i] = 4;
        } else {
            mixed_results[i] = 0;
        }
        sum_int += mixed_results[i];
    }
    
    printf("Integer checksum: %lld\n", sum_int);
    printf("Float checksum: %f\n", sum_float);
    
    return 0;
}

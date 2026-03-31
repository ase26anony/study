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

/* Additional test with float comparisons */
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
            dst[i] = src[i];
        }
    }
}

int main() {
    int src_int[N];
    int dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    float src_float[N];
    float dst_float_gt[N], dst_float_le[N];
    
    /* Initialize with non-uniform, non-constant data */
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        src_int[i] = (i % 200) - 100;  /* Range: -100 to 99 */
        src_float[i] = (float)((i % 150) - 75) * 0.7f;  /* Range: -52.5 to 52.5 */
    }
    
    /* Test all four integer comparison operators */
    test_gt(src_int, dst_gt, 25);      /* GT_EXPR */
    test_ge(src_int, dst_ge, -50);     /* GE_EXPR */
    test_lt(src_int, dst_lt, 0);       /* LT_EXPR */
    test_le(src_int, dst_le, 50);      /* LE_EXPR */
    
    /* Test floating-point comparisons */
    test_float_gt(src_float, dst_float_gt, 10.0f);  /* GT_EXPR with floats */
    test_float_le(src_float, dst_float_le, -10.0f); /* LE_EXPR with floats */
    
    /* Compute checksums to prevent dead code elimination */
    long long sum_int = 0;
    float sum_float = 0.0f;
    
    for (int i = 0; i < N; i++) {
        sum_int += dst_gt[i] + dst_ge[i] + dst_lt[i] + dst_le[i];
        sum_float += dst_float_gt[i] + dst_float_le[i];
    }
    
    /* Additional computation to ensure all results are used */
    int final_check = (int)(sum_int % 1000) + (int)(sum_float * 100) % 1000;
    
    printf("Final checksum: %d\n", final_check);
    
    return 0;
}

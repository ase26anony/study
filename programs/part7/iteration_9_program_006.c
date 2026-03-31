#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024
#define THRESHOLD 50
#define LIMIT 100
#define LOWER_BOUND -25
#define UPPER_BOUND 75

/* GT_EXPR transformation test */
void test_gt(int *src, int *dst_gt, int threshold) {
    for (int i = 0; i < N; i++) {
        if (src[i] > threshold) {
            dst_gt[i] = src[i] * 2;
        } else {
            dst_gt[i] = src[i];
        }
    }
}

/* GE_EXPR transformation test */
void test_ge(int *src, int *dst_ge, int limit) {
    for (int i = 0; i < N; i++) {
        if (src[i] >= limit) {
            dst_ge[i] = src[i] + 100;
        } else {
            dst_ge[i] = src[i] - 50;
        }
    }
}

/* LT_EXPR transformation test */
void test_lt(int *src, int *dst_lt, int lower_bound) {
    for (int i = 0; i < N; i++) {
        if (src[i] < lower_bound) {
            dst_lt[i] = 0;
        } else {
            dst_lt[i] = src[i];
        }
    }
}

/* LE_EXPR transformation test */
void test_le(int *src, int *dst_le, int upper_bound) {
    for (int i = 0; i < N; i++) {
        if (src[i] <= upper_bound) {
            dst_le[i] = -1;
        } else {
            dst_le[i] = 1;
        }
    }
}

/* Mixed comparison types in one loop to increase coverage probability */
void test_mixed_comparisons(short *src1, short *src2, short *dst_mixed) {
    for (int i = 0; i < N; i++) {
        /* Use all four comparison operators in different expressions */
        short val = src1[i];
        
        /* GT_EXPR pattern */
        if (val > src2[i]) {
            dst_mixed[i] = val;
        }
        /* GE_EXPR pattern */
        else if (val >= (src2[i] / 2)) {
            dst_mixed[i] = val * 2;
        }
        /* LT_EXPR pattern */
        else if (val < -src2[i]) {
            dst_mixed[i] = -val;
        }
        /* LE_EXPR pattern */
        else if (val <= (src2[i] + 10)) {
            dst_mixed[i] = val + src2[i];
        }
        else {
            dst_mixed[i] = 0;
        }
    }
}

/* Floating point comparisons to test different data types */
void test_float_comparisons(float *fsrc, float *fdst, float fthreshold) {
    for (int i = 0; i < N; i++) {
        /* GT_EXPR with floats */
        if (fsrc[i] > fthreshold) {
            fdst[i] = fsrc[i] * 1.5f;
        }
        /* GE_EXPR with floats */
        else if (fsrc[i] >= (fthreshold * 0.5f)) {
            fdst[i] = fsrc[i] + 10.0f;
        }
        /* LT_EXPR with floats */
        else if (fsrc[i] < -fthreshold) {
            fdst[i] = 0.0f;
        }
        /* LE_EXPR with floats */
        else if (fsrc[i] <= (fthreshold * 0.25f)) {
            fdst[i] = -1.0f;
        }
        else {
            fdst[i] = fsrc[i];
        }
    }
}

int main() {
    /* Initialize with non-constant, predictable pattern */
    int src[N];
    int dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    
    short src1[N], src2[N], dst_mixed[N];
    float fsrc[N], fdst[N];
    
    srand(time(NULL));
    
    /* Initialize integer arrays with mixed values */
    for (int i = 0; i < N; i++) {
        /* Pattern that ensures all comparisons will be taken for some elements */
        src[i] = (i % 200) - 100;  /* Values from -100 to 99 */
        
        /* For short arrays */
        src1[i] = (short)((i * 3) % 256 - 128);
        src2[i] = (short)((i * 5) % 256 - 128);
        
        /* For float arrays */
        fsrc[i] = (float)((i % 150) - 75) * 0.7f;
    }
    
    /* Test all four comparison operators */
    test_gt(src, dst_gt, THRESHOLD);
    test_ge(src, dst_ge, LIMIT);
    test_lt(src, dst_lt, LOWER_BOUND);
    test_le(src, dst_le, UPPER_BOUND);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(src1, src2, dst_mixed);
    
    /* Test floating point comparisons */
    test_float_comparisons(fsrc, fdst, 25.0f);
    
    /* Compute checksums to prevent dead code elimination */
    long long sum = 0;
    for (int i = 0; i < N; i++) {
        sum += dst_gt[i] + dst_ge[i] + dst_lt[i] + dst_le[i];
        sum += dst_mixed[i];
        sum += (long long)fdst[i];
    }
    
    printf("Final checksum: %lld\n", sum);
    
    /* Additional verification print to ensure all arrays are used */
    printf("Sample values - GT[0]=%d, GE[100]=%d, LT[200]=%d, LE[300]=%d\n",
           dst_gt[0], dst_ge[100], dst_lt[200], dst_le[300]);
    
    return 0;
}

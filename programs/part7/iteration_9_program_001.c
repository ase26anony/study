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

/* Mixed comparisons in a single loop to increase coverage probability */
void test_mixed_comparisons(short *src1, short *src2, short *dst_mixed) {
    for (int i = 0; i < N; i++) {
        /* Use all four comparison operators in different expressions */
        short val = src1[i];
        
        /* GT_EXPR */
        if (val > src2[i]) {
            dst_mixed[i] = val;
        }
        /* GE_EXPR */
        else if (val >= (src2[i] - 10)) {
            dst_mixed[i] = val / 2;
        }
        /* LT_EXPR */
        else if (val < (src2[i] + 20)) {
            dst_mixed[i] = val * 2;
        }
        /* LE_EXPR */
        else if (val <= (src2[i] + 40)) {
            dst_mixed[i] = val + 5;
        } else {
            dst_mixed[i] = 0;
        }
    }
}

/* Floating-point comparisons to test different data types */
void test_float_comparisons(float *fsrc, float *fdst, float fthreshold) {
    for (int i = 0; i < N; i++) {
        /* GT_EXPR with floats */
        if (fsrc[i] > fthreshold) {
            fdst[i] = fsrc[i] * 1.5f;
        }
        /* GE_EXPR with floats */
        else if (fsrc[i] >= (fthreshold - 10.0f)) {
            fdst[i] = fsrc[i] + 5.0f;
        }
        /* LT_EXPR with floats */
        else if (fsrc[i] < (fthreshold + 20.0f)) {
            fdst[i] = fsrc[i] - 3.0f;
        }
        /* LE_EXPR with floats */
        else if (fsrc[i] <= (fthreshold + 40.0f)) {
            fdst[i] = fsrc[i] / 2.0f;
        } else {
            fdst[i] = 0.0f;
        }
    }
}

int main() {
    /* Initialize with non-constant but predictable data */
    int src_int[N];
    short src_short1[N], src_short2[N];
    float src_float[N];
    
    /* Output arrays for each comparison type */
    int dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    short dst_mixed[N];
    float dst_float[N];
    
    /* Initialize random seed */
    srand(time(NULL));
    
    /* Initialize arrays with varying data patterns */
    for (int i = 0; i < N; i++) {
        /* Integer data with mix of positive and negative values */
        src_int[i] = (i % 200) - 100;  /* Range: -100 to 99 */
        
        /* Short data with different patterns */
        src_short1[i] = (short)((i * 3) % 256 - 128);
        src_short2[i] = (short)((i * 5) % 256 - 128);
        
        /* Float data with varying values */
        src_float[i] = (float)((i % 150) - 75) * 0.5f;
    }
    
    /* Test all four comparison operators in separate loops */
    test_gt(src_int, dst_gt, THRESHOLD);
    test_ge(src_int, dst_ge, LIMIT);
    test_lt(src_int, dst_lt, LOWER_BOUND);
    test_le(src_int, dst_le, UPPER_BOUND);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(src_short1, src_short2, dst_mixed);
    
    /* Test floating-point comparisons */
    test_float_comparisons(src_float, dst_float, 25.0f);
    
    /* Compute checksums to prevent dead code elimination */
    long long checksum = 0;
    float fchecksum = 0.0f;
    
    for (int i = 0; i < N; i++) {
        checksum += dst_gt[i] + dst_ge[i] + dst_lt[i] + dst_le[i] + dst_mixed[i];
        fchecksum += dst_float[i];
    }
    
    /* Print results to ensure side effects */
    printf("Integer checksum: %lld\n", checksum);
    printf("Float checksum: %f\n", fchecksum);
    
    /* Additional test with unsigned types */
    unsigned int usrc[N], udst[N];
    for (int i = 0; i < N; i++) {
        usrc[i] = (unsigned int)(i * 7) % 1000;
    }
    
    /* Test with unsigned comparisons */
    for (int i = 0; i < N; i++) {
        /* GT_EXPR with unsigned */
        if (usrc[i] > 500U) {
            udst[i] = usrc[i] * 2U;
        }
        /* GE_EXPR with unsigned */
        else if (usrc[i] >= 250U) {
            udst[i] = usrc[i] + 100U;
        }
        /* LT_EXPR with unsigned */
        else if (usrc[i] < 100U) {
            udst[i] = 0U;
        }
        /* LE_EXPR with unsigned */
        else if (usrc[i] <= 750U) {
            udst[i] = usrc[i] / 2U;
        } else {
            udst[i] = usrc[i];
        }
    }
    
    /* Final checksum */
    unsigned long long uchecksum = 0;
    for (int i = 0; i < N; i++) {
        uchecksum += udst[i];
    }
    printf("Unsigned checksum: %llu\n", uchecksum);
    
    return 0;
}

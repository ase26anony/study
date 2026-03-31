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

/* Mixed comparisons in one loop to potentially trigger multiple patterns */
void test_mixed_comparisons(short *src1, short *src2, short *dst_mixed) {
    for (int i = 0; i < N; i++) {
        /* Use all four comparison operators in one loop */
        short val = 0;
        
        if (src1[i] > src2[i]) {
            val += 10;
        }
        
        if (src1[i] >= src2[i] + 5) {
            val += 20;
        }
        
        if (src1[i] < src2[i] - 3) {
            val += 30;
        }
        
        if (src1[i] <= src2[i] + 2) {
            val += 40;
        }
        
        dst_mixed[i] = val;
    }
}

/* Floating-point comparisons to test different data types */
void test_float_comparisons(float *fsrc, float *fdst, float fthreshold) {
    for (int i = 0; i < N; i++) {
        if (fsrc[i] > fthreshold) {
            fdst[i] = fsrc[i] * 1.5f;
        } else if (fsrc[i] >= fthreshold - 10.0f) {
            fdst[i] = fsrc[i] + 5.0f;
        } else if (fsrc[i] < fthreshold - 20.0f) {
            fdst[i] = 0.0f;
        } else if (fsrc[i] <= fthreshold - 5.0f) {
            fdst[i] = -fsrc[i];
        } else {
            fdst[i] = fsrc[i];
        }
    }
}

int main() {
    /* Initialize with predictable but non-constant data */
    int src[N];
    int dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    
    short src1[N], src2[N], dst_mixed[N];
    float fsrc[N], fdst[N];
    
    srand(time(NULL));
    
    /* Initialize integer arrays with varying data */
    for (int i = 0; i < N; i++) {
        /* Create data that will trigger different comparison outcomes */
        src[i] = (i % 200) - 100;  /* Values from -100 to 99 */
        
        /* For short arrays */
        src1[i] = (short)((i * 3) % 256 - 128);
        src2[i] = (short)((i * 7) % 256 - 128);
        
        /* For float arrays */
        fsrc[i] = (float)((i % 150) - 75) * 0.5f;
    }
    
    /* Test each comparison operator in separate loops */
    test_gt(src, dst_gt, THRESHOLD);
    test_ge(src, dst_ge, LIMIT);
    test_lt(src, dst_lt, LOWER_BOUND);
    test_le(src, dst_le, UPPER_BOUND);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(src1, src2, dst_mixed);
    
    /* Test floating-point comparisons */
    test_float_comparisons(fsrc, fdst, 25.0f);
    
    /* Compute checksums to prevent dead code elimination */
    long long sum_gt = 0, sum_ge = 0, sum_lt = 0, sum_le = 0;
    long long sum_mixed = 0;
    float sum_float = 0.0f;
    
    for (int i = 0; i < N; i++) {
        sum_gt += dst_gt[i];
        sum_ge += dst_ge[i];
        sum_lt += dst_lt[i];
        sum_le += dst_le[i];
        sum_mixed += dst_mixed[i];
        sum_float += fdst[i];
    }
    
    /* Print results to ensure no optimization eliminates the loops */
    printf("GT checksum: %lld\n", sum_gt);
    printf("GE checksum: %lld\n", sum_ge);
    printf("LT checksum: %lld\n", sum_lt);
    printf("LE checksum: %lld\n", sum_le);
    printf("Mixed checksum: %lld\n", sum_mixed);
    printf("Float checksum: %f\n", sum_float);
    
    /* Final aggregate to ensure all results are used */
    long long final_sum = sum_gt + sum_ge + sum_lt + sum_le + sum_mixed + (long long)sum_float;
    printf("Final aggregate: %lld\n", final_sum);
    
    return 0;
}

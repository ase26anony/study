#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024
#define THRESHOLD 50
#define LIMIT 100
#define LOWER_BOUND -25
#define UPPER_BOUND 75

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

/* Mixed comparisons in one loop to increase coverage probability */
void test_mixed_comparisons(int *src1, int *src2, int *dst1, int *dst2, 
                           int *dst3, int *dst4) {
    for (int i = 0; i < N; i++) {
        /* GT_EXPR */
        if (src1[i] > THRESHOLD) {
            dst1[i] = src1[i] * 3;
        } else {
            dst1[i] = src1[i];
        }
        
        /* GE_EXPR */
        if (src1[i] >= LIMIT) {
            dst2[i] = src1[i] + 200;
        } else {
            dst2[i] = src1[i] - 100;
        }
        
        /* LT_EXPR */
        if (src2[i] < LOWER_BOUND) {
            dst3[i] = src2[i] * -1;
        } else {
            dst3[i] = src2[i];
        }
        
        /* LE_EXPR */
        if (src2[i] <= UPPER_BOUND) {
            dst4[i] = src2[i] / 2;
        } else {
            dst4[i] = src2[i] * 2;
        }
    }
}

int main() {
    int src1[N], src2[N];
    int dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    int dst_mixed1[N], dst_mixed2[N], dst_mixed3[N], dst_mixed4[N];
    
    /* Initialize with predictable but non-constant pattern */
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        /* Mix of positive and negative values to trigger various comparisons */
        src1[i] = (i % 200) - 100;      /* Range: -100 to 99 */
        src2[i] = (rand() % 200) - 50;  /* Range: -50 to 149 */
    }
    
    /* Test each comparison operator separately */
    test_gt(src1, dst_gt, THRESHOLD);
    test_ge(src1, dst_ge, LIMIT);
    test_lt(src1, dst_lt, LOWER_BOUND);
    test_le(src1, dst_le, UPPER_BOUND);
    
    /* Test mixed comparisons in one loop */
    test_mixed_comparisons(src1, src2, dst_mixed1, dst_mixed2, 
                          dst_mixed3, dst_mixed4);
    
    /* Compute checksums to prevent dead code elimination */
    long long sum = 0;
    for (int i = 0; i < N; i++) {
        sum += dst_gt[i] + dst_ge[i] + dst_lt[i] + dst_le[i];
        sum += dst_mixed1[i] + dst_mixed2[i] + dst_mixed3[i] + dst_mixed4[i];
    }
    
    printf("Final checksum: %lld\n", sum);
    
    /* Additional test with floating point to cover more cases */
    float fsrc[N], fdst_gt[N], fdst_ge[N], fdst_lt[N], fdst_le[N];
    for (int i = 0; i < N; i++) {
        fsrc[i] = (i % 100) * 0.5f - 25.0f; /* Range: -25.0 to 24.5 */
    }
    
    /* Floating point comparisons should also trigger the transformations */
    for (int i = 0; i < N; i++) {
        /* GT_EXPR with float */
        if (fsrc[i] > 0.0f) {
            fdst_gt[i] = fsrc[i] * 2.0f;
        } else {
            fdst_gt[i] = fsrc[i];
        }
        
        /* GE_EXPR with float */
        if (fsrc[i] >= 10.0f) {
            fdst_ge[i] = fsrc[i] + 5.0f;
        } else {
            fdst_ge[i] = fsrc[i] - 5.0f;
        }
        
        /* LT_EXPR with float */
        if (fsrc[i] < -10.0f) {
            fdst_lt[i] = 0.0f;
        } else {
            fdst_lt[i] = fsrc[i];
        }
        
        /* LE_EXPR with float */
        if (fsrc[i] <= 20.0f) {
            fdst_le[i] = -1.0f;
        } else {
            fdst_le[i] = 1.0f;
        }
    }
    
    /* Add floating point results to checksum */
    float fsum = 0.0f;
    for (int i = 0; i < N; i++) {
        fsum += fdst_gt[i] + fdst_ge[i] + fdst_lt[i] + fdst_le[i];
    }
    
    printf("Floating point checksum: %f\n", fsum);
    
    return 0;
}

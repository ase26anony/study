#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024
#define THRESHOLD 50
#define LIMIT 75
#define LOWER_BOUND -25
#define UPPER_BOUND 25

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
            dst[i] = src[i] * 3;
        }
    }
}

/* LE_EXPR transformation test */
void test_le(int *src, int *dst, int upper_bound) {
    for (int i = 0; i < N; i++) {
        if (src[i] <= upper_bound) {
            dst[i] = -1;
        } else {
            dst[i] = src[i] / 2;
        }
    }
}

/* Mixed comparisons in a single loop to potentially trigger multiple patterns */
void test_mixed_comparisons(int *src1, int *src2, int *dst1, int *dst2, int *dst3, int *dst4) {
    for (int i = 0; i < N; i++) {
        /* GT_EXPR */
        if (src1[i] > THRESHOLD) {
            dst1[i] = src1[i] + src2[i];
        } else {
            dst1[i] = src1[i] - src2[i];
        }
        
        /* GE_EXPR */
        if (src2[i] >= LIMIT) {
            dst2[i] = src1[i] * src2[i];
        } else {
            dst2[i] = src1[i];
        }
        
        /* LT_EXPR */
        if (src1[i] < LOWER_BOUND) {
            dst3[i] = src2[i];
        } else {
            dst3[i] = src1[i];
        }
        
        /* LE_EXPR */
        if (src2[i] <= UPPER_BOUND) {
            dst4[i] = 255;
        } else {
            dst4[i] = 0;
        }
    }
}

/* Initialize array with non-uniform, non-constant pattern */
void init_array(int *arr, int size, int seed) {
    srand(seed);
    for (int i = 0; i < size; i++) {
        /* Create a mix of values that will trigger different comparison outcomes */
        arr[i] = (rand() % 200) - 100;  /* Values between -100 and 99 */
    }
}

int main() {
    int src1[N], src2[N];
    int dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    int dst_mixed1[N], dst_mixed2[N], dst_mixed3[N], dst_mixed4[N];
    
    /* Initialize with different seeds for varied data patterns */
    init_array(src1, N, 42);
    init_array(src2, N, 123);
    
    /* Test each comparison operator separately */
    test_gt(src1, dst_gt, THRESHOLD);
    test_ge(src1, dst_ge, LIMIT);
    test_lt(src1, dst_lt, LOWER_BOUND);
    test_le(src1, dst_le, UPPER_BOUND);
    
    /* Test mixed comparisons in a single loop */
    test_mixed_comparisons(src1, src2, dst_mixed1, dst_mixed2, dst_mixed3, dst_mixed4);
    
    /* Compute checksums to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += dst_gt[i] + dst_ge[i] + dst_lt[i] + dst_le[i];
        checksum += dst_mixed1[i] + dst_mixed2[i] + dst_mixed3[i] + dst_mixed4[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    /* Additional test with different data types to cover more cases */
    {
        short src_short[N];
        short dst_short[N];
        
        for (int i = 0; i < N; i++) {
            src_short[i] = (short)((i * 37) % 256 - 128);
        }
        
        /* LT_EXPR with short type */
        for (int i = 0; i < N; i++) {
            if (src_short[i] < -64) {
                dst_short[i] = src_short[i] * 2;
            } else {
                dst_short[i] = src_short[i];
            }
        }
        
        short short_sum = 0;
        for (int i = 0; i < N; i++) {
            short_sum += dst_short[i];
        }
        printf("Short checksum: %d\n", short_sum);
    }
    
    return 0;
}

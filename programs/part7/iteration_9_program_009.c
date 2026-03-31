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

/* Initialize array with varying data pattern */
void init_array(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        /* Create a mix of values that will trigger different comparison outcomes */
        arr[i] = (i % 200) - 100;  /* Values from -100 to 99 */
    }
}

int main() {
    int src[N];
    int dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    long long sum = 0;
    
    /* Initialize with varying data */
    srand(time(NULL));
    init_array(src, N);
    
    /* Add some randomness to ensure non-uniform data */
    for (int i = 0; i < N; i++) {
        src[i] += rand() % 20 - 10;
    }
    
    /* Test all four comparison operators */
    test_gt(src, dst_gt, THRESHOLD);
    test_ge(src, dst_ge, LIMIT);
    test_lt(src, dst_lt, LOWER_BOUND);
    test_le(src, dst_le, UPPER_BOUND);
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        sum += dst_gt[i] + dst_ge[i] + dst_lt[i] + dst_le[i];
    }
    
    printf("Final checksum: %lld\n", sum);
    
    return 0;
}

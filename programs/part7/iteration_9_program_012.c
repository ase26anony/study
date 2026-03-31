#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024
#define THRESHOLD 50
#define LIMIT 100
#define LOWER_BOUND -30
#define UPPER_BOUND 70

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

int main() {
    /* Initialize with predictable but non-constant data */
    int src[N];
    int dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    
    /* Mixed pattern to ensure comparisons have varying results */
    for (int i = 0; i < N; i++) {
        /* Create a sawtooth pattern from -100 to 100 */
        src[i] = (i % 201) - 100;
    }
    
    /* Add some randomness to prevent complete optimization */
    srand(time(NULL));
    for (int i = 0; i < N/4; i++) {
        int idx = rand() % N;
        src[idx] = rand() % 200 - 100;
    }
    
    /* Test all four comparison operators */
    test_gt(src, dst_gt, THRESHOLD);
    test_ge(src, dst_ge, LIMIT);
    test_lt(src, dst_lt, LOWER_BOUND);
    test_le(src, dst_le, UPPER_BOUND);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += dst_gt[i];
        checksum += dst_ge[i];
        checksum += dst_lt[i];
        checksum += dst_le[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Additional verification prints */
    printf("Sample results (first 5 elements):\n");
    for (int i = 0; i < 5; i++) {
        printf("src[%d]=%d, gt=%d, ge=%d, lt=%d, le=%d\n",
               i, src[i], dst_gt[i], dst_ge[i], dst_lt[i], dst_le[i]);
    }
    
    return 0;
}

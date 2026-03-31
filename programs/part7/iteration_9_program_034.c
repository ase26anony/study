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

/* Initialize array with mixed values to ensure both true/false branches are taken */
void init_array(int *arr) {
    for (int i = 0; i < N; i++) {
        /* Pattern: values from -100 to 100, ensuring all comparisons will be triggered */
        arr[i] = (i % 201) - 100;
    }
}

int main() {
    /* Allocate arrays */
    int src[N];
    int dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    
    /* Initialize source array with non-uniform data */
    init_array(src);
    
    /* Seed random number generator for additional variation */
    srand(time(NULL));
    
    /* Add some random perturbation to ensure data is not perfectly predictable */
    for (int i = 0; i < N; i++) {
        src[i] += (rand() % 5) - 2;
    }
    
    /* Execute all four comparison tests */
    test_gt(src, dst_gt, THRESHOLD);      /* > comparison */
    test_ge(src, dst_ge, LIMIT);          /* >= comparison */
    test_lt(src, dst_lt, LOWER_BOUND);    /* < comparison */
    test_le(src, dst_le, UPPER_BOUND);    /* <= comparison */
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += dst_gt[i] + dst_ge[i] + dst_lt[i] + dst_le[i];
    }
    
    /* Print result to ensure side effects are visible */
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}

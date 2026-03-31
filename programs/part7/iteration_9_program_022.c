#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024
#define THRESHOLD 50
#define LIMIT 75
#define LOWER_BOUND -25
#define UPPER_BOUND 25

/* GT_EXPR pattern: if (a[i] > threshold) b[i] = value */
void test_gt(int *a, int *b, int threshold, int value) {
    for (int i = 0; i < N; i++) {
        if (a[i] > threshold) {
            b[i] = value;
        } else {
            b[i] = 0;
        }
    }
}

/* GE_EXPR pattern: if (a[i] >= limit) c[i] = a[i] */
void test_ge(int *a, int *c, int limit) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= limit) {
            c[i] = a[i];
        } else {
            c[i] = -a[i];
        }
    }
}

/* LT_EXPR pattern: if (a[i] < lower_bound) d[i] = 0 */
void test_lt(int *a, int *d, int lower_bound) {
    for (int i = 0; i < N; i++) {
        if (a[i] < lower_bound) {
            d[i] = 0;
        } else {
            d[i] = 1;
        }
    }
}

/* LE_EXPR pattern: if (a[i] <= upper_bound) e[i] = -1 */
void test_le(int *a, int *e, int upper_bound) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= upper_bound) {
            e[i] = -1;
        } else {
            e[i] = 1;
        }
    }
}

/* Additional test with floating point to ensure different data types */
void test_float_gt(float *fa, float *fb, float threshold) {
    for (int i = 0; i < N; i++) {
        if (fa[i] > threshold) {
            fb[i] = fa[i] * 2.0f;
        } else {
            fb[i] = fa[i];
        }
    }
}

int main() {
    /* Initialize with different patterns to avoid constant propagation */
    int a[N], b[N], c[N], d[N], e[N];
    float fa[N], fb[N];
    
    srand(time(NULL));
    
    /* Create non-uniform data with predictable but varying patterns */
    for (int i = 0; i < N; i++) {
        /* Mix of positive and negative values to trigger different comparisons */
        a[i] = (i % 200) - 100;  /* Values from -100 to 99 */
        fa[i] = (float)((i % 150) - 75) * 0.5f;  /* Float values from -37.5 to 37.0 */
    }
    
    /* Test all four comparison operators */
    test_gt(a, b, THRESHOLD, 100);
    test_ge(a, c, LIMIT);
    test_lt(a, d, LOWER_BOUND);
    test_le(a, e, UPPER_BOUND);
    test_float_gt(fa, fb, 10.0f);
    
    /* Compute checksums to prevent dead code elimination */
    long long sum_gt = 0, sum_ge = 0, sum_lt = 0, sum_le = 0;
    float sum_float = 0.0f;
    
    for (int i = 0; i < N; i++) {
        sum_gt += b[i];
        sum_ge += c[i];
        sum_lt += d[i];
        sum_le += e[i];
        sum_float += fb[i];
    }
    
    /* Print results to ensure side effects */
    printf("GT checksum: %lld\n", sum_gt);
    printf("GE checksum: %lld\n", sum_ge);
    printf("LT checksum: %lld\n", sum_lt);
    printf("LE checksum: %lld\n", sum_le);
    printf("Float GT checksum: %.2f\n", sum_float);
    
    /* Final aggregate to ensure all results are used */
    long long total = sum_gt + sum_ge + sum_lt + sum_le + (long long)sum_float;
    printf("Total aggregate: %lld\n", total);
    
    return 0;
}

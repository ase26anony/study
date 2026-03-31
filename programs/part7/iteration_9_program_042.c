#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024
#define THRESHOLD 50
#define LIMIT 100
#define LOWER_BOUND -30
#define UPPER_BOUND 70

/* Function for GT_EXPR (>) comparison */
void vector_gt_expr(int *src, int *dst, int threshold) {
    for (int i = 0; i < N; i++) {
        if (src[i] > threshold) {
            dst[i] = src[i] * 2;
        } else {
            dst[i] = src[i];
        }
    }
}

/* Function for GE_EXPR (>=) comparison */
void vector_ge_expr(int *src, int *dst, int limit) {
    for (int i = 0; i < N; i++) {
        if (src[i] >= limit) {
            dst[i] = src[i] + 100;
        } else {
            dst[i] = src[i] - 50;
        }
    }
}

/* Function for LT_EXPR (<) comparison */
void vector_lt_expr(int *src, int *dst, int lower_bound) {
    for (int i = 0; i < N; i++) {
        if (src[i] < lower_bound) {
            dst[i] = 0;
        } else {
            dst[i] = src[i] * 3;
        }
    }
}

/* Function for LE_EXPR (<=) comparison */
void vector_le_expr(int *src, int *dst, int upper_bound) {
    for (int i = 0; i < N; i++) {
        if (src[i] <= upper_bound) {
            dst[i] = -1;
        } else {
            dst[i] = src[i] / 2;
        }
    }
}

/* Initialize array with varying values to ensure both true and false comparisons */
void init_array(int *arr) {
    for (int i = 0; i < N; i++) {
        /* Create a pattern that ensures some elements satisfy conditions, others don't */
        arr[i] = (i % 200) - 100;  /* Values from -100 to 99 */
    }
}

int main() {
    int src[N];
    int dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    long long sum = 0;
    
    /* Seed random number generator for varied initialization */
    srand(time(NULL));
    
    /* Initialize source array with non-uniform data */
    init_array(src);
    
    /* Add some random variation to ensure compiler can't predict all values */
    for (int i = 0; i < N; i += 7) {
        src[i] = rand() % 200 - 100;
    }
    
    /* Execute all four comparison patterns */
    vector_gt_expr(src, dst_gt, THRESHOLD);      /* > comparison */
    vector_ge_expr(src, dst_ge, LIMIT);          /* >= comparison */
    vector_lt_expr(src, dst_lt, LOWER_BOUND);    /* < comparison */
    vector_le_expr(src, dst_le, UPPER_BOUND);    /* <= comparison */
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        sum += dst_gt[i] + dst_ge[i] + dst_lt[i] + dst_le[i];
    }
    
    /* Print result to create side effect */
    printf("Checksum: %lld\n", sum);
    
    return 0;
}

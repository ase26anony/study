#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024
#define THRESHOLD 50
#define LIMIT 75
#define LOWER_BOUND 25
#define UPPER_BOUND 150

/* Function prototypes for each comparison type */
void vector_gt(int *src, int *dst, int threshold) {
    for (int i = 0; i < N; i++) {
        if (src[i] > threshold) {
            dst[i] = src[i] * 2;  // Conditional assignment for GT_EXPR
        } else {
            dst[i] = src[i];      // Ensure all elements are written
        }
    }
}

void vector_ge(int *src, int *dst, int limit) {
    for (int i = 0; i < N; i++) {
        if (src[i] >= limit) {
            dst[i] = src[i] + 100;  // Conditional assignment for GE_EXPR
        } else {
            dst[i] = src[i] - 50;
        }
    }
}

void vector_lt(int *src, int *dst, int lower_bound) {
    for (int i = 0; i < N; i++) {
        if (src[i] < lower_bound) {
            dst[i] = 0;            // Conditional assignment for LT_EXPR
        } else {
            dst[i] = src[i];
        }
    }
}

void vector_le(int *src, int *dst, int upper_bound) {
    for (int i = 0; i < N; i++) {
        if (src[i] <= upper_bound) {
            dst[i] = -1;           // Conditional assignment for LE_EXPR
        } else {
            dst[i] = 1;
        }
    }
}

/* Additional test with floating point to cover more cases */
void vector_gt_float(float *src, float *dst, float threshold) {
    for (int i = 0; i < N; i++) {
        if (src[i] > threshold) {   // GT_EXPR with floats
            dst[i] = src[i] * 1.5f;
        } else {
            dst[i] = src[i];
        }
    }
}

void vector_le_float(float *src, float *dst, float upper_bound) {
    for (int i = 0; i < N; i++) {
        if (src[i] <= upper_bound) { // LE_EXPR with floats
            dst[i] = src[i] * 0.5f;
        } else {
            dst[i] = src[i];
        }
    }
}

int main() {
    /* Initialize with different patterns to avoid constant propagation */
    int src_int[N];
    float src_float[N];
    int dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    float dst_gt_float[N], dst_le_float[N];
    
    srand(time(NULL));
    
    /* Create non-uniform data patterns */
    for (int i = 0; i < N; i++) {
        /* Integer pattern: mix of values around thresholds */
        src_int[i] = (i % 200) - 100;  // Values from -100 to 99
        
        /* Float pattern: similar but with fractional parts */
        src_float[i] = ((i % 200) - 100) * 0.7f;
    }
    
    /* Add some randomness to prevent optimization */
    for (int i = 0; i < N/4; i++) {
        int idx = rand() % N;
        src_int[idx] = rand() % 300 - 150;
        src_float[idx] = (rand() % 300 - 150) * 0.5f;
    }
    
    /* Execute all four comparison types */
    vector_gt(src_int, dst_gt, THRESHOLD);      // Triggers GT_EXPR case
    vector_ge(src_int, dst_ge, LIMIT);          // Triggers GE_EXPR case
    vector_lt(src_int, dst_lt, LOWER_BOUND);    // Triggers LT_EXPR case
    vector_le(src_int, dst_le, UPPER_BOUND);    // Triggers LE_EXPR case
    
    /* Also test with floating point */
    vector_gt_float(src_float, dst_gt_float, 35.0f);    // GT_EXPR with float
    vector_le_float(src_float, dst_le_float, 85.0f);    // LE_EXPR with float
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += dst_gt[i] + dst_ge[i] + dst_lt[i] + dst_le[i];
        checksum += (long long)(dst_gt_float[i] + dst_le_float[i]);
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Additional test with different data types to increase coverage */
    short src_short[N];
    short dst_short_gt[N], dst_short_le[N];
    
    for (int i = 0; i < N; i++) {
        src_short[i] = (short)((i % 256) - 128);
    }
    
    /* Short type loops with comparisons */
    for (int i = 0; i < N; i++) {
        if (src_short[i] > 64) {           // GT_EXPR with short
            dst_short_gt[i] = src_short[i] * 2;
        } else {
            dst_short_gt[i] = src_short[i];
        }
        
        if (src_short[i] <= -64) {         // LE_EXPR with short
            dst_short_le[i] = -src_short[i];
        } else {
            dst_short_le[i] = src_short[i];
        }
    }
    
    /* Add to checksum */
    for (int i = 0; i < N; i++) {
        checksum += dst_short_gt[i] + dst_short_le[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    return 0;
}

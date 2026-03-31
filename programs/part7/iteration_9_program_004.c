#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024  // Multiple of typical vector widths (128, 256 bits)

// Function prototypes for each comparison type
void vectorize_gt(int *src, int *dst, int threshold);
void vectorize_ge(int *src, int *dst, int limit);
void vectorize_lt(int *src, int *dst, int lower_bound);
void vectorize_le(int *src, int *dst, int upper_bound);

int main() {
    // Seed random number generator for non-constant data
    srand(time(NULL));
    
    // Allocate and initialize input arrays with varying data
    int src1[N], src2[N];
    for (int i = 0; i < N; i++) {
        // Mix of positive, negative, and zero values
        src1[i] = (i % 100) - 50;          // Range: -50 to 49
        src2[i] = (rand() % 200) - 100;    // Range: -100 to 99
    }
    
    // Allocate output arrays for each comparison type
    int dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    
    // Initialize output arrays to known values
    for (int i = 0; i < N; i++) {
        dst_gt[i] = 0;
        dst_ge[i] = 0;
        dst_lt[i] = 0;
        dst_le[i] = 0;
    }
    
    // Define thresholds for each comparison
    int gt_threshold = 25;      // For GT_EXPR: a[i] > 25
    int ge_limit = -10;         // For GE_EXPR: a[i] >= -10
    int lt_lower_bound = 30;    // For LT_EXPR: a[i] < 30
    int le_upper_bound = 40;    // For LE_EXPR: a[i] <= 40
    
    // Execute all four vectorizable loops
    vectorize_gt(src1, dst_gt, gt_threshold);
    vectorize_ge(src1, dst_ge, ge_limit);
    vectorize_lt(src2, dst_lt, lt_lower_bound);
    vectorize_le(src2, dst_le, le_upper_bound);
    
    // Compute checksum to prevent dead code elimination
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += dst_gt[i];
        checksum += dst_ge[i];
        checksum += dst_lt[i];
        checksum += dst_le[i];
    }
    
    // Print result to ensure side effects
    printf("Final checksum: %lld\n", checksum);
    
    return 0;
}

// GT_EXPR transformation: if (src[i] > threshold) dst[i] = value;
// Should trigger: bitop1 = BIT_NOT_EXPR; bitop2 = BIT_AND_EXPR;
void vectorize_gt(int *src, int *dst, int threshold) {
    for (int i = 0; i < N; i++) {
        // Conditional assignment using GT_EXPR
        if (src[i] > threshold) {
            dst[i] = src[i] * 2;  // Non-trivial transformation
        } else {
            dst[i] = -1;          // Different value for false case
        }
    }
}

// GE_EXPR transformation: if (src[i] >= limit) dst[i] = value;
// Should trigger: bitop1 = BIT_NOT_EXPR; bitop2 = BIT_IOR_EXPR;
void vectorize_ge(int *src, int *dst, int limit) {
    for (int i = 0; i < N; i++) {
        // Conditional assignment using GE_EXPR
        if (src[i] >= limit) {
            dst[i] = src[i] + 100;  // Different operation
        } else {
            dst[i] = 0;
        }
    }
}

// LT_EXPR transformation: if (src[i] < lower_bound) dst[i] = value;
// Should trigger: bitop1 = BIT_NOT_EXPR; bitop2 = BIT_AND_EXPR;
//                 std::swap(cond_expr0, cond_expr1);
void vectorize_lt(int *src, int *dst, int lower_bound) {
    for (int i = 0; i < N; i++) {
        // Conditional assignment using LT_EXPR
        if (src[i] < lower_bound) {
            dst[i] = src[i] / 2;  // Different operation
        } else {
            dst[i] = 999;
        }
    }
}

// LE_EXPR transformation: if (src[i] <= upper_bound) dst[i] = value;
// Should trigger: bitop1 = BIT_NOT_EXPR; bitop2 = BIT_IOR_EXPR;
//                 std::swap(cond_expr0, cond_expr1);
void vectorize_le(int *src, int *dst, int upper_bound) {
    for (int i = 0; i < N; i++) {
        // Conditional assignment using LE_EXPR
        if (src[i] <= upper_bound) {
            dst[i] = src[i] - 50;
        } else {
            dst[i] = 777;
        }
    }
}

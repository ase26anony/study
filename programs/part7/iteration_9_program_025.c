#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024
#define THRESHOLD 50
#define LIMIT 75
#define LOWER_BOUND 25
#define UPPER_BOUND 150

/* GT_EXPR transformation test */
void test_gt_expr(int *src, int *dst, int threshold) {
    for (int i = 0; i < N; i++) {
        if (src[i] > threshold) {
            dst[i] = src[i] * 2;
        } else {
            dst[i] = src[i];
        }
    }
}

/* GE_EXPR transformation test */
void test_ge_expr(int *src, int *dst, int limit) {
    for (int i = 0; i < N; i++) {
        if (src[i] >= limit) {
            dst[i] = src[i] + 100;
        } else {
            dst[i] = src[i] - 50;
        }
    }
}

/* LT_EXPR transformation test */
void test_lt_expr(int *src, int *dst, int lower_bound) {
    for (int i = 0; i < N; i++) {
        if (src[i] < lower_bound) {
            dst[i] = 0;
        } else {
            dst[i] = src[i] * 3;
        }
    }
}

/* LE_EXPR transformation test */
void test_le_expr(int *src, int *dst, int upper_bound) {
    for (int i = 0; i < N; i++) {
        if (src[i] <= upper_bound) {
            dst[i] = -1;
        } else {
            dst[i] = src[i] / 2;
        }
    }
}

/* Mixed comparisons in a single loop to increase coverage probability */
void test_mixed_comparisons(int *src1, int *src2, int *dst1, int *dst2, 
                           int *dst3, int *dst4) {
    for (int i = 0; i < N; i++) {
        /* GT_EXPR pattern */
        if (src1[i] > THRESHOLD) {
            dst1[i] = src1[i] + src2[i];
        } else {
            dst1[i] = src1[i] - src2[i];
        }
        
        /* GE_EXPR pattern */
        if (src2[i] >= LIMIT) {
            dst2[i] = src1[i] * src2[i];
        } else {
            dst2[i] = src1[i];
        }
        
        /* LT_EXPR pattern */
        if (src1[i] < LOWER_BOUND) {
            dst3[i] = src2[i];
        } else {
            dst3[i] = src1[i];
        }
        
        /* LE_EXPR pattern */
        if (src2[i] <= UPPER_BOUND) {
            dst4[i] = 1;
        } else {
            dst4[i] = 0;
        }
    }
}

int main() {
    /* Initialize with different patterns to avoid constant propagation */
    int src1[N], src2[N];
    int dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    int dst_mixed1[N], dst_mixed2[N], dst_mixed3[N], dst_mixed4[N];
    
    srand(time(NULL));
    
    /* Create non-uniform data patterns that will trigger different comparison outcomes */
    for (int i = 0; i < N; i++) {
        /* Pattern 1: Values ranging from -100 to 200 */
        src1[i] = (i % 300) - 100;
        
        /* Pattern 2: Random values with some structure */
        src2[i] = (rand() % 256) - 50;
    }
    
    /* Test each comparison operator separately */
    test_gt_expr(src1, dst_gt, THRESHOLD);
    test_ge_expr(src1, dst_ge, LIMIT);
    test_lt_expr(src1, dst_lt, LOWER_BOUND);
    test_le_expr(src1, dst_le, UPPER_BOUND);
    
    /* Test mixed comparisons in one loop */
    test_mixed_comparisons(src1, src2, dst_mixed1, dst_mixed2, 
                          dst_mixed3, dst_mixed4);
    
    /* Compute checksums to prevent dead code elimination */
    long long sum_gt = 0, sum_ge = 0, sum_lt = 0, sum_le = 0;
    long long sum_mixed1 = 0, sum_mixed2 = 0, sum_mixed3 = 0, sum_mixed4 = 0;
    
    for (int i = 0; i < N; i++) {
        sum_gt += dst_gt[i];
        sum_ge += dst_ge[i];
        sum_lt += dst_lt[i];
        sum_le += dst_le[i];
        sum_mixed1 += dst_mixed1[i];
        sum_mixed2 += dst_mixed2[i];
        sum_mixed3 += dst_mixed3[i];
        sum_mixed4 += dst_mixed4[i];
    }
    
    /* Combine all checksums into a final result */
    long long final_result = sum_gt + sum_ge + sum_lt + sum_le +
                           sum_mixed1 + sum_mixed2 + sum_mixed3 + sum_mixed4;
    
    printf("Final checksum: %lld\n", final_result);
    
    /* Additional test with floating point to cover more cases */
    float f_src[N], f_dst_gt[N], f_dst_le[N];
    for (int i = 0; i < N; i++) {
        f_src[i] = (i % 100) * 1.5f - 75.0f;
    }
    
    /* Floating point GT and LE comparisons */
    for (int i = 0; i < N; i++) {
        if (f_src[i] > 0.0f) {
            f_dst_gt[i] = f_src[i] * 2.0f;
        } else {
            f_dst_gt[i] = f_src[i];
        }
        
        if (f_src[i] <= 50.0f) {
            f_dst_le[i] = -f_src[i];
        } else {
            f_dst_le[i] = f_src[i];
        }
    }
    
    /* Compute floating point checksum */
    float f_sum = 0.0f;
    for (int i = 0; i < N; i++) {
        f_sum += f_dst_gt[i] + f_dst_le[i];
    }
    printf("Float checksum: %f\n", f_sum);
    
    return 0;
}

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

int main() {
    // Aligned arrays to help vectorization
    ALIGNED int a[N], b[N];
    ALIGNED char gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Initialize with distinct patterns to avoid trivial comparisons
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;      // Values 0-99
        b[i] = (i * 7) % 100;      // Different pattern
    }
    
    int sum_gt = 0, sum_ge = 0, sum_lt = 0, sum_le = 0;
    
    // Loop 1: Greater-than comparisons (GT_EXPR)
    // This should trigger the GT_EXPR -> BIT_AND_EXPR transformation
    for (int i = 0; i < N; i++) {
        gt_result[i] = (a[i] > b[i]);  // GT_EXPR
    }
    
    // Loop 2: Greater-or-equal comparisons (GE_EXPR)
    // This should trigger the GE_EXPR -> BIT_IOR_EXPR transformation
    for (int i = 0; i < N; i++) {
        ge_result[i] = (a[i] >= b[i]); // GE_EXPR
    }
    
    // Loop 3: Less-than comparisons (LT_EXPR)
    // This should trigger the LT_EXPR -> BIT_AND_EXPR with operand swap
    for (int i = 0; i < N; i++) {
        lt_result[i] = (a[i] < b[i]);  // LT_EXPR
    }
    
    // Loop 4: Less-or-equal comparisons (LE_EXPR)
    // This should trigger the LE_EXPR -> BIT_IOR_EXPR with operand swap
    for (int i = 0; i < N; i++) {
        le_result[i] = (a[i] <= b[i]); // LE_EXPR
    }
    
    // Use results to prevent dead code elimination
    for (int i = 0; i < N; i++) {
        sum_gt += gt_result[i];
        sum_ge += ge_result[i];
        sum_lt += lt_result[i];
        sum_le += le_result[i];
    }
    
    // Print checksums (also prevents optimization)
    printf("GT checksum: %d\n", sum_gt);
    printf("GE checksum: %d\n", sum_ge);
    printf("LT checksum: %d\n", sum_lt);
    printf("LE checksum: %d\n", sum_le);
    
    // Return value based on comparisons
    return (sum_gt > 0 && sum_ge > 0 && sum_lt > 0 && sum_le > 0) ? 0 : 1;
}

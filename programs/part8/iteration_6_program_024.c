#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

int main() {
    // Aligned arrays to help vectorization
    ALIGNED int a[N], b[N];
    ALIGNED int gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    // Initialize with distinct values to avoid trivial comparisons
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (>) and GE_EXPR (>=) comparisons
    for (int i = 0; i < N; i++) {
        // Both comparisons in same loop to potentially cover multiple types
        gt_result[i] = a[i] > b[i];    // GT_EXPR
        ge_result[i] = a[i] >= b[i];   // GE_EXPR
    }
    
    // Loop 2: LT_EXPR (<) and LE_EXPR (<=) comparisons  
    for (int i = 0; i < N; i++) {
        lt_result[i] = a[i] < b[i];    // LT_EXPR
        le_result[i] = a[i] <= b[i];   // LE_EXPR
    }
    
    // Loop 3: Mixed comparisons with result usage to prevent elimination
    for (int i = 0; i < N; i++) {
        // Use all comparison types with side effects
        if (a[i] > b[i]) sum += 1;     // GT_EXPR
        if (a[i] >= b[i]) sum += 2;    // GE_EXPR
        if (a[i] < b[i]) sum += 3;     // LT_EXPR
        if (a[i] <= b[i]) sum += 4;    // LE_EXPR
    }
    
    // Additional pattern: Comparison with scalar
    int threshold = 50;
    ALIGNED int scalar_comp[N];
    for (int i = 0; i < N; i++) {
        scalar_comp[i] = a[i] > threshold;  // GT_EXPR with scalar
    }
    
    // Use results to prevent dead code elimination
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i] + scalar_comp[i];
    }
    
    printf("Checksum: %d, Sum: %d\n", checksum, sum);
    
    // Return value based on comparisons to ensure they affect program output
    return (checksum > 1000) ? 0 : 1;
}
